#pragma once
#include <stdint.h>
#include <stddef.h>
uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
class Stream {
public:
  virtual int available() = 0;
  virtual int read() = 0;
  virtual size_t write(const uint8_t*, size_t) = 0;
  size_t write(uint8_t value) { return write(&value,1); }
};
