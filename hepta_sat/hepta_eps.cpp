#include "hepta_eps.h"


enum AdcChannel : uint8_t {
  ADC_5V_VOLTAGE        = 0,
  ADC_3V3_VOLTAGE       = 1,
  ADC_CURRENT_DISCHARGE = 3,
  ADC_CURRENT_CHARGE    = 4,
  ADC_SAP_VOLTAGE       = 5,
};


void HeptaEps::init(void) {
  HeptaEpsBase::init();
  adc.begin(_adc_cs_pin);
  pinMode(_sw_3v3_pin, OUTPUT);
  digitalWrite(_sw_3v3_pin, LOW);
}

void HeptaEps::switch_3V3_on(void) {
  digitalWrite(_sw_3v3_pin, HIGH);
}

void HeptaEps::switch_3V3_off(void) {
  digitalWrite(_sw_3v3_pin, LOW);
}

float HeptaEps::get_5v_voltage(void) {
  const float resistor_1 = 10000.0;
  const float resistor_2 = 15000.0;
  return adc.get_voltage(ADC_5V_VOLTAGE) * ((resistor_1 + resistor_2) / resistor_2);
}

float HeptaEps::get_3v3_voltage(void) {
  const float resistor_1 = 10000.0;
  const float resistor_2 = 100000.0;
  return adc.get_voltage(ADC_3V3_VOLTAGE) * ((resistor_1 + resistor_2) / resistor_2);
}

float HeptaEps::get_sap_voltage(void) {
  return adc.get_voltage(ADC_SAP_VOLTAGE);
}

float HeptaEps::get_current_discharge(void) {
  return (adc.get_voltage(ADC_CURRENT_DISCHARGE) / galvano_gain) / galvano_resistance;
}

float HeptaEps::get_current_charge(void) {
  return (adc.get_voltage(ADC_CURRENT_CHARGE) / galvano_gain) / galvano_resistance;
}
