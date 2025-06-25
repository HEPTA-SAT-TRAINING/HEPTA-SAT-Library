/**
 * @file gps_gp1818mk.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-02-26
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef GPS_1818MK
#define GPS_1818MK

#include <Arduino.h>

#define GPS_SERIAL Serial1

class Gps1818mk {
  public:
    void read_raw(void);
    bool get_position(float* lat, float* lon, float* alt);
    bool is_data_available(void);

  private:
    char read_byte(void);
    void wait_serial(void);
    bool get_header(char array[]);
};

#endif /* GPS_1818MK */
