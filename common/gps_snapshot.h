/** GPS data shared by the legacy API and the on-demand snapshot API. */
#ifndef HEPTA_GPS_SNAPSHOT_H
#define HEPTA_GPS_SNAPSHOT_H

#include <stdint.h>

// Keep the fields, order and units of the original public structs unchanged.
struct GpggaData {
  float utc_time;       ///< UTC time, hhmmss.sss.
  float lat;            ///< Decimal degrees; South is negative.
  float lon;            ///< Decimal degrees; West is negative.
  float alt;            ///< Metres above mean sea level.
  uint8_t fix_quality;  ///< 0 = no fix, 1 = GPS, 2 = DGPS.
  uint8_t sat_num;      ///< Satellites in use.
  float hdop;           ///< Horizontal dilution of precision.
};

struct GprmcData {
  float utc_time;       ///< UTC time, hhmmss.sss.
  float velocity;       ///< Ground speed in m/s.
  float heading;        ///< True course in degrees [0, 360).
  char date[7];         ///< UTC date, ddmmyy, null-terminated.
};

enum class GpsReceiverState : uint8_t { Stopped, Ready, ResourceError };

struct GpsSnapshot {
  GpggaData gga = {};
  GprmcData rmc = {};
  bool has_gga = false;
  bool has_rmc = false;
  bool position_valid = false;
  bool velocity_valid = false;
  bool heading_valid = false; // Course may be empty even in an active RMC.
  uint32_t gga_sequence = 0;
  uint32_t rmc_sequence = 0;
  // Processing times, NOT arrival times or proof that a buffered fix is fresh.
  uint32_t gga_parsed_ms = 0;
  uint32_t rmc_parsed_ms = 0;
  GpsReceiverState receiver_state = GpsReceiverState::Stopped;
  uint32_t rx_overrun_count = 0;  // Detected loss events, not number of lost bytes.
  uint32_t rx_error_count = 0;
  uint32_t parse_error_count = 0;
};

#endif
