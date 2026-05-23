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

#include "hepta_sensor_base.h"

bool HeptaSensorBase::begin(void) {
  analogReadResolution(12);
  pinMode(_temp_pin, INPUT);
  bno055.begin();
  return true;
}

float HeptaSensorBase::read_temp_voltage(void) {
  int32_t adc_value = analogRead(_temp_pin);
  return (adc_value / 4095.0f) * 3.3f;
}

void HeptaSensorBase::get_acceleration(float *ax, float *ay, float *az) {
  bno055.sen_acc(ax, ay, az);
}

void HeptaSensorBase::get_gyro(float *gx, float *gy, float *gz) {
  bno055.sen_gyro(gx, gy, gz);
}

void HeptaSensorBase::get_magnetometer(float *mx, float *my, float *mz) {
  bno055.sen_mag(mx, my, mz);
}

void HeptaSensorBase::print_acceleration(void) {
  bno055.print_acc();
}

void HeptaSensorBase::print_gyro(void) {
  bno055.print_gyro();
}

void HeptaSensorBase::print_magnetometer(void) {
  bno055.print_mag();
}
