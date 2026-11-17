#ifndef HEPTA_GPS_DMA_RX_H
#define HEPTA_GPS_DMA_RX_H

#include "../common/gps_snapshot.h"
#include <stddef.h>
#include <stdint.h>

// Internal RX-only 9600-baud 8N1 transport. All methods must run on the core
// that called begin(); DMA IRQ bookkeeping runs there, never NMEA parsing.
class GpsDmaRx {
 public:
  static constexpr size_t kCapacity = 2048; // Effective UART bytes, not DMA words.
  GpsDmaRx() = default;
  ~GpsDmaRx();
  GpsDmaRx(const GpsDmaRx&) = delete;
  GpsDmaRx& operator=(const GpsDmaRx&) = delete;
  bool begin(uint32_t rx_pin, uint32_t tx_pin);
  void end();
  uint64_t received();
  size_t available();
  // -1: no byte, -2: discontinuity; reads only positions below limit.
  int read_until(uint64_t limit);
  uint32_t overrun_count() const { return overruns_; }
  uint32_t error_count() const { return errors_; }
  GpsReceiverState state() const { return state_; }

 private:
  struct Hardware;
  Hardware* hw_ = nullptr;
  uint64_t consumed_ = 0;
  uint32_t overruns_ = 0;
  uint32_t errors_ = 0;
  uint32_t seen_loss_ = 0;
  GpsReceiverState state_ = GpsReceiverState::Stopped;
};

#endif
