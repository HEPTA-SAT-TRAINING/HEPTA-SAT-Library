#ifdef HEPTA_GPS_HOST_TEST
#include "../../drv/gps_gp1818mk.h"
#include "fake_rx.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {
int checks = 0;
void check(bool value, const char* text, int line) {
  ++checks;
  if (!value) { std::cerr << "FAIL line " << line << ": " << text << '\n'; std::exit(1); }
}
#define CHECK(x) check(bool(x), #x, __LINE__)
bool near(float a, float b, float tolerance = 0.0001f) { return std::fabs(a - b) < tolerance; }
std::string sentence(const std::string& body) {
  unsigned sum = 0;
  for (unsigned char c : body) sum ^= c;
  const char* digits = "0123456789ABCDEF";
  return "$" + body + "*" + digits[sum >> 4] + digits[sum & 15] + "\r\n";
}
const std::string gga = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
const std::string rmc = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W";
const std::string no_gga = "GPGGA,123520,,,,,0,00,99.99,,,,,,";
const std::string no_rmc = "GPRMC,123520,V,,,,,,,230394,,,N";

GpsNmeaParser::Event feed(GpsNmeaParser& p, const std::string& s, uint32_t at = 100) {
  auto result = GpsNmeaParser::Event::None;
  for (unsigned char c : s) {
    auto e = p.feed(c, at);
    if (e != GpsNmeaParser::Event::None) result = e;
  }
  return result;
}

