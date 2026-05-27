/**
 * @file gps_gp1818mk.h
 * @author Masaki Naito
 * @brief Driver for GP-1818MK GPS module (NMEA 0183 over SoftwareSerial)
 * @version 0.3
 * @date 2025-02-26
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef GPS_GP1818MK_H
#define GPS_GP1818MK_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class Gps1818mk {
  public:
    /**
     * @brief Construct a new Gps1818mk object.
     * @param rx_pin Arduino pin connected to the GPS TX line (default: 13)
     * @param tx_pin Arduino pin connected to the GPS RX line; -1 if unused (default: -1)
     */
    Gps1818mk(pin_size_t rx_pin = 13, pin_size_t tx_pin = static_cast<pin_size_t>(-1))
      : _rx_pin(rx_pin), _tx_pin(tx_pin) {}

    /** @brief Initialize the SoftwareSerial port at 9600 baud. Call once in setup(). */
    void begin(void);

    /** @brief Print raw NMEA sentences to Serial for 5 seconds (debug use only). */
    void read_raw(void);

    /**
     * @brief Read latitude, longitude, and altitude from a GPGGA sentence.
     * @param lat  Output: latitude  in decimal degrees; negative = South
     * @param lon  Output: longitude in decimal degrees; negative = West
     * @param alt  Output: altitude above mean sea level in meters
     * @return true on a valid fix, false if no fix or timeout
     */
    bool get_position(float* lat, float* lon, float* alt);

    /**
     * @brief Read ground speed and heading from a GPRMC sentence.
     * @param velocity Output: ground speed in m/s
     * @param heading  Output: true course in degrees (0–360)
     * @return true on a valid fix, false if no fix or timeout
     */
    bool get_velocity(float* velocity, float* heading);

    /**
     * @brief Read position, velocity, and heading in one call (GPGGA then GPRMC).
     * @param lat      Output: latitude  in decimal degrees; negative = South
     * @param lon      Output: longitude in decimal degrees; negative = West
     * @param alt      Output: altitude in meters
     * @param velocity Output: ground speed in m/s
     * @param heading  Output: true course in degrees
     * @return true on a valid fix, false if no fix or timeout
     */
    bool get_all(float* lat, float* lon, float* alt, float* velocity, float* heading);

    /** @brief Return true if at least one byte is waiting in the receive buffer. */
    bool is_data_available(void);

    /** @brief Attempt get_position() and print the result to Serial. */
    void test_gps(void);

  private:
    pin_size_t _rx_pin;
    pin_size_t _tx_pin;
    SoftwareSerial* _serial = nullptr;

    int  read_byte(void);              // Returns one byte, or -1 on 200 ms timeout
    bool wait_serial(void);            // Returns false if no data within 1 second
    bool get_header(const char* header); // Sliding-window search for 6-char NMEA header
    bool parse_gpgga(float* lat, float* lon, float* alt);
    bool parse_gprmc(float* velocity, float* heading);
};

#endif /* GPS_GP1818MK_H */
