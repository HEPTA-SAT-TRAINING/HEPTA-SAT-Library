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
    /**
     * @brief Read raw data from the GPS module
     */
    void read_raw(void);

    /**
     * @brief Get the current position from the GPS module
     * @param lat Pointer to store latitude
     * @param lon Pointer to store longitude
     * @param alt Pointer to store altitude
     * @return true if position data is successfully retrieved, false otherwise
     */
    bool get_position(float* lat, float* lon, float* alt);

    /**
     * @brief Check if new GPS data is available
     * @return true if data is available, false otherwise
     */
    bool is_data_available(void);

  private:
    char read_byte(void);
    bool wait_serial(void);
    bool get_header(char array[]);
};

#endif /* GPS_1818MK */
