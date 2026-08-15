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
#include "hepta_storage.h"


class HeptaComBase {
  public:
    static constexpr uint8_t IMAGE_ERROR_NOT_AVAILABLE   = 0x01;
    static constexpr uint8_t IMAGE_ERROR_CAPTURE_FAILED  = 0x02;
    static constexpr uint8_t IMAGE_ERROR_SIZE_TOO_LARGE  = 0x03;
    static constexpr uint8_t IMAGE_ERROR_INTERNAL_BUFFER = 0x04;

    bool begin(void);
    bool set_uart_baud(uint32_t baud);

    bool send(const char* text);
    bool send(const uint8_t* data, size_t length);

    size_t print(const String& text);
    size_t print(const char* text);
    size_t println(const String& text);
    size_t println(const char* text);
    size_t printf(const char* format, ...) __attribute__((format(printf, 2, 3)));
    size_t vprintf(const char* format, va_list args);
    size_t write(uint8_t data);
    size_t write(const uint8_t* buffer, size_t size);

    bool is_cmd_received(void);
    char get_command(void);

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

    bool downlink_image_file(const char* filename, uint16_t image_id = 0);
    void send_image_error(uint8_t error_code);

  protected:
    HeptaComBase(uint8_t rx, uint8_t tx,
                 uint8_t sd_cs, uint8_t sd_tx, uint8_t sd_rx, uint8_t sd_sck)
      : xbee_(rx, tx), storage_(sd_cs, sd_tx, sd_rx, sd_sck) {}

  private:
    Xbee xbee_;
    HeptaStorage storage_;
};


#endif /* HEPTA_COM_BASE_H */
