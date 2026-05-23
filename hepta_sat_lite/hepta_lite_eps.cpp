#include "hepta_lite_eps.h"


float HeptaLiteEps::get_current_discharge(void) {
  return (analogRead(_current_discharge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}

float HeptaLiteEps::get_current_charge(void) {
  return (analogRead(_current_charge_pin) * _adc_ref_voltage / _adc_max_value / galvano_gain) / galvano_resistance;
}
