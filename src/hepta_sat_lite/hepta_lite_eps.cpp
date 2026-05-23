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


#include "hepta_lite_eps.h"

void HeptaLiteEps::init(void) {
  analogReadResolution(12);
  pinMode(_bat_vol_pin, INPUT);
}

float HeptaLiteEps::get_battery_voltage(void) {
  //!@todo 電圧の分圧計算を実装する必要がある
  // 本来は分圧して電圧を測るはずだが，現在のバージョンではそもそもバッテリ電圧の計測がされていない
  float voltage = get_battery_voltage_raw() * (_adc_ref_voltage / _adc_max_value);
  return voltage;
}

uint16_t HeptaLiteEps::get_battery_voltage_raw(void) {
  return analogRead(_bat_vol_pin);
}

float HeptaLiteEps::get_current_discharge(void) {
  return (analogRead(_current_discharge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}

float HeptaLiteEps::get_current_charge(void) {
  return (analogRead(_current_charge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}
