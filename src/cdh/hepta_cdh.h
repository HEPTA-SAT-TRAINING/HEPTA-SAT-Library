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

class HeptaCDH {
  public:
    // For wrapping SD library
    // Wrapping function with variable length arguments is difficult.
    FILE_ERROR open_file(const char* file_name);

    // template <typename ... Args>
    // FILE_ERROR save_data(const char *format, Args const & ... args);
    // // FILE_ERROR save_data(const char *format, ...);
    FILE_ERROR close_file();

  private:
    const uint8_t _sd_mosi = 1;
    const uint8_t _sd_miso = 1;
    const uint8_t _sd_sclk = 1;
    const uint8_t _sd_cs = 1;    

    File _file;
};

// template <typename ... Args>
// FILE_ERROR HeptaCDH::save_data(const char *format, Args const & ... args) {
//   _file.print(format, args ...);

//   return NO_ERROR;
// }

#endif /* HEPTA_CDH_H */
