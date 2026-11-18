#ifndef HEPTA_PAYLOAD_PINS_H
#define HEPTA_PAYLOAD_PINS_H

#include <stdint.h>

// HEPTA-SAT V4.1.1: analog terminals are MCP3208 channels, digital terminals are GPIOs.
enum class HeptaAnalogInput : uint8_t { User1 = 5, User2 = 6, User3 = 7 };
enum class HeptaDigitalIo : uint8_t { UserIo1 = 21, UserIo2 = 22 };

constexpr HeptaAnalogInput USER1 = HeptaAnalogInput::User1;
constexpr HeptaAnalogInput USER2 = HeptaAnalogInput::User2;
constexpr HeptaAnalogInput USER3 = HeptaAnalogInput::User3;
constexpr HeptaDigitalIo USER_IO_1 = HeptaDigitalIo::UserIo1;
constexpr HeptaDigitalIo USER_IO_2 = HeptaDigitalIo::UserIo2;

namespace hepta_payload {
constexpr uint8_t adc_cs_pin = 17;

constexpr uint8_t adc_channel(HeptaAnalogInput input) {
  return static_cast<uint8_t>(input);
}

constexpr uint8_t gpio(HeptaDigitalIo pin) {
  return static_cast<uint8_t>(pin);
}
}  // namespace hepta_payload

#endif
