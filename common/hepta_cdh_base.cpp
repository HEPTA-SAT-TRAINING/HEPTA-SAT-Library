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

#include "hepta_cdh_base.h"

void HeptaCdhBase::begin(void) {
  Serial.begin(9600);
  sd_begin();
}

void HeptaCdhBase::wait_for_serial(void) {
  while (!Serial);
}

void HeptaCdhBase::wait_for_sd(void) {
  while (!_sd_initialized) {
    Serial.println("SD Card initialization failed. Retrying...");
    sd_begin();
    delay(1000);
  }
}

size_t HeptaCdhBase::print(const String &text) {
  return Serial.print(text);
}

size_t HeptaCdhBase::print(const char *text) {
  if (text == NULL) {
    return 0;
  }

  return Serial.print(text);
}

size_t HeptaCdhBase::println(const String &text) {
  return Serial.println(text);
}

size_t HeptaCdhBase::println(const char *text) {
  if (text == NULL) {
    return 0;
  }

  return Serial.println(text);
}

size_t HeptaCdhBase::printf(const char *format, ...) {
  if (format == NULL) {
    return 0;
  }

  va_list args;
  va_start(args, format);
  size_t n = vprintf(format, args);
  va_end(args);
  return n;
}

size_t HeptaCdhBase::vprintf(const char *format, va_list args) {
  if (format == NULL) {
    return 0;
  }

  char buffer[128];
  va_list args_copy;
  va_copy(args_copy, args);
  int needed = vsnprintf(buffer, sizeof(buffer), format, args_copy);
  va_end(args_copy);

  if (needed < 0) {
    return 0;
  }

  if ((size_t)needed < sizeof(buffer)) {
    return Serial.write((const uint8_t *)buffer, (size_t)needed);
  }

  // Buffer was too small: allocate exact size and retry.
  size_t size = (size_t)needed + 1;
  char *heap = (char *)malloc(size);
  if (heap == NULL) {
    return Serial.write((const uint8_t *)buffer, sizeof(buffer) - 1);
  }
  vsnprintf(heap, size, format, args);
  size_t written = Serial.write((const uint8_t *)heap, (size_t)needed);
  free(heap);
  return written;
}

size_t HeptaCdhBase::printf_file(File &file, const char *format, ...) {
  if (!file || format == NULL) {
    return 0;
  }

  va_list args;
  va_start(args, format);
  size_t n = vprintf_file(file, format, args);
  va_end(args);
  return n;
}

size_t HeptaCdhBase::vprintf_file(File &file, const char *format, va_list args) {
  if (!file || format == NULL) {
    return 0;
  }

  char buffer[128];
  va_list args_copy;
  va_copy(args_copy, args);
  int needed = vsnprintf(buffer, sizeof(buffer), format, args_copy);
  va_end(args_copy);

  if (needed < 0) {
    return 0;
  }

  if ((size_t)needed < sizeof(buffer)) {
    return file.write((const uint8_t *)buffer, (size_t)needed);
  }

  size_t size = (size_t)needed + 1;
  char *heap = (char *)malloc(size);
  if (heap == NULL) {
    return file.write((const uint8_t *)buffer, sizeof(buffer) - 1);
  }
  vsnprintf(heap, size, format, args);
  size_t written = file.write((const uint8_t *)heap, (size_t)needed);
  free(heap);
  return written;
}

size_t HeptaCdhBase::write(uint8_t data) {
  return Serial.write(data);
}

size_t HeptaCdhBase::write(const uint8_t *buffer, size_t size) {
  if (buffer == NULL) {
    return 0;
  }

  return Serial.write(buffer, size);
}

bool HeptaCdhBase::sd_begin(void) {
  SPI.setRX(_sd_rx_pin);
  SPI.setTX(_sd_tx_pin);
  SPI.setSCK(_sd_sck_pin);
  SPI.begin();
  delay(100);
  for (uint8_t i = 0; i < 3; i++) {
    _sd_initialized = SD.begin(_sd_cs_pin);
    if (_sd_initialized) break;
    delay(100);
  }
  return _sd_initialized;
}

bool HeptaCdhBase::sd_is_available(void) const {
  return _sd_initialized;
}

File HeptaCdhBase::open_file(const char *path, int mode) {
  if (!_sd_initialized && !sd_begin()) {
    return File();
  }

  return SD.open(path, mode);
}

File HeptaCdhBase::create_file(const char *path) {
  if (SD.exists(path)) {
    SD.remove(path);
  }
  return open_file(path, FILE_WRITE);
}

File HeptaCdhBase::append_file(const char *path) {
  return open_file(path, FILE_WRITE);
}

bool HeptaCdhBase::file_exists(const char *path) {
  if (!_sd_initialized && !sd_begin()) {
    return false;
  }

  return SD.exists(path);
}

bool HeptaCdhBase::remove_file(const char *path) {
  if (!_sd_initialized && !sd_begin()) {
    return false;
  }

  return SD.remove(path);
}

size_t HeptaCdhBase::write_file(File &file, const char *text) {
  if (!file || text == NULL) {
    return 0;
  }

  return file.print(text);
}

size_t HeptaCdhBase::write_file(File &file, const uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return 0;
  }

  return file.write(buffer, size);
}

int HeptaCdhBase::read_file(File &file) {
  if (!file) {
    return -1;
  }

  return file.read();
}

int HeptaCdhBase::read_file(File &file, uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return -1;
  }

  return file.read(buffer, size);
}

bool HeptaCdhBase::is_cmd_received(void) {
  // Check if there is any data available in the serial buffer
  return Serial.available() > 0;
}

char HeptaCdhBase::get_command(void) {
  // Read one command character and ignore line endings from Serial Monitor.
  while (is_cmd_received()) {
    char received_char = Serial.read();
    if (received_char != '\r' && received_char != '\n') {
      return received_char;
    }
  }
  return '\0'; // Return null character if no data is available
}

bool HeptaCdhBase::command_execute(cmd_t cmd, cmd_arg_t arg) {

  return true; // Return 0 to indicate success
}
