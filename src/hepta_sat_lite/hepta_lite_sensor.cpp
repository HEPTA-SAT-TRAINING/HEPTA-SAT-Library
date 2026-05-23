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

#include "hepta_lite_sensor.h"

bool HeptaLiteSensor::begin(void) {
  analogReadResolution(12);
  pinMode(_temp_pin, INPUT);
  bno055.begin();
  return true;
}

float HeptaLiteSensor::get_temperature(void) {
  int32_t adc_value = analogRead(_temp_pin);
  float voltage = (adc_value / 4095.0) * 3.3;
  float temp = (voltage - 0.6) * 100.0;
  return temp;
}

void HeptaLiteSensor::get_acceleration(float *ax, float *ay, float *az) {
  bno055.sen_acc(ax, ay, az);
}

void HeptaLiteSensor::get_gyro(float *gx, float *gy, float *gz) {
  bno055.sen_gyro(gx, gy, gz);
}

void HeptaLiteSensor::get_magnetometer(float *mx, float *my, float *mz) {
  bno055.sen_mag(mx, my, mz);
}

void HeptaLiteSensor::print_acceleration(void) {
  bno055.print_acc();
}

void HeptaLiteSensor::print_gyro(void) {
  bno055.print_gyro();
}

void HeptaLiteSensor::print_magnetometer(void) {
  bno055.print_mag();
}
