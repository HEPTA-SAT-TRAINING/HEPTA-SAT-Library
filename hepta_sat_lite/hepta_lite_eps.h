/**
 * @file hepta_lite_eps.h
 * @brief HEPTA-SAT Lite EPS (Ver4.2.0): bus voltage + MCP3208 rails / solar
 *        current, plus bus current on the MCU ADC (GP28). No charge current.
 */

#ifndef HEPTA_LITE_EPS_H
#define HEPTA_LITE_EPS_H

#include "../common/hepta_eps_base.h"
#include "../drv/adc_mcp3208.h"


class HeptaLiteEps : public HeptaEpsBase {
  public:
    // Ver4.2.0: same bus divider and current-sense amp as Full V4.1.1.
    // 1.3k (top) / 1.5k (to GND) -> gain ~1.867; MAX4372T gain 20.
    HeptaLiteEps()
      : HeptaEpsBase(/*bus_vol_pin=*/26,
                     /*bus_vol_divider_gain=*/(1300.0f + 1500.0f) / 1500.0f,
                     /*galvano_gain=*/20.0f) {}

    void init(void);

    void switch_3V3_on(void);
    void switch_3V3_off(void);

    float get_5v_voltage(void);
    float get_3v3_voltage(void);
    float get_sap_voltage(void);
    float get_current_solar(void);
    float get_current_bus(void);

  private:
    AdcMcp3208 adc;

    const uint8_t _sw_3v3_pin = 20;
    const uint8_t _adc_cs_pin = 17;
    // MAX4372T output (max ~2V), directly wired to the MCU ADC.
    const uint8_t _current_bus_pin = 28;
};


#endif /* HEPTA_LITE_EPS_H */
