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
  adc.begin(_adc_cs_pin);
  pinMode(_temp_pin, INPUT);

}

float HeptaSensor::get_temperature(void) {
  //resistance
  const float R1 = 2500.0;
  const float R2 = 2500.0;
  const float R3 = 110.0;
  const float R4 = 1000.0;
  const float R5 = 68000.0;
  const float Pt = 100.0;
  const float R_1 = 3.0;
  const float R_2 = 2.0;

  //current
  const float I = 0.001;

  //voltage
  const float Vref = 2.5;

  //temperature coefficient
  const float ce = 0.003851;

  float gain = -R5 * I / R4;
  float offset = Vref + I * R3;

  uint16_t adc_volt = analogRead(_temp_pin) * 3.3 / 4096.0;
  float raw_volt = adc_volt * 3.3 * (R_1 + R_2) / R_1;
  float Rth = (raw_volt - offset) / gain + R3;
  float temp = (Rth - Pt) / (ce * Pt);

  return temp;
}
