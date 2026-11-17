#include "src/HeptaSat.h"
#include <string.h>
#include <stdlib.h>

HeptaEps eps;
HeptaSensor sensor;
HeptaCom com;
GpsSnapshot last_snapshot;
bool imu_ok = false, com_ok = false, raw_mode = false;
uint32_t next_sample = 0, next_report = 0;
uint32_t calls = 0, imu_samples = 0, imu_failures = 0, missed = 0;
uint32_t max_gps_us = 0, max_imu_us = 0, max_late_us = 0;
uint64_t sum_gps_us = 0;
char command[64];
uint8_t command_length = 0;

void reset_stats() {
  calls = imu_samples = imu_failures = missed = 0;
  max_gps_us = max_imu_us = max_late_us = 0;
  sum_gps_us = 0;
  next_sample = micros() + 10000;
  next_report = millis() + 1000;
}

void snapshot_line(const char* label, uint32_t elapsed = 0) {
  Serial.printf("%s ms=%lu state=%u gga=%lu rmc=%lu pos=%u vel=%u quality=%u sats=%u overrun=%lu rxerr=%lu parseerr=%lu elapsed_us=%lu heap=%d\n",
    label, millis(), unsigned(last_snapshot.receiver_state),
    last_snapshot.gga_sequence, last_snapshot.rmc_sequence,
    last_snapshot.position_valid, last_snapshot.velocity_valid,
    last_snapshot.gga.fix_quality, last_snapshot.gga.sat_num,
    last_snapshot.rx_overrun_count, last_snapshot.rx_error_count,
    last_snapshot.parse_error_count, elapsed, rp2040.getFreeHeap());
}

void poll_gps() {
  uint32_t start = micros();
  sensor.gps_get_latest(&last_snapshot);
  uint32_t elapsed = micros() - start;
  ++calls;
  sum_gps_us += elapsed;
  if (elapsed > max_gps_us) max_gps_us = elapsed;
}

