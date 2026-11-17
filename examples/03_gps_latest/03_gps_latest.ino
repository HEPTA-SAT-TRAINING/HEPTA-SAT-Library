// Put this library in the sketch's src/ folder, like the existing Lab sketches.
#include "src/HeptaSat.h"

HeptaCdh cdh;
HeptaEps eps;
HeptaSensor sensor;
uint32_t last_gga = 0;
uint32_t last_rmc = 0;

void setup() {
  cdh.begin();
  eps.init();
  sensor.begin();
}

void loop() {
  GpsSnapshot snapshot;
  if (sensor.gps_get_latest(&snapshot)) {
    if (snapshot.has_gga && snapshot.gga_sequence != last_gga) {
      last_gga = snapshot.gga_sequence;
      if (snapshot.position_valid) {
        cdh.printf("GPS lat=%.6f lon=%.6f alt=%.2f\n",
                   snapshot.gga.lat, snapshot.gga.lon, snapshot.gga.alt);
      } else {
        cdh.println("GPS: no fix");
      }
    }
    if (snapshot.has_rmc && snapshot.rmc_sequence != last_rmc) {
      last_rmc = snapshot.rmc_sequence;
      if (snapshot.velocity_valid) cdh.printf("GPS speed=%.2f m/s\n", snapshot.rmc.velocity);
    }
  }
  // Inspect status even when no GGA/RMC has arrived yet.
  if (snapshot.receiver_state == GpsReceiverState::ResourceError) {
    cdh.println("GPS: receiver resources unavailable");
  }
  // IMU, telemetry and application work can run here without waiting for GPS.
  // Keep the interval well below the 2048-byte RX capacity (~2.1 s at 9600 8N1).
  delay(10);
}
