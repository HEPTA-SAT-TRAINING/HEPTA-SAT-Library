#pragma once
#include <stdint.h>
#include <stddef.h>

using uint = unsigned;
using pin_size_t = uint32_t;
uint32_t millis();
void delay(uint32_t ms);
#define NUM_DMA_CHANNELS 12
#define NUM_CORES 2
#define NUM_BANK0_GPIOS 30
#define NOPIN 255u
#define DMA_IRQ_1 1
#define DMA_CH0_CTRL_TRIG_EN_BITS 1u
#define DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS (1u << 31)
#define PIO_FDEBUG_RXSTALL_LSB 24
#define PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY 128
#define PICO_PIO_VERSION 0
#define PIO_FIFO_JOIN_RX 1
#define DMA_SIZE_8 0
#define clk_sys 0

// Model write-one-to-clear status registers; ordinary assignments must NOT
// replace all status bits or accidentally acknowledge another DMA client.
struct StatusRegister {
  uint32_t bits = 0;
  operator uint32_t() const { return bits; }
  void operator=(uint32_t value) { bits &= ~value; }
};
struct pio_program {
  const uint16_t* instructions;
  uint length;
  int origin;
  uint pio_version;
};
struct pio_sm_config { uint pin = 0; };
struct PioRegisters {
  volatile uint32_t rxf[4] = {};
  StatusRegister fdebug;
};
using PIO = PioRegisters*;
struct DmaChannel { uint32_t ctrl_trig = 0, al1_ctrl = 0, transfer_count = 0; };
struct DmaRegisters {
  DmaChannel ch[NUM_DMA_CHANNELS];
  StatusRegister ints1;
};
extern DmaRegisters* dma_hw;
struct dma_channel_config { uint ring_bits = 0, chain = 0, sm = 0; };
using irq_handler_t = void (*)();

namespace sdk_model {
extern bool no_pio;
extern int dma_limit;
extern bool exclusive_irq;
void inject(uint8_t byte);
void stall();
void bus_error();
unsigned claimed_dma();
unsigned claimed_sm();
unsigned shared_handlers();
void unrelated_irq(uint32_t bits);
}

class PIOProgram {
 public:
  explicit PIOProgram(const pio_program*) {}
  ~PIOProgram();
  bool prepare(PIO*, int*, int*, int, int);
 private:
  bool claimed_ = false;
};

uint get_core_num();
uint32_t save_and_disable_interrupts();
void restore_interrupts(uint32_t);
void __dmb();
uint32_t clock_get_hz(uint);
int dma_claim_unused_channel(bool);
void dma_channel_unclaim(uint);
DmaChannel* dma_channel_hw_addr(uint);
bool dma_channel_is_busy(uint);
dma_channel_config dma_channel_get_default_config(uint);
void channel_config_set_transfer_data_size(dma_channel_config*, uint);
void channel_config_set_read_increment(dma_channel_config*, bool);
void channel_config_set_write_increment(dma_channel_config*, bool);
void channel_config_set_ring(dma_channel_config*, bool, uint);
void channel_config_set_dreq(dma_channel_config*, uint);
void channel_config_set_chain_to(dma_channel_config*, uint);
void dma_channel_configure(uint, const dma_channel_config*, volatile void*, const volatile void*, uint, bool);
void dma_channel_acknowledge_irq1(uint);
void dma_channel_set_irq1_enabled(uint, bool);
void dma_start_channel_mask(uint32_t);
void dma_channel_abort(uint);
void hw_clear_bits(uint32_t*, uint32_t);
irq_handler_t irq_get_exclusive_handler(uint);
void irq_add_shared_handler(uint, irq_handler_t, uint);
void irq_remove_handler(uint, irq_handler_t);
void irq_set_enabled(uint, bool);
pio_sm_config pio_get_default_sm_config();
void sm_config_set_wrap(pio_sm_config*, uint, uint);
void sm_config_set_in_pins(pio_sm_config*, uint);
void sm_config_set_jmp_pin(pio_sm_config*, uint);
void sm_config_set_in_shift(pio_sm_config*, bool, bool, uint);
void sm_config_set_fifo_join(pio_sm_config*, uint);
void sm_config_set_clkdiv(pio_sm_config*, float);
void pio_sm_init(PIO, uint, uint, const pio_sm_config*);
void pio_sm_set_enabled(PIO, uint, bool);
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool);
uint pio_get_dreq(PIO, uint, bool);
void pio_gpio_init(PIO, uint);
void gpio_set_input_enabled(uint, bool);
void gpio_pull_up(uint);
void gpio_init(uint);
void gpio_put(uint, bool);
void gpio_set_dir(uint, bool);
void gpio_disable_pulls(uint);
