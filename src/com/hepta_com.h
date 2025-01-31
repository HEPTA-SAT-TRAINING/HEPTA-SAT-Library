/**
 * @file hepta_com.h
 * @author Masaki Naito
 * @brief
 * @version 0.1
 * @date 2024-12-08
 * 
 * @copyright UNISEC all rights reserved.
 */

#ifndef HEPTA_COM_H
#define HEPTA_COM_H

#include <Arduino.h>

class HeptaCom {
  public:
    HeptaCom(uint16_t baud_rate);
    uint8_t get_text(void);
    void send_text(const char *format, ... );
  private:
};

#endif /* HEPTA_COM_H */
