#include "hepta_eps_base.h"


void HeptaEpsBase::init(void) {
  analogReadResolution(12);
  pinMode(_bus_vol_pin, INPUT);
}

float HeptaEpsBase::get_bus_voltage(void) {
  return get_bus_voltage_raw() * (_adc_ref_voltage / _adc_max_value) * _bus_vol_divider_gain;
}

uint16_t HeptaEpsBase::get_bus_voltage_raw(void) {
  return analogRead(_bus_vol_pin);
}
