/**
 * @file gps_gp1818mk.cpp
 * @author Masaki Naito
 * @brief Driver implementation for GP-1818MK GPS module
 * @version 0.3
 * @date 2025-02-26
 *
 * @copyright UNISEC all rights reserved.
 */

#include "gps_gp1818mk.h"

/**
 * @brief Convert NMEA coordinate format (DDMM.MMMM) to decimal degrees.
 *
 * GPS outputs latitude as DDMM.MMMM (e.g. 3543.1234 = 35°43.1234')
 * and longitude as DDDMM.MMMM. Formula: DD + MM.MMMM / 60
 */
static float nmea_to_decimal(float nmea_value) {
  int   degrees = (int)(nmea_value / 100.0f);
  float minutes = nmea_value - (float)degrees * 100.0f;
  return (float)degrees + minutes / 60.0f;
}

void Gps1818mk::begin(void) {
  if (!_serial) {
    _serial = new SoftwareSerial(_rx_pin, _tx_pin);
    _serial->begin(9600);
  }
}

int Gps1818mk::read_byte(void) {
  if (!_serial || !_serial->available()) return -1;
  return _serial->read();
}

bool Gps1818mk::get_position(float* lat, float* lon, float* alt) {
  if (!wait_serial())        return false;
  if (!get_header("$GPGGA")) return false;
  return parse_gpgga(lat, lon, alt);
}

bool Gps1818mk::get_velocity(float* velocity, float* heading) {
  if (!wait_serial())        return false;
  if (!get_header("$GPRMC")) return false;
  return parse_gprmc(velocity, heading);
}

bool Gps1818mk::get_all(float* lat, float* lon, float* alt,
                         float* velocity, float* heading) {
  if (!wait_serial()) return false;

  // GP-1818MK outputs sentences in standard NMEA order: $GPGGA before $GPRMC.
  // Searching GGA first and then RMC is therefore safe for this module — after GGA
  // is found and consumed, RMC will arrive later in the same 1 Hz cycle.
  // If called with a non-standard module that emits RMC before GGA, the RMC search
  // would have to wait for the next cycle (~1 s); use get_position()/get_velocity()
  // separately in that case.

  // GPGGA: position + altitude
  if (!get_header("$GPGGA"))       return false;
  if (!parse_gpgga(lat, lon, alt)) return false;

  // GPRMC: velocity + heading (arrives after GPGGA in the same cycle)
  if (!get_header("$GPRMC"))       return false;
  return parse_gprmc(velocity, heading);
}

bool Gps1818mk::is_data_available(void) {
  return _serial && _serial->available();
}

int Gps1818mk::read_byte_timeout(void) {
  if (!_serial) return -1;
  const uint32_t TIMEOUT_MS = 200;
  uint32_t start = millis();
  while (millis() - start < TIMEOUT_MS) {
    if (_serial->available()) return _serial->read();
  }
  return -1;  // timeout — caller (get_header/read_sentence) treats this as failure
}

bool Gps1818mk::wait_serial(void) {
  if (!_serial) return false;
  const uint32_t TIMEOUT_MS = 1000;
  uint32_t start = millis();
  while (millis() - start < TIMEOUT_MS) {
    if (_serial->available()) return true;
    delay(1);
  }
  return false;
}

bool Gps1818mk::get_header(const char* header) {
  /*
   * Sliding-window search: shift the 6-byte window on each new byte,
   * then compare against the target header.
   *
   *   before: [A][B][C][D][E][F]
   *   +G   →  [B][C][D][E][F][G]  ← compare here
   */
  const uint8_t  HEADER_LEN       = 6;
  const uint32_t TOTAL_TIMEOUT_MS = 5000;
  char     window[HEADER_LEN]     = {0};
  uint32_t start                  = millis();

  // Pre-fill the first HEADER_LEN-1 bytes of the window.
  // Use index-advance-on-success so inter-sentence gaps (read_byte_timeout() == -1) are retried
  // without consuming a slot in the window.
  for (uint8_t j = 0; j < HEADER_LEN - 1; ) {
    if (millis() - start >= TOTAL_TIMEOUT_MS) return false;
    int b = read_byte_timeout();
    if (b < 0) continue;  // timeout on this byte — retry within the budget
    window[j++] = (char)b;
  }

  while (millis() - start < TOTAL_TIMEOUT_MS) {
    int b = read_byte_timeout();
    if (b < 0) continue;  // inter-sentence gap — retry within the 5 s budget

    for (uint8_t j = 0; j < HEADER_LEN - 1; j++) window[j] = window[j + 1];
    window[HEADER_LEN - 1] = (char)b;

    bool match = true;
    for (uint8_t j = 0; j < HEADER_LEN; j++) {
      if (window[j] != header[j]) { match = false; break; }
    }
    if (match) return true;
  }

  return false;
}

