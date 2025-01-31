/**
 * @file hepta_cdh.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "hepta_cdh.h"

FILE_ERROR HeptaCDH::open_file(const char* file_name) {
  if(!SD.begin(_sd_cs)) {
    return DEVICE_ACCESS_ERROR;
  }

  _file = SD.open(file_name, FILE_WRITE);

  if(!_file) {
    return FILE_OPEN_ERROR;
  }
}

// template <typename ... Args>
// FILE_ERROR HeptaCDH::save_data(const char *format, Args const & ... args) {
//   _file.print(format, args ...);

//   return NO_ERROR;
// }

// FILE_ERROR HeptaCDH::save_data(const char *format, ...) {
//   va_list va;
//   va_start(va, format);
//   // int vprintf(const char *format, va_list ap);
//   // vprintf(format, va);
//   _file.print(format, va);
//   va_end(va);

//   return NO_ERROR;
// }

FILE_ERROR HeptaCDH::close_file() {
  _file.close();
}
