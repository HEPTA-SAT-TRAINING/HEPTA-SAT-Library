/**
 * @file hepta_eps.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef HEPTA_EPS_H
#define HEPTA_EPS_H


#include <Arduino.h>

#include "../drv/adc_mcp3208.h"


typedef enum {
  ADC_5V_VOLTAGE = 0,
  ADC_3V3_VOLTAGE = 1,
  ADC_CURRENT_DISCHARGE = 3,
  ADC_CURRENT_CHARGE = 4,
  ADC_SAP_VOLTAGE = 5,
} ADC_PIN_ASSGNMENT;

class HeptaEPS {
  public:
    void begin(void);

    void switch_3V3_on(void);
    void switch_3V3_off(void);

    float get_battery_voltage(void);
    float get_5v_voltage(void);
    float get_3v3_voltage(void);
    float get_current_discharge(void);
    float get_current_charge(void);
    float get_sap_voltage(void);

  private:
    AdcMcp3208 adc;

    const uint8_t _sw_3v3_pin = 20;
    const uint8_t _bat_vol_pin = 26;
    const uint8_t _adc_cs_pin = 17;

    const float _adc_ref_voltage = 3.3;
    const uint16_t _adc_max_value = 4096;

    const float galvano_gain = 20;
    const float galvano_resistance = 0.02;
};


#endif /* HEPTA_EPS_H */
