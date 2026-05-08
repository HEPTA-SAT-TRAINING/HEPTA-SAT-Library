/**
 * @file hepta_eps.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#include "hepta_eps.h"

void HeptaEps::init(void) {
  pinMode(_bat_vol_pin, INPUT);
}

float HeptaEps::get_battery_voltage(void) {
  float voltage = analogRead(_bat_vol_pin) * (_adc_ref_voltage / _adc_max_value);
  return voltage;
}

float HeptaEps::get_current_discharge(void) {
  return (analogRead(_current_discharge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}

float HeptaEps::get_current_charge(void) {
  return (analogRead(_current_charge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}