bool Gps1818mk::read_sentence(char* buf, uint16_t len) {
  // The 6-char NMEA header (e.g. "$GPGGA") is always followed by a comma.
  // Consume that comma first, then read until the CRLF line terminator.
  if (read_byte_timeout() < 0) return false;

  bool found_end = false;
  for (uint16_t i = 0; i < len - 1; i++) {
    int b = read_byte_timeout();
    if (b < 0) return false;  // read timeout
    buf[i] = (char)b;
    if (i > 0 && buf[i - 1] == '\r' && buf[i] == '\n') { found_end = true; break; }
  }

  if (!found_end) return false;  // sentence too long or missing terminator
  return true;
}

bool Gps1818mk::parse_gpgga(float* lat, float* lon, float* alt) {
  /*
   * GPGGA fields after "$GPGGA,":
   *   UTC, Lat, N/S, Lon, E/W, Fix, Sats, HDOP, Alt, M, Geoid, M
   *
   * Fix: 0 = no fix, 1 = GPS, 2 = DGPS
   */

  const uint16_t BUF_LEN = 128;
  char raw[BUF_LEN] = {0};
  if (!read_sentence(raw, BUF_LEN)) return false;

  float utc_time = 0, lat_raw = 0, lon_raw = 0, msl_alt = 0, hdop = 0, geoid = 0;
  int   fix_quality = 0, sat_num = 0;
  char  lat_dir = 0, lon_dir = 0, alt_unit = 0, geoid_unit = 0;

  sscanf(raw, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c",
         &utc_time, &lat_raw, &lat_dir, &lon_raw, &lon_dir,
         &fix_quality, &sat_num, &hdop, &msl_alt, &alt_unit, &geoid, &geoid_unit);

  if (fix_quality == 0) return false;

  *lat = nmea_to_decimal(lat_raw);
  *lon = nmea_to_decimal(lon_raw);

  // South latitude and West longitude are expressed as negative values
  if (lat_dir == 'S') *lat = -*lat;
  if (lon_dir == 'W') *lon = -*lon;

  *alt = msl_alt;
  return true;
}

bool Gps1818mk::parse_gprmc(float* velocity, float* heading) {
  /*
   * GPRMC fields after "$GPRMC,":
   *   UTC, Status, Lat, N/S, Lon, E/W, Speed(kt), Course(deg), Date, ...
   *
   * Status: 'A' = valid fix, 'V' = void (no fix)
   */

  const uint16_t BUF_LEN = 128;
  char raw[BUF_LEN] = {0};
  if (!read_sentence(raw, BUF_LEN)) return false;

  float utc_time = 0, lat_raw = 0, lon_raw = 0, speed_kt = 0, course = 0;
  char  status = 0, lat_dir = 0, lon_dir = 0, date[7] = {0};

  sscanf(raw, "%f,%c,%f,%c,%f,%c,%f,%f,%6s",
         &utc_time, &status,
         &lat_raw, &lat_dir,  // consumed to reach speed_kt and course
         &lon_raw, &lon_dir,
         &speed_kt, &course, date);

  if (status != 'A') return false;  // 'A' = active/valid, 'V' = void; anything else is malformed

  const float KNOT_TO_MPS = 0.514444f;  // 1 knot = 0.514444 m/s
  *velocity = speed_kt * KNOT_TO_MPS;
  *heading  = course;

  return true;
}
