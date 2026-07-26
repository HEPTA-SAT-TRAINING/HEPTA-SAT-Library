/**
 * @file hepta_eps.h
 * @brief HEPTA-SAT EPS (V4.1.1): bus voltage + MCP3208 rails / solar-charge
 *        currents, plus bus current on the MCU ADC (GP28).
 */

#ifndef HEPTA_EPS_H
#define HEPTA_EPS_H

#include "../common/hepta_eps_base.h"
#include "../drv/adc_mcp3208.h"


class HeptaEps : public HeptaEpsBase {
  public:
    HeptaEps() : HeptaEpsBase(/*bus_vol_pin=*/26) {}

    void init(void);

    void switch_3V3_on(void);
    void switch_3V3_off(void);

    float get_5v_voltage(void);
    float get_3v3_voltage(void);
    float get_sap_voltage(void);
    float get_current_solar(void);
    float get_current_charge(void);
    float get_current_bus(void);

  private:
    AdcMcp3208 adc;

    const uint8_t _sw_3v3_pin = 20;
    const uint8_t _adc_cs_pin = 17;
    // MAX4372T output (max ~2V), directly wired to the MCU ADC.
    // Shunt sits on the main bus path (schematic net CURRENT_DISCHARGE).
    const uint8_t _current_bus_pin = 28;
};


#endif /* HEPTA_EPS_H */
