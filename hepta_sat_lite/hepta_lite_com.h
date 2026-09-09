/**
 * @file hepta_lite_com.h
 * @brief HEPTA-SAT Lite COM: XBee SoftwareSerial RX/TX pins + reset line (Ver4.2.0).
 */

#ifndef HEPTA_LITE_COM_H
#define HEPTA_LITE_COM_H

#include "../common/hepta_com_base.h"


class HeptaLiteCom : public HeptaComBase {
  public:
    HeptaLiteCom()
      : HeptaComBase(/*rx=*/15, /*tx=*/14,
                     /*sd_cs=*/3, /*sd_tx=*/19, /*sd_rx=*/16, /*sd_sck=*/18) {}

    /** @brief Deassert XBEE_RESET (GP1), then start the XBee serial port. */
    bool begin(void) {
      pinMode(_xbee_reset_pin, OUTPUT);
      digitalWrite(_xbee_reset_pin, HIGH);
      return HeptaComBase::begin();
    }

    /** @brief Hardware-reset the XBee via the active-low RESET line. */
    void xbee_reset(void) {
      digitalWrite(_xbee_reset_pin, LOW);
      delay(5);
      digitalWrite(_xbee_reset_pin, HIGH);
      delay(1000);  // XBee UART is ready ~1 s after RESET
    }

  private:
    const uint8_t _xbee_reset_pin = 1;
};


#endif /* HEPTA_LITE_COM_H */
