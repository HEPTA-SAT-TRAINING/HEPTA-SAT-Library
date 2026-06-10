/**
 * @file hepta_com.h
 * @brief HEPTA-SAT COM: XBee SoftwareSerial RX/TX pins.
 */

#ifndef HEPTA_COM_H
#define HEPTA_COM_H

#include "../common/hepta_com_base.h"


class HeptaCom : public HeptaComBase {
  public:
    HeptaCom() : HeptaComBase(/*rx=*/15, /*tx=*/14) {}
};


#endif /* HEPTA_COM_H */
