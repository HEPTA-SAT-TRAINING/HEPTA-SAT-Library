#ifdef HEPTA_GPS_DMA_TEST
#include "../../drv/gps_dma_rx.h"
#include "../../drv/gps_gp1818mk.h"
#include "dma_stubs/fake_sdk.h"
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
unsigned checks = 0;
void check(bool value, int line) { ++checks; if (!value) { std::cerr << "DMA FAIL line " << line << '\n'; std::exit(1); } }
#define CHECK(x) check(bool(x), __LINE__)
uint8_t value(unsigned n) { return uint8_t(n % 255 + 1); }
void empty_resources() { CHECK(sdk_model::claimed_dma() == 0); CHECK(sdk_model::claimed_sm() == 0); CHECK(sdk_model::shared_handlers() == 0); }
}

int main() {
  GpsDmaRx rx;
  CHECK(rx.available() == 0 && rx.read_until(100) == -1);
  CHECK(rx.begin(13, 2));
  CHECK(sdk_model::claimed_dma() == 2 && sdk_model::claimed_sm() == 1);
  CHECK(rx.state() == GpsReceiverState::Ready && rx.received() == 0);
  // Many laps with regular reads: no invented drops, duplicated or reordered bytes.
  for (unsigned n = 0; n < 20000; ++n) {
    sdk_model::inject(value(n));
    CHECK(rx.received() == n + 1);
    CHECK(rx.read_until(n + 1) == value(n));
  }
  CHECK(rx.overrun_count() == 0 && rx.error_count() == 0);
  rx.end(); empty_resources();

  CHECK(rx.begin(13, uint32_t(-1)));
  for (unsigned n = 0; n < 3000; ++n) sdk_model::inject(value(n));
  uint64_t limit = rx.received();
  CHECK(limit == 3000 && rx.available() == 2048 && rx.overrun_count() == 1);
  for (unsigned n = 3000 - 2048; n < 3000; ++n) CHECK(rx.read_until(limit) == value(n));
  CHECK(rx.read_until(limit) == -1);

  sdk_model::inject('A');
  limit = rx.received();
  sdk_model::inject('B');
  CHECK(rx.read_until(limit) == 'A' && rx.read_until(limit) == -1);
  CHECK(rx.read_until(rx.received()) == 'B');
  sdk_model::inject(0);
  CHECK(rx.read_until(rx.received()) == -2 && rx.error_count() == 1);
  sdk_model::stall();
  rx.received();
  CHECK(rx.error_count() == 2);

  // Deferred callbacks spanning several laps cannot reconstruct exact order.
  uint32_t saved = save_and_disable_interrupts();
  for (unsigned n = 0; n < 7000; ++n) sdk_model::inject(value(n));
  restore_interrupts(saved);
  CHECK(rx.available() == 0 && rx.overrun_count() == 2);
  sdk_model::inject('$');
  CHECK(rx.read_until(rx.received()) == '$');
  // A single pending completion is accounted even with its IRQ not dispatched.
  saved = save_and_disable_interrupts();
  for (unsigned n = 0; n < 1024; ++n) sdk_model::inject(value(n));
  CHECK(rx.available() == 1024);
  restore_interrupts(saved);
  limit = rx.received();
  for (unsigned n = 0; n < 1024; ++n) CHECK(rx.read_until(limit) == value(n));
  sdk_model::unrelated_irq(1u << 11);
  rx.received();
  CHECK((dma_hw->ints1 & (1u << 11)) != 0); // Another client's IRQ was not cleared.
  dma_hw->ints1 = 1u << 11;

  sdk_model::bus_error();
  CHECK(rx.available() == 0 && rx.state() == GpsReceiverState::ResourceError);
  rx.end(); empty_resources();
  CHECK(rx.begin(13, NOPIN)); rx.end(); empty_resources();
  CHECK(!rx.begin(99, 2)); empty_resources();
  CHECK(!rx.begin(13, 13)); empty_resources();
  sdk_model::no_pio = true;
  CHECK(!rx.begin(13, 2)); empty_resources();
  sdk_model::no_pio = false;
  sdk_model::dma_limit = 1;
  CHECK(!rx.begin(13, 2)); empty_resources();
  sdk_model::dma_limit = NUM_DMA_CHANNELS;
  sdk_model::exclusive_irq = true;
  CHECK(!rx.begin(13, 2)); empty_resources();
  sdk_model::exclusive_irq = false;
  for (int i = 0; i < 20; ++i) { CHECK(rx.begin(13, 2)); rx.end(); empty_resources(); }
  // Entire production stack: modelled DMA bytes -> real parser -> public API.
  Gps1818mk gps;
  gps.begin();
  const std::string body = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
  unsigned checksum = 0;
  for (unsigned char c : body) checksum ^= c;
  const char* digits = "0123456789ABCDEF";
  std::string frame = "$" + body + "*" + digits[checksum >> 4] + digits[checksum & 15] + "\r\n";
  for (unsigned char c : frame) sdk_model::inject(c);
  GpsSnapshot snapshot;
  CHECK(gps.get_latest(&snapshot) && snapshot.position_valid && snapshot.gga_sequence == 1);
  CHECK(snapshot.gga.sat_num == 8 && snapshot.gga.alt == 545.4f);
  GpggaData previous = snapshot.gga;
  CHECK(!gps.get_gpgga(&previous)); // Parsed cache does not satisfy legacy RX wait.
  for (unsigned char c : frame) sdk_model::inject(c);
  CHECK(gps.get_gpgga(&previous));
  CHECK(gps.get_latest(&snapshot) && snapshot.gga_sequence == 2);
  gps.end(); empty_resources();
  std::cout << "PASS: " << checks << " production DMA transport checks against SDK register model.\n";
}
#endif
