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

/**
 * @brief Parsed contents of a GPGGA sentence (position + fix quality).
 *
 * lat/lon are in decimal degrees (negative = South / West) and alt is in
 * meters above mean sea level — i.e. the same convention as get_position().
 */
struct GpggaData {
  float   utc_time;     ///< UTC time of fix, hhmmss.sss
  float   lat;          ///< Latitude  in decimal degrees; negative = South
  float   lon;          ///< Longitude in decimal degrees; negative = West
  float   alt;          ///< Altitude above mean sea level in meters
  uint8_t fix_quality;  ///< 0 = no fix, 1 = GPS, 2 = DGPS
  uint8_t sat_num;      ///< Number of satellites in use
  float   hdop;         ///< Horizontal dilution of precision
};

/**
 * @brief Parsed contents of a GPRMC sentence (velocity + heading).
 */
struct GprmcData {
  float utc_time;   ///< UTC time of fix, hhmmss.sss
  float velocity;   ///< Ground speed in m/s
  float heading;    ///< True course in degrees (0–360)
  char  date[7];    ///< UTC date, ddmmyy (null-terminated)
};

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

    /**
     * @brief Non-blocking read of one byte from the raw NMEA stream.
     * @return The next byte (0-255) if one is waiting, or -1 if no data is
     *         available. Pair with is_data_available() and call repeatedly
     *         from the sketch loop; the caller decides where to print it.
     */
    int read_byte(void);

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

    /**
     * @brief Read a full GPGGA sentence into a caller-owned struct.
     * @param out Output: filled on success; unchanged on failure
     * @return true on a valid fix, false if no fix or timeout
     *
     * Same data as get_position() plus fix quality, satellite count, and HDOP.
     * The struct is owned by the caller — no internal buffer is returned.
     */
    bool get_gpgga(GpggaData* out);

    /**
     * @brief Read a full GPRMC sentence into a caller-owned struct.
     * @param out Output: filled on success; unchanged on failure
     * @return true on a valid fix, false if no fix or timeout
     *
     * Same data as get_velocity() plus UTC time and date.
     */
    bool get_gprmc(GprmcData* out);

    /** @brief Return true if at least one byte is waiting in the receive buffer. */
    bool is_data_available(void);

  private:
    pin_size_t _rx_pin;
    pin_size_t _tx_pin;
    SoftwareSerial* _serial = nullptr;

    int  read_byte_timeout(void);                            // Returns one byte, or -1 on 200 ms timeout
    bool wait_serial(void);                          // Returns false if no data within 1 second
    bool get_header(const char* header);             // Sliding-window search for 6-char NMEA header
    bool read_sentence(char* buf, uint16_t len);     // Skip comma, read until CRLF into buf
    bool parse_gpgga(GpggaData* out);   // Parse a GPGGA sentence body into out
    bool parse_gprmc(GprmcData* out);   // Parse a GPRMC sentence body into out
};

#endif /* GPS_GP1818MK_H */
