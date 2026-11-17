#ifndef HEPTA_GPS_NMEA_H
#define HEPTA_GPS_NMEA_H

#include "gps_snapshot.h"
#include <stddef.h>

// No hardware, allocation, global state or waits. One parser per receiver.
class GpsNmeaParser {
 public:
  enum class Event { None, Gga, Rmc, InvalidGga, InvalidRmc };
  void reset();
  void discard_partial();
  Event feed(uint8_t byte, uint32_t parsed_ms);
  Event pending_type() const;
  const GpsSnapshot& snapshot() const { return snapshot_; }

 private:
  static constexpr size_t kLineSize = 128;
  char line_[kLineSize] = {};
  size_t length_ = 0;
  GpsSnapshot snapshot_;
  Event finish(uint32_t parsed_ms);
};

#endif
