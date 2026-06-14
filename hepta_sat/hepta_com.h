/**
 * @file hepta_com.h
 * @brief HEPTA-SAT COM: XBee SoftwareSerial RX/TX pins.
 */

#ifndef HEPTA_COM_H
#define HEPTA_COM_H

#include "../common/hepta_com_base.h"


class HeptaCom : public HeptaComBase {
  public:
    HeptaCom()
      : HeptaComBase(/*rx=*/15, /*tx=*/14,
                     /*sd_cs=*/3, /*sd_tx=*/19, /*sd_rx=*/16, /*sd_sck=*/18) {}
};


#endif /* HEPTA_COM_H */
