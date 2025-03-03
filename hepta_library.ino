#include "src/cdh/hepta_cdh.h"
#include "src/eps/hepta_eps.h"
#include "src/sensor/hepta_sensor.h"
#include "src/com/hepta_com.h"

#include <camera_c1098.h>
#include <gps_gp1818mk.h>

HeptaSensor sensor;
CameraC1098 cam;
Gps1818mk gps;

void setup() {
  Serial.begin(9600);
  Serial1.setFIFOSize(128);
  Serial1.begin(9600);
}

void loop() {
  float lat, lon, alt;
  // while(gps.is_data_available());
  if(gps.get_position(&lat, &lon, &alt)) {
    Serial.println("-------------------");
    Serial.print("Lat: ");
    Serial.println(lat, 6);
    Serial.print("Lon: ");
    Serial.println(lon, 6);
    Serial.print("Alt: ");
    Serial.println(alt, 1);
    Serial.println("-------------------");
  }
}
