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

#include <SD.h>
#include <SPI.h>

#include "src/drv/adc_mcp3208.h"
#include "src/drv/imu9axis_bno055.h"
#include "src/drv/gps_gp1818mk.h"
#include "src/drv/camera_c1098.h"

AdcMcp3208 adc;
Bno055 bno055;
Gps1818mk gps;
CameraC1098 cam;

void test_sd(void) {
  // BBM検証用．PCBで検証するときは変える
  if (!SD.begin(17)) {
    Serial.println("SD Card initialization failed!");
    return;
  }

  Serial.println("SD Card initialized successfully.");

  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open root directory.");
    return;
  }

  root.close();
}


void test_mcp3208(void) {
  adc.begin(17); // CS pin
  for(uint8_t i = 0; i < 8; i++) {
    uint16_t data = adc.get_raw_data(i);
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(data);
  }
}

void test_camera(void) {
  if (cam.begin(C1098_BAUD_RATE_115200, C1098_JPEG_SIZE_VGA)) {
    cam.take_picture();
  }
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
