#include "gps_dma_rx.h"

#include <Arduino.h>
#include <PIOProgram.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/sync.h>
#include <new>
#include <stdlib.h>

#include "gps_uart_rx.pio.h"

namespace {
constexpr uint kHalf = GpsDmaRx::kCapacity / 2;
constexpr uint kHalfBits = 10;
static_assert(kHalf == (1u << kHalfBits), "DMA ring alignment must match half size");
}

struct GpsDmaRx::Hardware {
  PIOProgram program{&hepta_gps_uart_rx_program};
  PIO pio = nullptr;
  int sm = -1;
  int channels[2] = {-1, -1};
  uint8_t* allocation = nullptr;
  volatile uint8_t* bytes = nullptr;
  uint32_t rx_pin = 0, tx_pin = uint32_t(-1);
  uint core = 0;
  volatile uint64_t completed = 0;
  volatile uint32_t lost_epochs = 0;
  bool irq_registered = false;
  bool faulted = false;
  static Hardware* owners[NUM_DMA_CHANNELS];
  static uint users[NUM_CORES];

  // DMA channels chain to one another; ring addresses and transfer counts
  // reload in hardware. Interrupts ONLY count completed halves. Reception
  // continues even if a callback is delayed; coalesced callbacks flag loss.
  void account() {
    if (faulted) return;
    uint32_t mask = (1u << channels[0]) | (1u << channels[1]);
    uint32_t pending = dma_hw->ints1 & mask;
    if (!pending) return;
    dma_hw->ints1 = pending;
    for (int ch : channels) {
      if (dma_channel_hw_addr(ch)->ctrl_trig & DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS) {
        faulted = true;
        for (int other : channels) {
          dma_channel_set_irq1_enabled(other, false);
          hw_clear_bits(&dma_channel_hw_addr(other)->al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
        }
        return;
      }
    }
    if (pending == mask) {
      ++lost_epochs;
      // At least a complete ring elapsed while IRQs were unserviced. Exact
      // elapsed laps are unknowable. Resynchronise the phase and discard.
      uint phase = dma_channel_is_busy(channels[1]) ? 1 : 0;
      completed = ((completed + 2) & ~uint64_t(1)) + phase;
    } else {
      uint expected = uint(completed & 1u);
      if (pending != (1u << channels[expected])) {
        ++lost_epochs;
        ++completed;
      }
      ++completed;
    }
  }

  static void irq() {
    for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch) {
      Hardware* owner = owners[ch];
      if (owner && owner->core == get_core_num() && owner->channels[0] == int(ch)) owner->account();
    }
  }

  uint64_t position() {
    uint32_t saved = save_and_disable_interrupts();
    uint64_t value = 0;
    // Reading transfer_count commits only completed DMA writes. Retry if a
    // block completed between accounting and reading that block's counter.
    for (;;) {
      account();
      if (faulted) break;
      uint phase = uint(completed & 1u);
      uint left = dma_channel_hw_addr(channels[phase])->transfer_count;
      uint32_t mask = (1u << channels[0]) | (1u << channels[1]);
      if (dma_hw->ints1 & mask) continue;
      value = completed * kHalf + (kHalf - left);
      break;
    }
    __dmb();
    restore_interrupts(saved);
    return value;
  }
};

GpsDmaRx::Hardware* GpsDmaRx::Hardware::owners[NUM_DMA_CHANNELS] = {};
uint GpsDmaRx::Hardware::users[NUM_CORES] = {};

GpsDmaRx::~GpsDmaRx() { end(); }

