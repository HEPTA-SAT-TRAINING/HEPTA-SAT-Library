/**
 * @file hepta_cdh.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#ifndef HEPTA_CDH_H
#define HEPTA_CDH_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

typedef uint8_t cmd_t;
typedef uint64_t cmd_arg_t; // not yet used, but reserved for future use


class HeptaCdh {
  public:
    void begin(void);
    cmd_t get_command(void);
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

  private:
    uint8_t _sd_cs_pin = 3;
    uint8_t _sd_tx_pin = 19;
    uint8_t _sd_rx_pin = 16;
    uint8_t _sd_sck_pin = 18;
    bool _sd_initialized = false;
};


#endif /* HEPTA_CDH_H */
