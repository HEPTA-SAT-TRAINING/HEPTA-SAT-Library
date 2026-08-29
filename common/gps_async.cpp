/**
 * @file gps_async.cpp
 * @brief Non-blocking incremental NMEA parser for GP-1818MK.
 */

#include "gps_async.h"

#include "../drv/gps_gp1818mk.h"

#include <stdlib.h>
#include <string.h>

namespace {

GpsFix    gps_fix;
mutex_t   gps_fix_mutex;
Gps1818mk *gps_driver = nullptr;

constexpr uint16_t kLineBufLen = 96;
char     line_buf[kLineBufLen];
uint8_t  line_len = 0;

float nmea_to_decimal(float nmea_value) {
  int   degrees = (int)(nmea_value / 100.0f);
  float minutes = nmea_value - (float)degrees * 100.0f;
  return (float)degrees + minutes / 60.0f;
}

void handle_gpgga(char *fields) {
  float utc_time = 0, lat_raw = 0, lon_raw = 0, msl_alt = 0, hdop = 0;
  int   fix_quality = 0, sat_num = 0;
  char  lat_dir = 0, lon_dir = 0;

  sscanf(fields, "%f,%f,%c,%f,%c,%d,%d,%f,%f",
         &utc_time, &lat_raw, &lat_dir, &lon_raw, &lon_dir,
         &fix_quality, &sat_num, &hdop, &msl_alt);

  mutex_enter_blocking(&gps_fix_mutex);
  gps_fix.gga_utc     = utc_time;
  gps_fix.fix_quality = (uint8_t)fix_quality;
  gps_fix.sat_num     = (uint8_t)sat_num;
  gps_fix.hdop        = hdop;
  gps_fix.fix_millis  = millis();
  gps_fix.has_fix     = fix_quality > 0;
  if (fix_quality > 0) {
    float lat = nmea_to_decimal(lat_raw);
    float lon = nmea_to_decimal(lon_raw);
    if (lat_dir == 'S') lat = -lat;
    if (lon_dir == 'W') lon = -lon;
    gps_fix.lat = lat;
    gps_fix.lon = lon;
    gps_fix.alt = msl_alt;
  }
  mutex_exit(&gps_fix_mutex);
}

void handle_gprmc(char *fields) {
  float utc_time = 0, lat_raw = 0, lon_raw = 0, speed_kt = 0, course = 0;
  char  status = 0, lat_dir = 0, lon_dir = 0, date[7] = {0};

  sscanf(fields, "%f,%c,%f,%c,%f,%c,%f,%f,%6s",
         &utc_time, &status,
         &lat_raw, &lat_dir,
         &lon_raw, &lon_dir,
         &speed_kt, &course, date);

  if (status != 'A') return;

  const float kKnotToMps = 0.514444f;
  mutex_enter_blocking(&gps_fix_mutex);
  gps_fix.rmc_utc  = utc_time;
  gps_fix.velocity = speed_kt * kKnotToMps;
  gps_fix.heading  = course;
  memcpy(gps_fix.date, date, sizeof(gps_fix.date));
  mutex_exit(&gps_fix_mutex);
}

void handle_gpgsv(char *fields) {
  char *field = strtok(fields, ",");
  if (!field) return;
  int total_msgs = atoi(field);

  field = strtok(nullptr, ",");
  if (!field) return;
  int msg_num = atoi(field);

  field = strtok(nullptr, ",");
  if (!field) return;
  int sats_in_view = atoi(field);

  if (total_msgs <= 0 || msg_num <= 0) return;

  int remaining = sats_in_view - (msg_num - 1) * 4;
  if (remaining > 4) remaining = 4;
  if (remaining < 0) remaining = 0;

  const uint8_t kMaxSats = sizeof(gps_fix.sats) / sizeof(gps_fix.sats[0]);

  mutex_enter_blocking(&gps_fix_mutex);
  gps_fix.sats_in_view = (uint8_t)sats_in_view;
  gps_fix.gsv_millis   = millis();
  if (msg_num == 1) gps_fix.sat_count = 0;

  for (int i = 0; i < remaining; i++) {
    char *prn_s  = strtok(nullptr, ",");
    char *elev_s = strtok(nullptr, ",");
    char *azim_s = strtok(nullptr, ",");
    char *snr_s  = strtok(nullptr, ",*");
    if (!prn_s) break;

    if (gps_fix.sat_count < kMaxSats) {
      GpsSatellite &sat = gps_fix.sats[gps_fix.sat_count++];
      sat.prn       = (uint8_t)atoi(prn_s);
      sat.elevation = elev_s ? (uint8_t)atoi(elev_s) : 0;
      sat.azimuth   = azim_s ? (uint16_t)atoi(azim_s) : 0;
      sat.snr       = (snr_s && snr_s[0] != '\0') ? (uint8_t)atoi(snr_s) : 0;
    }
  }
  mutex_exit(&gps_fix_mutex);
}

void handle_line(char *line) {
  if (line[0] != '$') return;

  char *comma = strchr(line, ',');
  if (!comma) return;
  *comma  = '\0';
  char *fields = comma + 1;
  const char *type = line + 1;

  if (strcmp(type, "GPGGA") == 0) handle_gpgga(fields);
  else if (strcmp(type, "GPRMC") == 0) handle_gprmc(fields);
  else if (strcmp(type, "GPGSV") == 0) handle_gpgsv(fields);
}

}  // namespace

void gps_async_init(void) {
  mutex_init(&gps_fix_mutex);
}

void gps_async_begin(Gps1818mk *gps) {
  gps_driver = gps;
}

void gps_service(void) {
  if (!gps_driver) return;

  while (gps_driver->is_data_available()) {
    int b = gps_driver->read_byte();
    if (b < 0) break;

    if (b == '\r') continue;
    if (b == '\n') {
      line_buf[line_len] = '\0';
      handle_line(line_buf);
      line_len = 0;
      continue;
    }

    if (line_len < kLineBufLen - 1) {
      line_buf[line_len++] = (char)b;
    } else {
      line_len = 0;
    }
  }
}

bool gps_get_latest(GpsFix *out) {
  if (!out) return false;
  mutex_enter_blocking(&gps_fix_mutex);
  *out = gps_fix;
  mutex_exit(&gps_fix_mutex);
  return out->has_fix;
}
