/**
 * @file hepta_cdh_base.h
 * @brief Shared CDH implementation for HEPTA-SAT and HEPTA-SAT Lite.
 *
 * Board-specific classes (HeptaCdh, HeptaLiteCdh) inherit from this base
 * and supply pin assignments via the protected constructor.
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_CDH_BASE_H
#define HEPTA_CDH_BASE_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <stdarg.h>

typedef uint8_t cmd_t;
typedef uint64_t cmd_arg_t; // not yet used, but reserved for future use


class HeptaCdhBase {
  public:
    void begin(void);
    void wait_for_serial(void);
    void wait_for_sd(void);
    size_t print(const String &text);
    size_t print(const char *text);
    size_t println(const String &text);
    size_t println(const char *text);
    size_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)));
    size_t vprintf(const char *format, va_list args);
    size_t printf_file(File &file, const char *format, ...) __attribute__((format(printf, 3, 4)));
    size_t vprintf_file(File &file, const char *format, va_list args);
    size_t write(uint8_t data);
    size_t write(const uint8_t *buffer, size_t size);
    bool is_cmd_received(void);
    char get_command(void);
    bool command_execute(cmd_t cmd, cmd_arg_t arg = 0);

    bool sd_begin(void);
    bool sd_is_available(void) const;
    File open_file(const char *path, int mode = FILE_READ);
    File create_file(const char *path);
    File append_file(const char *path);
    bool file_exists(const char *path);
    bool remove_file(const char *path);
    size_t write_file(File &file, const char *text);
    size_t write_file(File &file, const uint8_t *buffer, size_t size);
    int read_file(File &file);
    int read_file(File &file, uint8_t *buffer, size_t size);

  protected:
    HeptaCdhBase(uint8_t sd_cs, uint8_t sd_tx, uint8_t sd_rx, uint8_t sd_sck)
      : _sd_cs_pin(sd_cs), _sd_tx_pin(sd_tx), _sd_rx_pin(sd_rx), _sd_sck_pin(sd_sck) {}

  private:
    const uint8_t _sd_cs_pin;
    const uint8_t _sd_tx_pin;
    const uint8_t _sd_rx_pin;
    const uint8_t _sd_sck_pin;
    bool _sd_initialized = false;
};


#endif /* HEPTA_CDH_BASE_H */
