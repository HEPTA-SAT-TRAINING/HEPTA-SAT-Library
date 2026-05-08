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

#include "src/drv/adc_mcp3208.h"
#include "src/drv/imu9axis_bno055.h"
#include "src/drv/gps_gp1818mk.h"
#include "src/drv/camera_c1098.h"
#include "src/drv/unit_rolleri2c.hpp"
#include <Wire.h>

AdcMcp3208 adc;
Bno055 bno055;
Gps1818mk gps;
CameraC1098 cam;
UnitRollerI2C motor;

void test_sd(HeptaCdh &cdh) {
  File file = cdh.create_file("test.txt");
  if (!file) {
    Serial.println("Failed to create file.");
    return;
  }

  cdh.write_file(file, "Hello, this is a test file.\n");
  file.close();
  Serial.println("File created successfully.");

  file = cdh.open_file("test.txt");
  if (!file) {
    Serial.println("Failed to open file.");
    return;
  }

  Serial.println("File content:");
  while (file.available()) {
    Serial.write(cdh.read_file(file));
  }
  file.close();
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
  gps.begin();
  // gps.read_raw();
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

void test_motor(void) {
  uint32_t p, i, d;         // Defines a variable to store the PID value
  uint8_t r, g, b;

  // Wire SDA:4, SCL:5
  motor.begin(&Wire, 0x64, 400000);

  // current mode
  motor.setMode(ROLLER_MODE_ENCODER);
  motor.setCurrent(100000);
  motor.setOutput(1);
  Serial.printf("current: %d\n", motor.getCurrent());
  delay(100);
  Serial.printf("actualCurrent: %f\n", motor.getCurrentReadback() / 100.0f);
  delay(2000);

  // position mode
  motor.setOutput(0);
  motor.setMode(ROLLER_MODE_POSITION);
  motor.setPos(2000000);
  motor.setPosMaxCurrent(100000);
  motor.setOutput(1);
  motor.getPosPID(&p, &i, &d);
  Serial.printf("PosPID  P: %3.8f  I: %3.8f  D: %3.8f\n", p / 100000.0, i / 10000000.0, d / 100000.0);
  delay(100);
  Serial.printf("pos: %d\n", motor.getPos());
  delay(100);
  Serial.printf("posMaxCurrent: %f\n", motor.getPosMaxCurrent() / 100.0f);
  delay(100);
  Serial.printf("actualPos: %f\n", motor.getPosReadback() / 100.0f);
  delay(2000);

  // speed mode
  motor.setOutput(0);
  motor.setMode(ROLLER_MODE_SPEED);
  motor.setSpeed(700 * 100);
  motor.setSpeedMaxCurrent(100000);
  motor.setOutput(1);
  motor.getSpeedPID(&p, &i, &d);
  Serial.printf("SpeedPID  P: %3.8f  I: %3.8f  D: %3.8f\n", p / 100000.0, i / 10000000.0, d / 100000.0);
  delay(100);
  Serial.printf("speed: %d\n", motor.getSpeed());
  delay(100);
  Serial.printf("speedMaxCurrent: %f\n", motor.getSpeedMaxCurrent() / 100.0f);
  delay(100);
  Serial.printf("actualSpeed: %f\n", motor.getSpeedReadback() / 100.0f);
  delay(2000);

  // encoder mode
  motor.setOutput(0);
  motor.setMode(ROLLER_MODE_ENCODER);
  motor.setDialCounter(240000);
  motor.setOutput(1);
  Serial.printf("DialCounter:%d\n", motor.getDialCounter());
  delay(2000);
  Serial.printf("temp:%d\n", motor.getTemp());
  delay(100);
  Serial.printf("Vin:%3.2f\n", motor.getVin() / 100.0);
  delay(100);
  Serial.printf("RGBBrightness:%d\n", motor.getRGBBrightness());
  delay(1000);
  motor.setRGBBrightness(100);
  delay(100);
  // motor.setRGBMode(ROLLER_RGB_MODE_USER_DEFINED);
  // delay(1000);
  // motor.setRGB(TFT_WHITE);
  // delay(1000);
  // motor.setRGB(TFT_BLUE);
  // delay(2000);
  // motor.setRGB(TFT_YELLOW);
  // delay(2000);
  // motor.setRGB(TFT_RED);
  // delay(2000);
  // motor.setRGBMode(ROLLER_RGB_MODE_DEFAULT);
  // delay(100);
  motor.setKeySwitchMode(1);
  delay(100);
  Serial.printf("I2CAddress:%d\n", motor.getI2CAddress());
  delay(100);
  Serial.printf("485 BPS:%d\n", motor.getBPS());
  delay(100);
  Serial.printf("485 motor id:%d\n", motor.getMotorID());
  delay(100);
  Serial.printf("motor output:%d\n", motor.getOutputStatus());
  delay(100);
  Serial.printf("SysStatus:%d\n", motor.getSysStatus());
  delay(100);
  Serial.printf("ErrorCode:%d\n", motor.getErrorCode());
  delay(100);
  Serial.printf("Button switching mode enable:%d\n", motor.getKeySwitchMode());
  delay(100);
  motor.getRGB(&r, &g, &b);
  Serial.printf("RGB-R: 0x%02X  RGB-G: 0x%02X  RGB-B: 0x%02X\n", r, g, b);
}
