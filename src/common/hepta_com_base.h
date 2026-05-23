/**
 * @file hepta_com_base.h
 * @brief Shared XBee/SoftwareSerial COM implementation.
 *
 * Board-specific classes (HeptaCom, HeptaLiteCom) inherit from this base
 * and supply RX/TX pins via the protected constructor.
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_COM_BASE_H
#define HEPTA_COM_BASE_H

#include <Arduino.h>
#include <SoftwareSerial.h>


class HeptaComBase {
  public:
    void begin(uint16_t baud_rate);
    char get_char(void);
    void send_char(const char c);

    String get_text(void);
    void send_text(const String text);

  protected:
    HeptaComBase(uint8_t rx, uint8_t tx) : XbeeSerial(rx, tx) {}

  private:
    SoftwareSerial XbeeSerial;
};


#endif /* HEPTA_COM_BASE_H */
