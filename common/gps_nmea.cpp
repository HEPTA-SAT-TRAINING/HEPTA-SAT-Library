#include "gps_nmea.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace {
bool number(const char* s, float* value) {
  if (!s || !*s) return false;
  // NMEA decimal fields do not contain whitespace, exponents, NaN or infinity.
  const char* p = s;
  if (*p == '-' || *p == '+') ++p;
  bool digit = false, dot = false;
  for (; *p; ++p) {
    if (*p >= '0' && *p <= '9') digit = true;
    else if (*p == '.' && !dot) dot = true;
    else return false;
  }
  if (!digit) return false;
  char* end = nullptr;
  *value = strtof(s, &end);
  return end && !*end && isfinite(*value);
}

bool integer(const char* s, unsigned max, uint8_t* value) {
  if (!s || !*s) return false;
  unsigned n = 0;
  for (; *s; ++s) {
    if (*s < '0' || *s > '9') return false;
    n = n * 10 + unsigned(*s - '0');
    if (n > max) return false;
  }
  *value = static_cast<uint8_t>(n);
  return true;
}

bool utc(const char* s, float* value) {
  // Preserve hhmmss.sss as float for compatibility; validate using double.
  if (!number(s, value)) return false;
  double t = strtod(s, nullptr);
  if (t < 0 || t >= 240000) return false;
  int h = int(t / 10000), m = int(t / 100) % 100;
  double sec = t - h * 10000 - m * 100;
  return t >= 0 && h < 24 && m < 60 && sec >= 0 && sec < 61;
}

bool coordinate(const char* s, const char* dir, bool latitude, float* out) {
  float raw;
  if (!number(s, &raw) || raw < 0 || strlen(dir) != 1) return false;
  if (latitude ? (*dir != 'N' && *dir != 'S') : (*dir != 'E' && *dir != 'W')) return false;
  double n = strtod(s, nullptr);
  if (n > (latitude ? 9000.0 : 18000.0)) return false;
  int degrees = int(n / 100);
  double minutes = n - degrees * 100;
  int limit = latitude ? 90 : 180;
  if (degrees > limit || minutes >= 60 || (degrees == limit && minutes != 0)) return false;
  *out = float(degrees + minutes / 60);
  if (*dir == 'S' || *dir == 'W') *out = -*out;
  return true;
}

bool date(const char* s, char* out) {
  if (strlen(s) != 6) return false;
  for (int i = 0; i < 6; ++i) if (s[i] < '0' || s[i] > '9') return false;
  int day = (s[0] - '0') * 10 + s[1] - '0';
  int month = (s[2] - '0') * 10 + s[3] - '0';
  int year = (s[4] - '0') * 10 + s[5] - '0';
  const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12) return false;
  int last = days[month - 1] + ((month == 2 && year % 4 == 0) ? 1 : 0);
  if (day < 1 || day > last) return false;
  memcpy(out, s, 7);
  return true;
}

int hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

bool gga(char** f, size_t n, GpggaData* d) {
  if (n < 15 || !utc(f[1], &d->utc_time) ||
      !integer(f[6], 8, &d->fix_quality) || !integer(f[7], 99, &d->sat_num)) return false;
  if (*f[8] && (!number(f[8], &d->hdop) || d->hdop < 0)) return false;
  if (!d->fix_quality) return true; // Empty coordinates are normal before a fix.
  return coordinate(f[2], f[3], true, &d->lat) &&
         coordinate(f[4], f[5], false, &d->lon) &&
         number(f[8], &d->hdop) && number(f[9], &d->alt) && !strcmp(f[10], "M");
}

bool rmc(char** f, size_t n, GprmcData* d, bool* valid, bool* heading_valid) {
  if (n < 10 || !utc(f[1], &d->utc_time) ||
      (strcmp(f[2], "A") && strcmp(f[2], "V"))) return false;
  if (*f[9] && !date(f[9], d->date)) return false;
  *valid = !strcmp(f[2], "A");
  if (!*valid) return true;
  float lat, lon, knots;
  if (!coordinate(f[3], f[4], true, &lat) || !coordinate(f[5], f[6], false, &lon) ||
      !number(f[7], &knots) || knots < 0 || !date(f[9], d->date)) return false;
  if (*f[8]) {
    if (!number(f[8], &d->heading) || d->heading < 0 || d->heading >= 360) return false;
    *heading_valid = true;
  }
  d->velocity = knots * 0.514444f;
  return isfinite(d->velocity);
}
} // namespace

void GpsNmeaParser::reset() {
  snapshot_ = GpsSnapshot{};
  discard_partial();
}

void GpsNmeaParser::discard_partial() { length_ = 0; }

GpsNmeaParser::Event GpsNmeaParser::pending_type() const {
  if (length_ < 6) return Event::None;
  if (!memcmp(line_, "$GPGGA", 6)) return Event::Gga;
  if (!memcmp(line_, "$GPRMC", 6)) return Event::Rmc;
  return Event::None;
}

GpsNmeaParser::Event GpsNmeaParser::feed(uint8_t byte, uint32_t parsed_ms) {
  if (byte == '$') { length_ = 0; line_[length_++] = '$'; return Event::None; }
  if (!length_) return Event::None;
  if (byte == '\n') {
    if (length_ && line_[length_ - 1] == '\r') --length_;
    line_[length_] = '\0';
    Event result = finish(parsed_ms);
    length_ = 0;
    return result;
  }
  if (!byte || length_ >= kLineSize - 1) {
    Event type = pending_type();
    ++snapshot_.parse_error_count;
    length_ = 0;
    return type == Event::Gga ? Event::InvalidGga :
           type == Event::Rmc ? Event::InvalidRmc : Event::None;
  }
  line_[length_++] = char(byte);
  return Event::None;
}

GpsNmeaParser::Event GpsNmeaParser::finish(uint32_t parsed_ms) {
  Event type = pending_type();
  if (type == Event::None) return Event::None;
  Event invalid = type == Event::Gga ? Event::InvalidGga : Event::InvalidRmc;
  auto fail = [&]() { ++snapshot_.parse_error_count; return invalid; };
  if (line_[6] != ',') return fail();
  char* star = strchr(line_, '*');
  if (!star || strlen(star) != 3 || hex(star[1]) < 0 || hex(star[2]) < 0) return fail();
  uint8_t sum = 0;
  for (char* p = line_ + 1; p != star; ++p) sum ^= uint8_t(*p);
  if (sum != (hex(star[1]) * 16 + hex(star[2]))) return fail();
  *star = '\0';
  char* fields[24];
  size_t count = 0;
  fields[count++] = line_;
  for (char* p = line_; *p; ++p) {
    if (*p == ',') {
      *p = '\0';
      if (count == sizeof(fields) / sizeof(fields[0])) return fail();
      fields[count++] = p + 1;
    }
  }
  if (type == Event::Gga) {
    GpggaData d = {};
    if (!gga(fields, count, &d)) return fail();
    snapshot_.gga = d;
    snapshot_.has_gga = true;
    snapshot_.position_valid = d.fix_quality > 0;
    ++snapshot_.gga_sequence;
    snapshot_.gga_parsed_ms = parsed_ms;
  } else {
    GprmcData d = {};
    bool valid = false, heading_valid = false;
    if (!rmc(fields, count, &d, &valid, &heading_valid)) return fail();
    snapshot_.rmc = d;
    snapshot_.has_rmc = true;
    snapshot_.velocity_valid = valid;
    snapshot_.heading_valid = heading_valid;
    ++snapshot_.rmc_sequence;
    snapshot_.rmc_parsed_ms = parsed_ms;
  }
  return type;
}
