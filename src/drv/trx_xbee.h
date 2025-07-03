/**
 * @file trx_xbee.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-03-04
 * 
 * @copyright PRELUDE-Project all rights reserved.
 * 
 */

#ifndef TRX_XBEE_H
#define TRX_XBEE_H

#include <Arduino.h>


typedef enum {
  XBEE_AT_MODE = 0x00,
  XBEE_API_MODE = 0x01, 
} xbee_mode_t;

typedef enum {
  XBEE_BAUD_RATE_1200   = 0x00,
  XBEE_BAUD_RATE_2400   = 0x01,
  XBEE_BAUD_RATE_4800   = 0x02,
  XBEE_BAUD_RATE_9600   = 0x03,
  XBEE_BAUD_RATE_19200  = 0x04,
  XBEE_BAUD_RATE_38400  = 0x05,
  XBEE_BAUD_RATE_57600  = 0x06,
  XBEE_BAUD_RATE_115200 = 0x07
} xbee_baud_rate_t;


class TrxXbee {
  public:
    TrxXbee(xbee_mode_t mode = XBEE_AT_MODE,
            xbee_baud_rate_t baud_rate = XBEE_BAUD_RATE_9600 );

    bool send_data(const uint8_t *data, size_t len);
    bool receive_data(uint8_t *data, size_t len);

  private:
    xbee_mode_t _mode;
    xbee_baud_rate_t _baud_rate;
};

#endif /* TRX_XBEE_H */
