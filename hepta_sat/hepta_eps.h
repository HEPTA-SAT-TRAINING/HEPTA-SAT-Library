/**
 * @file hepta_eps.h
 * @brief HEPTA-SAT EPS: battery + MCP3208 for rail voltages and current sense.
 */

#ifndef HEPTA_EPS_H
#define HEPTA_EPS_H

#include "../common/hepta_eps_base.h"
#include "../drv/adc_mcp3208.h"


class HeptaEps : public HeptaEpsBase {
  public:
    HeptaEps() : HeptaEpsBase(/*bat_vol_pin=*/26) {}

    void init(void);

    void switch_3V3_on(void);
    void switch_3V3_off(void);

    float get_5v_voltage(void);
    float get_3v3_voltage(void);
    float get_sap_voltage(void);
    float get_current_discharge(void);
    float get_current_charge(void);

  private:
    AdcMcp3208 adc;

    const uint8_t _sw_3v3_pin = 20;
    const uint8_t _adc_cs_pin = 17;
};


#endif /* HEPTA_EPS_H */
