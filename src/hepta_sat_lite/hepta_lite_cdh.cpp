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

#include "hepta_lite_cdh.h"

void HeptaLiteCdh::begin(void) {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);
  Serial1.begin(9600);

  if (sd_begin()) {
    Serial.println("SD Card initialized successfully.");
  } else {
    Serial.println("SD Card initialization failed.");
  }
}

size_t HeptaLiteCdh::print(const String &text) {
  return Serial.print(text);
}

size_t HeptaLiteCdh::print(const char *text) {
  if (text == NULL) {
    return 0;
  }

  return Serial.print(text);
}

size_t HeptaLiteCdh::println(const String &text) {
  return Serial.println(text);
}

size_t HeptaLiteCdh::println(const char *text) {
  if (text == NULL) {
    return 0;
  }

  return Serial.println(text);
}

size_t HeptaLiteCdh::write(uint8_t data) {
  return Serial.write(data);
}

size_t HeptaLiteCdh::write(const uint8_t *buffer, size_t size) {
  if (buffer == NULL) {
    return 0;
  }

  return Serial.write(buffer, size);
}

bool HeptaLiteCdh::sd_begin(void) {
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

bool HeptaLiteCdh::sd_is_available(void) const {
  return _sd_initialized;
}

File HeptaLiteCdh::open_file(const char *path, int mode) {
  if (!_sd_initialized && !sd_begin()) {
    return File();
  }

  return SD.open(path, mode);
}

File HeptaLiteCdh::create_file(const char *path) {
  if (SD.exists(path)) {
    SD.remove(path);
  }
  return open_file(path, FILE_WRITE);
}

File HeptaLiteCdh::append_file(const char *path) {
  return open_file(path, FILE_WRITE);
}

bool HeptaLiteCdh::file_exists(const char *path) {
  if (!_sd_initialized && !sd_begin()) {
    return false;
  }

  return SD.exists(path);
}

bool HeptaLiteCdh::remove_file(const char *path) {
  if (!_sd_initialized && !sd_begin()) {
    return false;
  }

  return SD.remove(path);
}

size_t HeptaLiteCdh::write_file(File &file, const char *text) {
  if (!file || text == NULL) {
    return 0;
  }

  return file.print(text);
}

size_t HeptaLiteCdh::write_file(File &file, const uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return 0;
  }

  return file.write(buffer, size);
}

int HeptaLiteCdh::read_file(File &file) {
  if (!file) {
    return -1;
  }

  return file.read();
}

int HeptaLiteCdh::read_file(File &file, uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return -1;
  }

  return file.read(buffer, size);
}

cmd_t HeptaLiteCdh::get_command(void) {
  cmd_t cmd = 0;

  char *e;
  uint8_t base = 10;
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // 1行分読み込む
    cmd = (cmd_t)strtol(input.c_str(), &e, base);
  }


  if(cmd!= 0) {
    Serial.print("Received command: ");
    Serial.println(cmd, HEX);
  }

  return cmd; // Return the received command
}

bool HeptaLiteCdh::command_execute(cmd_t cmd, cmd_arg_t arg) {

  return true; // Return 0 to indicate success
}
