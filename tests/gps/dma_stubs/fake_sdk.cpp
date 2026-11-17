#ifdef HEPTA_GPS_DMA_TEST
#include "fake_sdk.h"
#include <algorithm>
#include <cassert>
#include <vector>

namespace {
DmaRegisters registers;
PioRegisters pio;
struct Channel {
  bool claimed = false, active = false, irq = false;
  volatile uint8_t* write = nullptr;
  uintptr_t address = 0;
  uint count = 0;
  dma_channel_config config;
};
Channel channels[NUM_DMA_CHANNELS];
bool interrupts = true, irq_enabled = false, pio_enabled = false;
unsigned sms = 0;
uint32_t clock_ms = 0;
std::vector<irq_handler_t> handlers;
void dispatch() {
  if (!interrupts || !irq_enabled) return;
  for (auto handler : handlers) handler();
}
void start(uint ch) {
  auto& c = channels[ch];
  if (!(registers.ch[ch].ctrl_trig & DMA_CH0_CTRL_TRIG_EN_BITS)) return;
  c.active = true;
  registers.ch[ch].transfer_count = c.count;
}
}
DmaRegisters* dma_hw = &registers;
uint32_t millis() { return clock_ms++; }
void delay(uint32_t ms) { clock_ms += ms; }
namespace sdk_model {
bool no_pio = false;
int dma_limit = NUM_DMA_CHANNELS;
bool exclusive_irq = false;
void inject(uint8_t byte) {
  if (!pio_enabled) return;
  for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch) {
    auto& c = channels[ch];
    if (!c.active) continue;
    *reinterpret_cast<volatile uint8_t*>(c.address) = byte;
    uintptr_t mask = (uintptr_t(1) << c.config.ring_bits) - 1;
    c.address = (c.address & ~mask) | ((c.address + 1) & mask);
    if (!--registers.ch[ch].transfer_count) {
      c.active = false;
      if (c.irq) registers.ints1.bits |= 1u << ch;
      start(c.config.chain);
      dispatch();
    }
    return;
  }
  pio.fdebug.bits |= 1u << PIO_FDEBUG_RXSTALL_LSB;
}
void stall() { pio.fdebug.bits |= 1u << PIO_FDEBUG_RXSTALL_LSB; }
void bus_error() {
  for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch) if (channels[ch].active) {
    registers.ch[ch].ctrl_trig |= DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS;
    channels[ch].active = false;
    registers.ints1.bits |= 1u << ch;
    dispatch();
    break;
  }
}
unsigned claimed_dma() { unsigned n = 0; for (auto& c : channels) n += c.claimed; return n; }
unsigned claimed_sm() { return sms; }
unsigned shared_handlers() { return unsigned(handlers.size()); }
void unrelated_irq(uint32_t bits) { registers.ints1.bits |= bits; }
}

PIOProgram::~PIOProgram() { if (claimed_) --sms; }
bool PIOProgram::prepare(PIO* out, int* sm, int* offset, int, int) {
  if (sdk_model::no_pio) return false;
  claimed_ = true; ++sms; *out = &pio; *sm = 0; *offset = 0; return true;
}
uint get_core_num() { return 0; }
uint32_t save_and_disable_interrupts() { uint32_t old = interrupts; interrupts = false; return old; }
void restore_interrupts(uint32_t old) { interrupts = bool(old); dispatch(); }
void __dmb() {}
uint32_t clock_get_hz(uint) { return 150000000; }
int dma_claim_unused_channel(bool) {
  for (int ch = 0; ch < sdk_model::dma_limit; ++ch) if (!channels[ch].claimed) {
    channels[ch] = Channel{}; channels[ch].claimed = true;
    registers.ch[ch] = DmaChannel{};
    return ch;
  }
  return -1;
}
void dma_channel_unclaim(uint ch) { channels[ch].claimed = false; }
DmaChannel* dma_channel_hw_addr(uint ch) { return &registers.ch[ch]; }
bool dma_channel_is_busy(uint ch) { return channels[ch].active; }
dma_channel_config dma_channel_get_default_config(uint ch) { dma_channel_config c; c.chain = ch; return c; }
void channel_config_set_transfer_data_size(dma_channel_config*, uint size) { assert(size == DMA_SIZE_8); }
void channel_config_set_read_increment(dma_channel_config*, bool value) { assert(!value); }
void channel_config_set_write_increment(dma_channel_config*, bool value) { assert(value); }
void channel_config_set_ring(dma_channel_config* c, bool write, uint bits) { assert(write); c->ring_bits = bits; }
void channel_config_set_dreq(dma_channel_config* c, uint sm) { c->sm = sm; }
void channel_config_set_chain_to(dma_channel_config* c, uint ch) { c->chain = ch; }
void dma_channel_configure(uint ch, const dma_channel_config* config, volatile void* write,
                           const volatile void* read, uint count, bool trigger) {
  assert(read == reinterpret_cast<const volatile uint8_t*>(&pio.rxf[0]) + 3);
  auto& c = channels[ch];
  c.config = *config; c.write = static_cast<volatile uint8_t*>(write);
  c.address = reinterpret_cast<uintptr_t>(write); c.count = count;
  assert(!(c.address & ((uintptr_t(1) << config->ring_bits) - 1)));
  registers.ch[ch].ctrl_trig = DMA_CH0_CTRL_TRIG_EN_BITS;
  registers.ch[ch].transfer_count = count;
  if (trigger) start(ch);
}
void dma_channel_acknowledge_irq1(uint ch) { registers.ints1 = 1u << ch; }
void dma_channel_set_irq1_enabled(uint ch, bool value) { channels[ch].irq = value; }
void dma_start_channel_mask(uint32_t mask) { for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch) if (mask & (1u << ch)) start(ch); }
void dma_channel_abort(uint ch) { channels[ch].active = false; }
void hw_clear_bits(uint32_t* address, uint32_t mask) {
  for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch) if (address == &registers.ch[ch].al1_ctrl) {
    registers.ch[ch].ctrl_trig &= ~mask; return;
  }
  *address &= ~mask;
}
irq_handler_t irq_get_exclusive_handler(uint) { return sdk_model::exclusive_irq ? +[](){} : nullptr; }
void irq_add_shared_handler(uint, irq_handler_t handler, uint) { handlers.push_back(handler); }
void irq_remove_handler(uint, irq_handler_t handler) { handlers.erase(std::remove(handlers.begin(), handlers.end(), handler), handlers.end()); }
void irq_set_enabled(uint, bool value) { irq_enabled = value; }
pio_sm_config pio_get_default_sm_config() { return {}; }
void sm_config_set_wrap(pio_sm_config*, uint, uint) {}
void sm_config_set_in_pins(pio_sm_config* c, uint pin) { c->pin = pin; }
void sm_config_set_jmp_pin(pio_sm_config*, uint) {}
void sm_config_set_in_shift(pio_sm_config*, bool right, bool push, uint) { assert(right && !push); }
void sm_config_set_fifo_join(pio_sm_config*, uint) {}
void sm_config_set_clkdiv(pio_sm_config*, float) {}
void pio_sm_init(PIO, uint, uint, const pio_sm_config*) {}
void pio_sm_set_enabled(PIO, uint, bool value) { pio_enabled = value; }
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool) {}
uint pio_get_dreq(PIO, uint sm, bool) { return sm; }
void pio_gpio_init(PIO, uint) {}
void gpio_set_input_enabled(uint, bool) {}
void gpio_pull_up(uint) {}
void gpio_init(uint) {}
void gpio_put(uint, bool) {}
void gpio_set_dir(uint, bool) {}
void gpio_disable_pulls(uint) {}
#endif
