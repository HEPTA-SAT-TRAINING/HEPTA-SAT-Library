/**
 * @file trx_xbee.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-03-04
 * 
 * @copyright PRELUDE-Project all rights reserved.
 * 
 */

#include "trx_xbee.h"

TrxXbee::TrxXbee(xbee_mode_t mode, xbee_baud_rate_t baud_rate) {
  _mode = mode;
  _baud_rate = baud_rate;

  // Initialize the XBee module with the specified mode
  if (mode == XBEE_AT_MODE) {
    Serial.println("XBee initialized in AT mode.");
  } else if (mode == XBEE_API_MODE) {
    Serial.println("Xbee API mode is not implemented yet.");
    // Serial.println("XBee initialized in API mode.");
  } else {
    Serial.println("Unknown XBee mode.");
  }
}


