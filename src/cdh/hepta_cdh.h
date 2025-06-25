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

#include <stdarg.h>
#include <SD.h>

typedef enum {
  NO_ERROR,
  DEVICE_ACCESS_ERROR,
  FILE_OPEN_ERROR,
  FILE_CLOSE_ERROR,
  OTHER_ERROR
} FILE_ERROR;

typedef uint8_t cmd_t;
typedef uint64_t cmd_arg_t;

class HeptaCDH : public File, public SDClass {
  public:
    int8_t command_execute(cmd_t cmd, cmd_arg_t arg = 0);

  private:
    const uint8_t _sd_mosi = 1;
    const uint8_t _sd_miso = 1;
    const uint8_t _sd_sclk = 1;
    const uint8_t _sd_cs = 1;    

    File _file;
};


#endif /* HEPTA_CDH_H */
