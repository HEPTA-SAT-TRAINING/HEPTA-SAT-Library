#pragma once
#include <stdint.h>
#include <string>
namespace fake_rx {
extern uint32_t clock_ms;
extern bool fail_begin;
void inject(uint32_t rx_pin, const std::string& bytes);
void inject_after_read(uint32_t rx_pin, const std::string& bytes);
void inject_after_delay(uint32_t rx_pin, const std::string& bytes);
void error(uint32_t rx_pin);
}
