/**
 * @file hepta_lite_com.h
 * @brief HEPTA-SAT Lite COM: XBee SoftwareSerial RX/TX pins for the Lite board.
 */

#ifndef HEPTA_LITE_COM_H
#define HEPTA_LITE_COM_H

#include "../common/hepta_com_base.h"


class HeptaLiteCom : public HeptaComBase {
  public:
    HeptaLiteCom()
      : HeptaComBase(/*rx=*/15, /*tx=*/14,
                     /*sd_cs=*/3, /*sd_tx=*/19, /*sd_rx=*/16, /*sd_sck=*/18) {}
};


#endif /* HEPTA_LITE_COM_H */