void handle_command() {
  if (!strcmp(command, "INFO")) {
    Serial.printf("INFO imu=%u xbee=%u raw=%u clock=%lu\n", imu_ok, com_ok, raw_mode, rp2040.f_cpu());
    snapshot_line("SNAP");
  } else if (!strcmp(command, "RAW")) {
    raw_mode = true;
    sensor.gps_end(); sensor.gps_begin();
    Serial.println("RAW_BEGIN");
  } else if (!strcmp(command, "NORMAL")) {
    raw_mode = false;
    sensor.gps_end(); sensor.gps_begin();
    last_snapshot = GpsSnapshot{};
    reset_stats();
    Serial.println("NORMAL_BEGIN");
  } else if (!strcmp(command, "RESET_STATS")) {
    reset_stats();
    Serial.println("STATS_RESET");
  } else if (!strncmp(command, "PAUSE ", 6)) {
    uint32_t duration = strtoul(command + 6, nullptr, 10);
    if (duration > 30000) duration = 30000;
    Serial.printf("PAUSE_BEGIN duration_ms=%lu\n", duration);
    Serial.flush();
    delay(duration);
    uint32_t start = micros();
    sensor.gps_get_latest(&last_snapshot);
    snapshot_line("PAUSE_END", micros() - start);
    reset_stats();
  } else if (!strncmp(command, "RESTART ", 8)) {
    uint32_t count = strtoul(command + 8, nullptr, 10);
    if (count > 100) count = 100;
    uint32_t failures = 0;
    int before = rp2040.getFreeHeap();
    for (uint32_t i = 0; i < count; ++i) {
      sensor.gps_end();
      GpsSnapshot stopped;
      bool cached = sensor.gps_get_latest(&stopped);
      if (cached || stopped.has_gga || stopped.has_rmc || stopped.receiver_state != GpsReceiverState::Stopped) ++failures;
      sensor.gps_begin();
      sensor.gps_get_latest(&last_snapshot);
      if (last_snapshot.receiver_state != GpsReceiverState::Ready) ++failures;
    }
    Serial.printf("RESTART_END count=%lu failures=%lu heap_before=%d heap_after=%d\n", count, failures, before, rp2040.getFreeHeap());
    reset_stats();
  } else if (!strcmp(command, "LEGACY")) {
    GpggaData gga = {};
    gga.lat = 123.0f;
    GprmcData rmc = {};
    rmc.velocity = 123.0f;
    uint32_t start = millis();
    bool g = sensor.gps_get_gpgga(&gga);
    uint32_t g_ms = millis() - start;
    start = millis();
    bool r = sensor.gps_get_gprmc(&rmc);
    uint32_t r_ms = millis() - start;
    float lat=123, lon=123, alt=123, speed=123, heading=123;
    start = millis();
    bool all = sensor.gps_get_all(&lat, &lon, &alt, &speed, &heading);
    uint32_t all_ms = millis() - start;
    Serial.printf("LEGACY_END gga_ok=%u rmc_ok=%u all_ok=%u gga_ms=%lu rmc_ms=%lu all_ms=%lu gga_unchanged=%u rmc_unchanged=%u all_unchanged=%u\n",
      g, r, all, g_ms, r_ms, all_ms, gga.lat==123.0f, rmc.velocity==123.0f,
      lat==123 && lon==123 && alt==123 && speed==123 && heading==123);
    poll_gps(); snapshot_line("LEGACY_SNAP");
    reset_stats();
  } else if (!strcmp(command, "SNAP")) {
    poll_gps(); snapshot_line("SNAP");
  } else {
    Serial.println("UNKNOWN_COMMAND");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(0, OUTPUT); digitalWrite(0, HIGH);
  pinMode(3, OUTPUT); digitalWrite(3, HIGH);
  pinMode(17, OUTPUT); digitalWrite(17, HIGH);
  SPI.setRX(16); SPI.setTX(19); SPI.setSCK(18); SPI.begin();
  eps.init();
  delay(800);
  imu_ok = sensor.begin();
  com_ok = com.begin(); // Allocate XBee PIO only; no send or AT operation.
  reset_stats();
  Serial.printf("READY imu=%u xbee=%u\n", imu_ok, com_ok);
}

void loop() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      command[command_length] = 0;
      handle_command();
      command_length = 0;
    } else if (command_length < sizeof(command)-1) {
      command[command_length++] = char(c);
    }
  }
  if (raw_mode) {
    while (sensor.gps_is_data_available()) {
      int c = sensor.gps_read_byte();
      if (c >= 0) Serial.write(uint8_t(c));
    }
    return;
  }
  uint32_t now = micros();
  if (int32_t(now - next_sample) >= 0) {
    uint32_t late = now - next_sample;
    if (late > max_late_us) max_late_us = late;
    missed += late / 10000;
    next_sample += (late / 10000 + 1) * 10000;
    poll_gps();
    if (imu_ok) {
      float ax, ay, az;
      uint32_t start = micros();
      bool ok = sensor.get_acceleration(&ax, &ay, &az);
      uint32_t elapsed = micros() - start;
      if (elapsed > max_imu_us) max_imu_us = elapsed;
      if (ok) ++imu_samples; else ++imu_failures;
    }
  }
  if (Serial && int32_t(millis() - next_report) >= 0) {
    next_report = millis() + 1000;
    Serial.printf("STATS ms=%lu calls=%lu imu=%lu imu_fail=%lu missed=%lu late_us=%lu max_gps_us=%lu mean_gps_us=%lu max_imu_us=%lu heap=%d\n",
      millis(), calls, imu_samples, imu_failures, missed, max_late_us,
      max_gps_us, calls ? uint32_t(sum_gps_us / calls) : 0, max_imu_us, rp2040.getFreeHeap());
    snapshot_line("STATE");
  }
}