bool GpsDmaRx::begin(uint32_t rx_pin, uint32_t tx_pin) {
  end();
  if (tx_pin == NOPIN) tx_pin = uint32_t(-1);
  consumed_ = 0;
  overruns_ = errors_ = seen_loss_ = 0;
  state_ = GpsReceiverState::ResourceError;
  if (rx_pin >= NUM_BANK0_GPIOS || (tx_pin != uint32_t(-1) && tx_pin >= NUM_BANK0_GPIOS) || rx_pin == tx_pin) return false;
  hw_ = new (std::nothrow) Hardware;
  if (!hw_) return false;
  hw_->rx_pin = rx_pin;
  hw_->tx_pin = tx_pin;
  hw_->core = get_core_num();
  int offset;
  if (!hw_->program.prepare(&hw_->pio, &hw_->sm, &offset, rx_pin, 1)) {
    end(); state_ = GpsReceiverState::ResourceError; return false;
  }
  hw_->allocation = static_cast<uint8_t*>(malloc(kCapacity + kHalf - 1));
  for (int i = 0; i < 2; ++i) hw_->channels[i] = dma_claim_unused_channel(false);
  if (!hw_->allocation || hw_->channels[0] < 0 || hw_->channels[1] < 0 ||
      irq_get_exclusive_handler(DMA_IRQ_1)) {
    end(); state_ = GpsReceiverState::ResourceError; return false;
  }
  uintptr_t aligned = (reinterpret_cast<uintptr_t>(hw_->allocation) + kHalf - 1) & ~uintptr_t(kHalf - 1);
  hw_->bytes = reinterpret_cast<volatile uint8_t*>(aligned);

  gpio_set_input_enabled(rx_pin, true);
  gpio_pull_up(rx_pin);
  pio_gpio_init(hw_->pio, rx_pin);
  pio_sm_set_consecutive_pindirs(hw_->pio, hw_->sm, rx_pin, 1, false);
  pio_sm_config config = hepta_gps_uart_rx_program_get_default_config(offset);
  sm_config_set_in_pins(&config, rx_pin);
  sm_config_set_jmp_pin(&config, rx_pin);
  sm_config_set_in_shift(&config, true, false, 32);
  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
  sm_config_set_clkdiv(&config, float(clock_get_hz(clk_sys)) / (9600.0f * 8.0f));
  pio_sm_init(hw_->pio, hw_->sm, offset, &config);
  hw_->pio->fdebug = 1u << (PIO_FDEBUG_RXSTALL_LSB + hw_->sm);

  for (int i = 0; i < 2; ++i) {
    int ch = hw_->channels[i];
    dma_channel_config dma_config = dma_channel_get_default_config(ch);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_ring(&dma_config, true, kHalfBits);
    channel_config_set_dreq(&dma_config, pio_get_dreq(hw_->pio, hw_->sm, false));
    channel_config_set_chain_to(&dma_config, hw_->channels[1 - i]);
    // PIO data occupies bits 31:24; a byte access still pops one FIFO word.
    const volatile uint8_t* fifo = reinterpret_cast<const volatile uint8_t*>(&hw_->pio->rxf[hw_->sm]) + 3;
    dma_channel_configure(ch, &dma_config, hw_->bytes + i * kHalf, fifo, kHalf, false);
    Hardware::owners[ch] = hw_;
    dma_channel_acknowledge_irq1(ch);
    dma_channel_set_irq1_enabled(ch, true);
  }
  if (!Hardware::users[hw_->core]++) {
    irq_add_shared_handler(DMA_IRQ_1, Hardware::irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  }
  hw_->irq_registered = true;
  irq_set_enabled(DMA_IRQ_1, true);
  // No public GPS TX operation exists. Keep the configured command input at
  // UART idle HIGH without consuming an additional TX state machine.
  if (tx_pin != uint32_t(-1)) { gpio_init(tx_pin); gpio_put(tx_pin, true); gpio_set_dir(tx_pin, true); }
  state_ = GpsReceiverState::Ready;
  dma_start_channel_mask(1u << hw_->channels[0]);
  pio_sm_set_enabled(hw_->pio, hw_->sm, true);
  return true;
}

void GpsDmaRx::end() {
  if (hw_) {
    if (hw_->sm >= 0) pio_sm_set_enabled(hw_->pio, hw_->sm, false);
    // Disable both chain targets before aborting either channel (RP2040 E13).
    for (int ch : hw_->channels) if (ch >= 0) {
      dma_channel_set_irq1_enabled(ch, false);
      hw_clear_bits(&dma_channel_hw_addr(ch)->al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
    }
    for (int ch : hw_->channels) if (ch >= 0) {
      dma_channel_abort(ch);
      dma_channel_acknowledge_irq1(ch);
      Hardware::owners[ch] = nullptr;
      dma_channel_unclaim(ch);
    }
    if (hw_->irq_registered && !--Hardware::users[hw_->core]) irq_remove_handler(DMA_IRQ_1, Hardware::irq);
    if (hw_->sm >= 0) { gpio_init(hw_->rx_pin); gpio_disable_pulls(hw_->rx_pin); }
    if (hw_->tx_pin < NUM_BANK0_GPIOS) gpio_init(hw_->tx_pin);
    free(hw_->allocation);
    delete hw_; // PIOProgram releases its SM; its instructions are cached by the core.
    hw_ = nullptr;
  }
  consumed_ = 0;
  state_ = GpsReceiverState::Stopped;
}

uint64_t GpsDmaRx::received() {
  if (!hw_ || state_ != GpsReceiverState::Ready) return 0;
  uint64_t value = hw_->position();
  if (hw_->faulted) {
    state_ = GpsReceiverState::ResourceError;
    ++errors_;
    consumed_ = 0;
    return 0;
  }
  uint32_t stall = 1u << (PIO_FDEBUG_RXSTALL_LSB + hw_->sm);
  if (hw_->pio->fdebug & stall) {
    hw_->pio->fdebug = stall;
    ++errors_;
    consumed_ = value; // The stream has a gap; do not splice partial sentences.
  }
  if (seen_loss_ != hw_->lost_epochs) {
    seen_loss_ = hw_->lost_epochs;
    ++overruns_;
    consumed_ = value;
  } else if (value - consumed_ > kCapacity) {
    ++overruns_;
    consumed_ = value - kCapacity;
  }
  return value;
}

size_t GpsDmaRx::available() {
  uint64_t produced = received();
  return size_t(produced - consumed_);
}

int GpsDmaRx::read_until(uint64_t limit) {
  uint32_t losses = overruns_ + errors_;
  uint64_t produced = received();
  if (losses != overruns_ + errors_) return -2;
  if (!hw_ || consumed_ >= limit || consumed_ >= produced) return -1;
  uint64_t at = consumed_;
  uint8_t byte = hw_->bytes[at % kCapacity];
  __dmb();
  produced = received();
  if (losses != overruns_ + errors_) return -2;
  if (produced - at > kCapacity) return -2;
  consumed_ = at + 1;
  if (!byte) { ++errors_; return -2; } // Framing/BREAK marker from the PIO.
  return byte;
}
