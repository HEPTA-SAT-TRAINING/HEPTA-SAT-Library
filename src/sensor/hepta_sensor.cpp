/**
 * @file hepta_sensor.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "hepta_sensor.h"

HeptaSensor::HeptaSensor() {
  pinMode(_pin, INPUT);
}

float HeptaSensor::get_temperature(void) {
  float gain = -R5 * I / R4;
  float offset = Vref + I * R3;

  float volt = (analogRead(_pin))*3.3*(R_1 + R_2)/R_1;
  float Rth = (volt-offset)/gain+R3;
  float temp = (Rth-Pt)/(ce*Pt);

  return temp;
}
