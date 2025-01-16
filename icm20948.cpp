/**
 * @file icm20948.cpp
 * @author Masaki Naito
 * @brief Arduino Library for ICM-20948
 * @version 0.1
 * @date 2025-01-16
 * 
 * @copyright UNISEC all rights reserved.
 */

#include "icm20948.h"

#include <Wire.h>

void Icm20948::begin(void) {
  uint8_t ack;

  Wire.begin();

  Wire.beginTransmission(ICM20948_I2C_ADDR);
  Wire.write(REG_WHO_AM_I);
  Wire.endTransmission();

  Wire.requestFrom(ICM20948_I2C_ADDR, 1);
  if (Wire.available() == 1) {
    ack = Wire.read();
  }
  Serial.print("ack: ");
  Serial.println(ack);
}


