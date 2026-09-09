/**
 * @file hepta_eps_base.h
 * @brief Shared EPS implementation (bus voltage readout + ADC reference).
 *
 * Board-specific classes (HeptaEps, HeptaLiteEps) inherit from this base and
 * supply the divider ADC pin plus their own divider / current-sense gains via
 * the protected constructor, then add rail / current readings (MCP3208 on
 * both boards; Lite has no charge-current channel). Sketches do not include
 * this header directly.
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
    HeptaEpsBase(uint8_t bus_vol_pin, float bus_vol_divider_gain,
                 float galvano_gain)
      : _bus_vol_pin(bus_vol_pin),
        _bus_vol_divider_gain(bus_vol_divider_gain),
        galvano_gain(galvano_gain) {}

    const uint8_t _bus_vol_pin;

    const float _adc_ref_voltage = 3.3;
    const uint16_t _adc_max_value = 4096;

    // Board-specific: Vbus = Vadc * (R_top + R_bottom) / R_bottom.
    const float _bus_vol_divider_gain;

    // Board-specific current-sense amp gain over the shared 0.02 ohm shunt.
    const float galvano_gain;
    const float galvano_resistance = 0.02;
};


#endif /* HEPTA_EPS_BASE_H */
