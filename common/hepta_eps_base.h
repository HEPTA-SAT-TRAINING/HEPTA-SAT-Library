/**
 * @file hepta_eps_base.h
 * @brief Shared EPS implementation (battery voltage + current-sense constants).
 *
 * Board-specific classes (HeptaEps, HeptaLiteEps) inherit from this base,
 * supply the battery voltage ADC pin via the protected constructor, and add
 * their own rail / current readings (MCP3208 on HEPTA-SAT, raw ADC on Lite).
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_EPS_BASE_H
#define HEPTA_EPS_BASE_H

#include <Arduino.h>


class HeptaEpsBase {
  public:
    void init(void);
    float get_battery_voltage(void);
    uint16_t get_battery_voltage_raw(void);

  protected:
    HeptaEpsBase(uint8_t bat_vol_pin) : _bat_vol_pin(bat_vol_pin) {}

    const uint8_t _bat_vol_pin;

    const float _adc_ref_voltage = 3.3;
    const uint16_t _adc_max_value = 4096;

    // Battery voltage divider: 12k (top) / 30k (to GND)
    // Vbat = Vadc * (R_top + R_bottom) / R_bottom = Vadc * 1.4
    const float _bat_vol_divider_gain = (12000.0f + 30000.0f) / 30000.0f;

    const float galvano_gain = 20;
    const float galvano_resistance = 0.02;
};


#endif /* HEPTA_EPS_BASE_H */
