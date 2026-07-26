/**
 * @file hepta_eps_base.h
 * @brief Shared EPS implementation (bus voltage + current-sense constants).
 *
 * Board-specific classes (HeptaEps, HeptaLiteEps) inherit from this base,
 * supply the divider ADC pin via the protected constructor, and add
 * their own rail / current readings (MCP3208 on HEPTA-SAT, raw ADC on Lite).
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_EPS_BASE_H
#define HEPTA_EPS_BASE_H

#include <Arduino.h>


class HeptaEpsBase {
  public:
    void init(void);
    float get_bus_voltage(void);
    uint16_t get_bus_voltage_raw(void);

  protected:
    HeptaEpsBase(uint8_t bus_vol_pin) : _bus_vol_pin(bus_vol_pin) {}

    const uint8_t _bus_vol_pin;

    const float _adc_ref_voltage = 3.3;
    const uint16_t _adc_max_value = 4096;

    // Main bus divider (V4.1.1): 1.3k (top) / 1.5k (to GND)
    // Vbus = Vadc * (R_top + R_bottom) / R_bottom ≈ Vadc * 1.867
    const float _bus_vol_divider_gain = (1300.0f + 1500.0f) / 1500.0f;

    // MAX4372T current-sense amp (the part actually mounted; schematic
    // symbol says MAX4372F) over a 0.02 ohm shunt.
    const float galvano_gain = 20;
    const float galvano_resistance = 0.02;
};


#endif /* HEPTA_EPS_BASE_H */
