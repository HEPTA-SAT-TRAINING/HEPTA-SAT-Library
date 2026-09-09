#include "hepta_lite_eps.h"


// MCP3208 channel map (Ver4.2.0). CH4 is USER; there is no charge-current sense.
enum AdcChannel : uint8_t {
  ADC_5V_VOLTAGE     = 0,
  ADC_3V3_VOLTAGE    = 1,
  ADC_SAP_VOLTAGE    = 2,
  ADC_CURRENT_SOLAR  = 3,
};


void HeptaLiteEps::init(void) {
  HeptaEpsBase::init();
  adc.begin(_adc_cs_pin);
  pinMode(_current_bus_pin, INPUT);
  pinMode(_sw_3v3_pin, OUTPUT);
  // Active-high enable: HIGH = 3V3 rail ON.
  digitalWrite(_sw_3v3_pin, HIGH);
}

void HeptaLiteEps::switch_3V3_on(void) {
  digitalWrite(_sw_3v3_pin, HIGH);
}

void HeptaLiteEps::switch_3V3_off(void) {
  digitalWrite(_sw_3v3_pin, LOW);
}

float HeptaLiteEps::get_5v_voltage(void) {
  const float resistor_1 = 1000.0;
  const float resistor_2 = 1500.0;
  return adc.get_voltage(ADC_5V_VOLTAGE) * ((resistor_1 + resistor_2) / resistor_2);
}

float HeptaLiteEps::get_3v3_voltage(void) {
  const float resistor_1 = 1000.0;
  const float resistor_2 = 10000.0;
  return adc.get_voltage(ADC_3V3_VOLTAGE) * ((resistor_1 + resistor_2) / resistor_2);
}

float HeptaLiteEps::get_sap_voltage(void) {
  const float resistor_1 = 1300.0;
  const float resistor_2 = 1500.0;
  return adc.get_voltage(ADC_SAP_VOLTAGE) * ((resistor_1 + resistor_2) / resistor_2);
}

float HeptaLiteEps::get_current_solar(void) {
  return (adc.get_voltage(ADC_CURRENT_SOLAR) / galvano_gain) / galvano_resistance;
}

float HeptaLiteEps::get_current_bus(void) {
  float voltage = analogRead(_current_bus_pin) * (_adc_ref_voltage / _adc_max_value);
  return (voltage / galvano_gain) / galvano_resistance;
}
