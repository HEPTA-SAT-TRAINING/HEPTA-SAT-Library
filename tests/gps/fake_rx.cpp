#ifdef HEPTA_GPS_HOST_TEST
// Test transport only: production driver + parser are compiled unchanged.
#include "fake_rx.h"
#include "../../drv/gps_dma_rx.h"
#include <algorithm>
#include <map>

namespace {
struct Stream {
  std::string bytes;
  std::string after_read;
  std::string after_delay;
  uint32_t errors = 0;
};
std::map<uint32_t, Stream> streams;
}
namespace fake_rx {
uint32_t clock_ms = 0;
bool fail_begin = false;
void inject(uint32_t p, const std::string& s) { streams[p].bytes += s; }
void inject_after_read(uint32_t p, const std::string& s) { streams[p].after_read = s; }
void inject_after_delay(uint32_t p, const std::string& s) { streams[p].after_delay = s; }
void error(uint32_t p) { ++streams[p].errors; }
}
uint32_t millis() { return fake_rx::clock_ms++; }
void delay(uint32_t ms) {
  fake_rx::clock_ms += ms;
  for (auto& item : streams) {
    item.second.bytes += item.second.after_delay;
    item.second.after_delay.clear();
  }
}

struct GpsDmaRx::Hardware { uint32_t pin; };
GpsDmaRx::~GpsDmaRx() { end(); }
bool GpsDmaRx::begin(uint32_t rx, uint32_t) {
  end(); consumed_ = 0; overruns_ = errors_ = seen_loss_ = 0;
  if (fake_rx::fail_begin) { state_ = GpsReceiverState::ResourceError; return false; }
  hw_ = new Hardware{rx};
  streams[rx] = Stream{};
  state_ = GpsReceiverState::Ready;
  return true;
}
void GpsDmaRx::end() {
  delete hw_; hw_ = nullptr; consumed_ = 0; state_ = GpsReceiverState::Stopped;
}
uint64_t GpsDmaRx::received() {
  if (!hw_) return 0;
  Stream& s = streams[hw_->pin];
  uint64_t n = s.bytes.size();
  if (errors_ != s.errors) { errors_ = s.errors; consumed_ = n; }
  if (n - consumed_ > kCapacity) { ++overruns_; consumed_ = n - kCapacity; }
  return n;
}
size_t GpsDmaRx::available() { uint64_t n = received(); return size_t(n - consumed_); }
int GpsDmaRx::read_until(uint64_t limit) {
  uint32_t losses = overruns_ + errors_;
  uint64_t n = received();
  if (losses != overruns_ + errors_) return -2;
  if (!hw_ || consumed_ >= n || consumed_ >= limit) return -1;
  Stream& s = streams[hw_->pin];
  int result = uint8_t(s.bytes[size_t(consumed_++)]);
  s.bytes += s.after_read;
  s.after_read.clear();
  return result;
}
#endif
