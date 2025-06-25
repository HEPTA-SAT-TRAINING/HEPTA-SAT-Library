/**
 * @file component_test.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-06-05
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "component_test.h"

#include "src/sensor/adc_mcp3208.h"
#include "src/sensor/camera_c1098.h"
#include "src/sensor/gps_gp1818mk.h"
#include "src/sensor/imu9axis_bno055.h"


Bno055 bno055;
CameraC1098 cam;
Gps1818mk gps;


void test_camera(void) {
  cam.begin(C1098_BAUD_RATE_115200, C1098_JPEG_SIZE_VGA);
  cam.take_picture();
}

void test_bno055(void) {
  bno055.begin();
  bno055.print_acc();
  bno055.print_gyro();
  bno055.print_mag();
}

void test_gps(void) {
  float lat, lon, alt;
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