void parser_tests() {
  using E = GpsNmeaParser::Event;
  GpsNmeaParser p;
  CHECK(!p.snapshot().has_gga);
  std::string g = sentence(gga);
  CHECK(feed(p, g.substr(0, 17)) == E::None);
  CHECK(!p.snapshot().has_gga);
  CHECK(feed(p, g.substr(17), 500) == E::Gga);
  CHECK(p.snapshot().gga_sequence == 1 && p.snapshot().gga_parsed_ms == 500);
  CHECK(near(p.snapshot().gga.lat, 48.1173f));
  CHECK(near(p.snapshot().gga.lon, 11.516667f));
  CHECK(near(p.snapshot().gga.alt, 545.4f));
  CHECK(p.snapshot().gga.sat_num == 8 && p.snapshot().position_valid);
  CHECK(feed(p, sentence(rmc)) == E::Rmc);
  CHECK(near(p.snapshot().rmc.velocity, 22.4f * 0.514444f));
  CHECK(near(p.snapshot().rmc.heading, 84.4f));
  CHECK(!strcmp(p.snapshot().rmc.date, "230394"));
  CHECK(p.snapshot().heading_valid);
  CHECK(feed(p, sentence("GPRMC,123519,A,4807.038,N,01131.000,E,0.0,,230394,,")) == E::Rmc);
  CHECK(p.snapshot().velocity_valid && !p.snapshot().heading_valid);
  CHECK(feed(p, sentence(no_gga)) == E::Gga);
  CHECK(p.snapshot().has_gga && !p.snapshot().position_valid && p.snapshot().gga.lat == 0);
  CHECK(feed(p, sentence(no_rmc)) == E::Rmc);
  CHECK(p.snapshot().has_rmc && !p.snapshot().velocity_valid && p.snapshot().rmc.velocity == 0);
  CHECK(feed(p, sentence("GPGGA,123521,4807.038,S,01131.000,W,1,08,0.9,-10.0,M,0.0,M,,")) == E::Gga);
  CHECK(p.snapshot().gga.lat < 0 && p.snapshot().gga.lon < 0 && p.snapshot().gga.alt == -10);

  unsigned seq = p.snapshot().gga_sequence;
  std::string bad = g; bad[10] ^= 1;
  CHECK(feed(p, bad) == E::InvalidGga);
  CHECK(p.snapshot().gga_sequence == seq);
  CHECK(feed(p, "$GPGGA,123519,48,N,11,E,1,08,0.9,1,M,1,M,,\r\n") == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,123519,4860.000,N,01131.000,E,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  // A negative coordinate must not pass when strtof underflows to negative zero.
  CHECK(feed(p, sentence("GPGGA,123519,-0." + std::string(48, '0') + "1,N,01131.000,E,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,999999999999999999999999999999,N,1,2,3,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,123519,999999999999999999,N,01131.000,E,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,126099,4807.038,N,01131.000,E,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,123519,4807.038,X,01131.000,E,1,08,0.9,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPGGA,123519,4807.038,N,01131.000,E,1,08,nan,1,M,1,M,,")) == E::InvalidGga);
  CHECK(feed(p, sentence("GPRMC,123519,A,4807.038,N,01131.000,E,2.0,84.4,310294,,")) == E::InvalidRmc);
  CHECK(feed(p, sentence("GPRMC,123519,X,4807.038,N,01131.000,E,2.0,84.4,230394,,")) == E::InvalidRmc);
  CHECK(feed(p, sentence("GPRMC,123519,A,4807.038,N,01131.000,E,-2.0,84.4,230394,,")) == E::InvalidRmc);
  CHECK(feed(p, sentence("GPRMC,123519,A,4807.038,N,01131.000,E,2.0,360,230394,,")) == E::InvalidRmc);
  CHECK(feed(p, sentence("GPGSV,1,1,0")) == E::None);
  CHECK(feed(p, "$GPGGA," + std::string(200, '1')) == E::InvalidGga);
  CHECK(feed(p, "garbage\r\n$GPGGA,broken" + sentence(gga)) == E::Gga);
  CHECK(p.snapshot().parse_error_count >= 12);
  p.reset();
  CHECK(!p.snapshot().has_gga && !p.snapshot().has_rmc && p.snapshot().gga_sequence == 0);
  feed(p, g.substr(0, 10)); p.discard_partial();
  CHECK(feed(p, g.substr(10)) == E::None);
  CHECK(feed(p, g) == E::Gga);
  // Every possible split point, including CR/LF and checksum boundaries.
  for (size_t split = 0; split <= g.size(); ++split) {
    p.reset();
    feed(p, g.substr(0, split));
    feed(p, g.substr(split));
    CHECK(p.snapshot().has_gga && p.snapshot().gga_sequence == 1);
  }
}

void snapshot_tests() {
  Gps1818mk gps(13, 2);
  GpsSnapshot s;
  CHECK(!gps.get_latest(&s) && s.receiver_state == GpsReceiverState::Stopped);
  gps.begin();
  CHECK(!gps.get_latest(&s) && s.receiver_state == GpsReceiverState::Ready);
  fake_rx::inject(13, sentence(gga));
  CHECK(!gps.get_latest(nullptr)); // Null must not consume bytes.
  CHECK(gps.get_latest(&s) && s.position_valid && s.gga_sequence == 1);
  unsigned at = s.gga_parsed_ms;
  CHECK(gps.get_latest(&s) && s.gga_sequence == 1 && s.gga_parsed_ms == at);
  fake_rx::inject(13, sentence(rmc));
  CHECK(gps.get_latest(&s) && s.has_gga && s.has_rmc && s.rmc_sequence == 1);
  fake_rx::inject(13, sentence(gga) + sentence(no_gga));
  CHECK(gps.get_latest(&s) && s.gga_sequence == 3 && !s.position_valid);

  gps.begin();
  CHECK(!gps.get_latest(&s) && !s.has_rmc && s.gga_sequence == 0);
  std::string g = sentence(gga);
  fake_rx::inject(13, g.substr(0, 20));
  CHECK(!gps.get_latest(&s));
  fake_rx::inject(13, g.substr(20));
  CHECK(gps.get_latest(&s) && s.gga_sequence == 1);

  gps.begin();
  fake_rx::inject(13, sentence(gga));
  fake_rx::inject_after_read(13, sentence(rmc));
  CHECK(gps.get_latest(&s) && s.has_gga && !s.has_rmc); // Endpoint frozen at entry.
  CHECK(gps.get_latest(&s) && s.has_rmc);

  gps.begin();
  fake_rx::inject(13, g.substr(0, 20)); gps.get_latest(&s);
  fake_rx::inject(13, std::string(3000, 'x') + g.substr(20) + sentence(rmc));
  CHECK(gps.get_latest(&s) && !s.has_gga && s.has_rmc && s.rx_overrun_count == 1);

  gps.begin();
  fake_rx::inject(13, g.substr(0, 20)); gps.get_latest(&s);
  fake_rx::error(13); gps.get_latest(&s);
  fake_rx::inject(13, g.substr(20));
  CHECK(!gps.get_latest(&s) && s.rx_error_count == 1);
  fake_rx::inject(13, g);
  CHECK(gps.get_latest(&s));
  gps.end();
  CHECK(!gps.get_latest(&s) && s.receiver_state == GpsReceiverState::Stopped);
  gps.begin();
  CHECK(!gps.get_latest(&s) && s.rx_overrun_count == 0 && s.rx_error_count == 0);

  fake_rx::fail_begin = true; gps.begin();
  CHECK(!gps.get_latest(&s) && s.receiver_state == GpsReceiverState::ResourceError);
  fake_rx::fail_begin = false;

  Gps1818mk other(12, 3);
  gps.begin(); other.begin();
  fake_rx::inject(13, sentence(gga)); fake_rx::inject(12, sentence(rmc));
  CHECK(gps.get_latest(&s) && s.has_gga && !s.has_rmc);
  CHECK(other.get_latest(&s) && s.has_rmc && !s.has_gga);
}

void legacy_tests() {
  Gps1818mk gps;
  gps.begin();
  std::string bytes = sentence(gga);
  fake_rx::inject(13, bytes);
  std::string raw;
  while (gps.is_data_available()) { int c = gps.read_byte(); if (c >= 0) raw += char(c); }
  CHECK(raw == bytes && gps.read_byte() == -1);
  GpsSnapshot s;
  CHECK(!gps.get_latest(&s)); // Raw bytes were consumed, not secretly parsed.

  fake_rx::inject(13, sentence(gga) + sentence(rmc));
  GpggaData g; GprmcData r;
  CHECK(gps.get_gpgga(&g) && gps.get_gprmc(&r));
  CHECK(near(g.lat, 48.1173f) && !strcmp(r.date, "230394"));
  CHECK(gps.get_latest(&s) && s.gga_sequence == 1 && s.rmc_sequence == 1);
  uint32_t before = fake_rx::clock_ms;
  GpggaData unchanged = g;
  CHECK(!gps.get_gpgga(&g)); // A cache does not satisfy a legacy wait.
  CHECK(fake_rx::clock_ms - before >= 1000 && !memcmp(&g, &unchanged, sizeof(g)));

  fake_rx::inject_after_delay(13, sentence(gga));
  CHECK(gps.get_gpgga(&g)); // Incoming data during the legacy wait.
  fake_rx::inject(13, sentence(no_gga));
  unchanged = g;
  CHECK(!gps.get_gpgga(&g) && !memcmp(&g, &unchanged, sizeof(g)));
  fake_rx::inject(13, sentence(no_rmc));
  GprmcData old_r = r;
  CHECK(!gps.get_gprmc(&r) && !memcmp(&r, &old_r, sizeof(r)));

  float lat = 9, lon = 9, alt = 9, speed = 9, heading = 9;
  fake_rx::inject(13, sentence(gga) + sentence(rmc));
  CHECK(gps.get_all(&lat, &lon, &alt, &speed, &heading));
  CHECK(near(lat, 48.1173f) && near(speed, 22.4f * 0.514444f));
  lat = lon = alt = speed = heading = 9;
  fake_rx::inject(13, sentence(gga) + sentence(no_rmc));
  CHECK(!gps.get_all(&lat, &lon, &alt, &speed, &heading));
  CHECK(lat == 9 && lon == 9 && alt == 9 && speed == 9 && heading == 9);
  fake_rx::inject(13, sentence(gga));
  CHECK(gps.get_position(&lat, &lon, &alt));
  fake_rx::inject(13, sentence(rmc));
  CHECK(gps.get_velocity(&speed, &heading));

  gps.begin();
  fake_rx::inject(13, "$GPGGA,123");
  before = fake_rx::clock_ms;
  CHECK(!gps.get_gpgga(&g));
  CHECK(fake_rx::clock_ms - before >= 200 && fake_rx::clock_ms - before < 1000);
  gps.begin();
  fake_rx::inject(13, sentence("GPGSV,1,1,0"));
  before = fake_rx::clock_ms;
  CHECK(!gps.get_gpgga(&g) && fake_rx::clock_ms - before >= 5000);
  fake_rx::clock_ms = UINT32_MAX - 100;
  CHECK(!gps.get_gpgga(&g)); // millis() rollover.
  CHECK(!gps.get_gpgga(nullptr) && !gps.get_gprmc(nullptr));
}
}

int main() {
  parser_tests(); snapshot_tests(); legacy_tests();
  std::cout << "PASS: " << checks << " GPS checks (production parser/driver, fake RX/time).\n";
}
#endif
