/** GP-1818MK: DMA reception with on-demand NMEA parsing. */
#include "gps_gp1818mk.h"

void Gps1818mk::begin(void) {
  end();
  _parser.reset();
  _seen_overruns = _seen_errors = 0;
  _receiver.begin(_rx_pin, _tx_pin);
}

void Gps1818mk::end(void) {
  _receiver.end();
  _parser.reset();
  _seen_overruns = _seen_errors = 0;
}

void Gps1818mk::sync_receiver(void) {
  if (_seen_overruns != _receiver.overrun_count() || _seen_errors != _receiver.error_count()) {
    _parser.discard_partial();
    _seen_overruns = _receiver.overrun_count();
    _seen_errors = _receiver.error_count();
  }
}

bool Gps1818mk::get_latest(GpsSnapshot* out) {
  if (!out) return false;
  // Freeze the endpoint, not the DMA: newly arriving bytes belong to the next
  // call. The work is bounded by kCapacity; there are no reception waits.
  uint64_t limit = _receiver.received();
  sync_receiver();
  for (size_t work = 0; work < GpsDmaRx::kCapacity; ++work) {
    int byte = _receiver.read_until(limit);
    sync_receiver();
    if (byte == -2) continue;
    if (byte < 0) break;
    _parser.feed(uint8_t(byte), millis());
  }
  *out = _parser.snapshot();
  out->receiver_state = _receiver.state();
  out->rx_overrun_count = _receiver.overrun_count();
  out->rx_error_count = _receiver.error_count();
  return out->has_gga || out->has_rmc;
}

int Gps1818mk::read_byte(void) {
  // Raw and parsed reads share one cursor. Do not join an old parser prefix
  // onto bytes left after a raw read; the next '$' starts a fresh sentence.
  _parser.discard_partial();
  uint64_t limit = _receiver.received();
  sync_receiver();
  int byte = _receiver.read_until(limit);
  sync_receiver();
  return byte >= 0 ? byte : -1;
}

bool Gps1818mk::is_data_available(void) {
  bool result = _receiver.available() > 0;
  sync_receiver();
  return result;
}

bool Gps1818mk::wait_sentence(GpsNmeaParser::Event target, bool initial_wait) {
  if (_receiver.state() != GpsReceiverState::Ready) return false;
  if (initial_wait) {
    uint32_t start = millis();
    while (!_receiver.available()) {
      sync_receiver();
      if (uint32_t(millis() - start) >= 1000) return false;
      delay(1);
    }
  }
  uint32_t start = millis(), last_byte = start;
  bool body = _parser.pending_type() == target;
  while (true) {
    uint64_t limit = _receiver.received();
    sync_receiver();
    int byte = _receiver.read_until(limit);
    sync_receiver();
    if (byte >= 0) {
      last_byte = millis();
      GpsNmeaParser::Event event = _parser.feed(uint8_t(byte), last_byte);
      if (event == target) return true;
      if ((target == GpsNmeaParser::Event::Gga && event == GpsNmeaParser::Event::InvalidGga) ||
          (target == GpsNmeaParser::Event::Rmc && event == GpsNmeaParser::Event::InvalidRmc)) return false;
      body = _parser.pending_type() == target;
    } else if (byte == -2) {
      body = false;
    }
    if (body) {
      if (uint32_t(millis() - last_byte) >= 200) { _parser.discard_partial(); return false; }
    } else if (uint32_t(millis() - start) >= 5000) return false;
    // Preserve the old blocking contract, but allow USB/system work to run.
    if (byte < 0) delay(1);
  }
}

bool Gps1818mk::get_gpgga(GpggaData* out) {
  if (!out || !wait_sentence(GpsNmeaParser::Event::Gga)) return false;
  if (!_parser.snapshot().position_valid) return false;
  *out = _parser.snapshot().gga;
  return true;
}

bool Gps1818mk::get_gprmc(GprmcData* out) {
  if (!out || !wait_sentence(GpsNmeaParser::Event::Rmc)) return false;
  if (!_parser.snapshot().velocity_valid) return false;
  *out = _parser.snapshot().rmc;
  return true;
}

bool Gps1818mk::get_position(float* lat, float* lon, float* alt) {
  if (!lat || !lon || !alt) return false;
  GpggaData d;
  if (!get_gpgga(&d)) return false;
  *lat = d.lat; *lon = d.lon; *alt = d.alt;
  return true;
}

bool Gps1818mk::get_velocity(float* velocity, float* heading) {
  if (!velocity || !heading) return false;
  GprmcData d;
  if (!get_gprmc(&d)) return false;
  *velocity = d.velocity; *heading = d.heading;
  return true;
}

bool Gps1818mk::get_all(float* lat, float* lon, float* alt, float* velocity, float* heading) {
  if (!lat || !lon || !alt || !velocity || !heading) return false;
  GpggaData gga;
  GprmcData rmc;
  if (!get_gpgga(&gga) || !wait_sentence(GpsNmeaParser::Event::Rmc, false) ||
      !_parser.snapshot().velocity_valid) return false;
  rmc = _parser.snapshot().rmc;
  *lat = gga.lat; *lon = gga.lon; *alt = gga.alt;
  *velocity = rmc.velocity; *heading = rmc.heading;
  return true;
}
