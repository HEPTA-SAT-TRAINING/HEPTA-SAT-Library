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

void HeptaCdh::begin(void) {
  Serial.begin(9600);
  Serial1.begin(9600);

  if (sd_begin(_sd_cs_pin)) {
    Serial.println("SD Card initialized successfully.");
  } else {
    Serial.println("SD Card initialization failed.");
  }
}

bool HeptaCdh::sd_begin(uint8_t cs_pin) {
  _sd_cs_pin = cs_pin;
  SPI.begin();
  _sd_initialized = SD.begin(_sd_cs_pin);
  return _sd_initialized;
}

bool HeptaCdh::sd_is_available(void) const {
  return _sd_initialized;
}

File HeptaCdh::open_file(const char *path, int mode) {
  if (!_sd_initialized && !sd_begin(_sd_cs_pin)) {
    return File();
  }

  return SD.open(path, mode);
}

File HeptaCdh::create_file(const char *path) {
  return open_file(path, FILE_WRITE);
}

bool HeptaCdh::file_exists(const char *path) {
  if (!_sd_initialized && !sd_begin(_sd_cs_pin)) {
    return false;
  }

  return SD.exists(path);
}

bool HeptaCdh::remove_file(const char *path) {
  if (!_sd_initialized && !sd_begin(_sd_cs_pin)) {
    return false;
  }

  return SD.remove(path);
}

size_t HeptaCdh::write_file(File &file, const char *text) {
  if (!file || text == NULL) {
    return 0;
  }

  return file.print(text);
}

size_t HeptaCdh::write_file(File &file, const uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return 0;
  }

  return file.write(buffer, size);
}

int HeptaCdh::read_file(File &file) {
  if (!file) {
    return -1;
  }

  return file.read();
}

int HeptaCdh::read_file(File &file, uint8_t *buffer, size_t size) {
  if (!file || buffer == NULL) {
    return -1;
  }

  return file.read(buffer, size);
}

cmd_t HeptaCdh::get_command(void) {
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

bool HeptaCdh::command_execute(cmd_t cmd, cmd_arg_t arg) {

  return true; // Return 0 to indicate success
}
