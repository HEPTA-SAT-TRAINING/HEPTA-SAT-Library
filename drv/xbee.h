/**
 * @file xbee.h
 * @brief XBee UART and AT-command driver.
 */

#ifndef XBEE_H
#define XBEE_H

#include <Arduino.h>
#include <SoftwareSerial.h>


class Xbee {
  public:
    Xbee(uint8_t rx_pin, uint8_t tx_pin)
      : serial_(rx_pin, tx_pin), last_error_(nullptr), started_(false) {}

    bool begin(void);

    bool send(const char* text);
    bool send(const uint8_t* data, size_t length);

    int receive(char* buffer, size_t buffer_size, uint32_t timeout_ms = 0);
    int receive(uint8_t* buffer, size_t buffer_size, uint32_t timeout_ms = 0);
    int receive_until(char terminator,
                      char* buffer,
                      size_t buffer_size,
                      uint32_t timeout_ms = 1000);

    bool available(void);
    int read_byte(void);
    size_t write_byte(uint8_t data);

    bool enter_command_mode(void);
    bool exit_command_mode(void);
    bool send_at_command(const char* command,
                         char* response,
                         size_t response_size,
                         uint32_t timeout_ms = 1000);
    bool set_transparent_mode(bool save = false);
    bool set_api_mode(bool save = false);

    const char* last_error(void) const;

  private:
    SoftwareSerial serial_;
    const char* last_error_;
    bool started_;

    void clear_input(void);
    bool wait_for_response(const char* expected, uint32_t timeout_ms);
    int read_response(char* buffer, size_t buffer_size, uint32_t timeout_ms);
    bool set_mode(const char* command, bool save);
};


#endif /* XBEE_H */
