/**
 * @file hepta_lite_eps.h
 * @brief HEPTA-SAT Lite EPS: battery + raw-ADC current sense.
 */

#ifndef HEPTA_LITE_EPS_H
#define HEPTA_LITE_EPS_H

#include "../common/hepta_eps_base.h"


class HeptaLiteEps : public HeptaEpsBase {
  public:
    // Lite hardware (unchanged by V4.1.1): battery divider 12k (top) /
    // 30k (to GND) -> gain 1.4; current-sense amp gain 50.
    HeptaLiteEps()
      : HeptaEpsBase(/*bus_vol_pin=*/26,
                     /*bus_vol_divider_gain=*/(12000.0f + 30000.0f) / 30000.0f,
                     /*galvano_gain=*/50.0f) {}

    float get_current_discharge(void);
    float get_current_charge(void);

  private:
    const uint8_t _current_discharge_pin = 27;
    const uint8_t _current_charge_pin = 28;
};


#endif /* HEPTA_LITE_EPS_H */
