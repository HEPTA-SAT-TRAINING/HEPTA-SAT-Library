/**
 * @file gps_fix.h
 * @brief Cached GPS state merged from GPGGA, GPRMC, and GPGSV sentences.
 */

#ifndef GPS_FIX_H
#define GPS_FIX_H

#include <Arduino.h>

struct GpsSatellite {
  uint8_t  prn       = 0;
  uint8_t  elevation = 0;
  uint16_t azimuth   = 0;
  uint8_t  snr       = 0;
};

struct GpsFix {
  bool     has_fix    = false;
  uint32_t fix_millis = 0;
  uint32_t gsv_millis = 0;

  float    lat = 0, lon = 0, alt = 0;
  uint8_t  fix_quality = 0;
  uint8_t  sat_num     = 0;
  float    hdop        = 0;

  float    velocity = 0;
  float    heading  = 0;
  float    gga_utc  = 0;
  float    rmc_utc  = 0;
  char     date[7]  = {0};

  uint8_t       sats_in_view = 0;
  uint8_t       sat_count    = 0;
  GpsSatellite  sats[12];
};

#endif /* GPS_FIX_H */
