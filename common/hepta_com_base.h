/**
 * @file hepta_com_base.h
 * @brief Shared HEPTA-SAT COM API backed by the XBee driver.
 *
 * Board-specific classes (HeptaCom, HeptaLiteCom) inherit from this base
 * and supply RX/TX pins via the protected constructor.
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_COM_BASE_H
#define HEPTA_COM_BASE_H

#include <Arduino.h>
#include <stdarg.h>

#include "../drv/xbee.h"


class HeptaComBase {
  public:
    bool begin(void);
    bool begin(uint16_t baud_rate);

    bool send(const char* text);
    bool send(const uint8_t* data, size_t length);

    size_t print(const String& text);
    size_t print(const char* text);
    size_t println(const String& text);
    size_t println(const char* text);
    size_t printf(const char* format, ...) __attribute__((format(printf, 2, 3)));
    size_t vprintf(const char* format, va_list args);

    int receive(char* buffer, size_t buffer_size, uint32_t timeout_ms = 0);
    int receive(uint8_t* buffer, size_t buffer_size, uint32_t timeout_ms = 0);
    int receive_until(char terminator,
                      char* buffer,
                      size_t buffer_size,
                      uint32_t timeout_ms = 1000);

    bool available(void);

    bool enter_command_mode(void);
    bool exit_command_mode(void);
    bool send_at_command(const char* command,
                         char* response,
                         size_t response_size,
                         uint32_t timeout_ms = 1000);
    bool set_transparent_mode(bool save = false);
    bool set_api_mode(bool save = false);

    const char* last_error(void) const;

    // Legacy API retained for existing sketches.
    char get_char(void);
    void send_char(const char c);

    String get_text(void);
    void send_text(const String text);

  protected:
    HeptaComBase(uint8_t rx, uint8_t tx)
      : xbee_(rx, tx) {}

  private:
    Xbee xbee_;
};


#endif /* HEPTA_COM_BASE_H */
