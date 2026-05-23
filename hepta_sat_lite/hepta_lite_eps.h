/**
 * @file hepta_lite_eps.h
 * @brief HEPTA-SAT Lite EPS: battery + raw-ADC current sense.
 */

#ifndef HEPTA_LITE_EPS_H
#define HEPTA_LITE_EPS_H

#include "../common/hepta_eps_base.h"


class HeptaLiteEps : public HeptaEpsBase {
  public:
    HeptaLiteEps() : HeptaEpsBase(/*bat_vol_pin=*/26) {}

    float get_current_discharge(void);
    float get_current_charge(void);

  private:
    const uint8_t _current_discharge_pin = 27;
    const uint8_t _current_charge_pin = 28;
};


#endif /* HEPTA_LITE_EPS_H */
