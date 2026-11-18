#include "xbee.h"

#include <string.h>
#include <new>


namespace {

constexpr uint32_t XBEE_GUARD_TIME_MS = 1100;
constexpr uint32_t XBEE_COMMAND_TIMEOUT_MS = 1000;
constexpr int XBEE_ERROR = -1;
constexpr int XBEE_BUFFER_OVERFLOW = -2;

}  // namespace


bool Xbee::begin(uint32_t baud) {
  // Arduino-Pico SoftwareSerial claims PIO state machines on begin().
  // Calling begin() again without end() leaks SMs and can hang USB CDC
  // (seen after camera SPI, when Lab99 retries AT with com.begin()).
  if (started_) {
    serial_.end();
    delay(1);
  }
  api_.reset();
  managed_ = false;
  serial_.begin(baud);
  started_ = true;
  last_error_ = nullptr;
  return true;
}

bool Xbee::begin_managed(XbeeReceive mode) {
  begin();
  bool switched = set_api_mode(false);
  managed_ = true;  // A failed handshake must never fall back to raw RF output.
  if (!switched) return false;
  api_.reset(new (std::nothrow) HeptaZigbee(serial_, mode));
  if (!api_) { last_error_ = "API allocation failed"; return false; }
  if (!api_->initialize()) {
    last_error_ = api_->error();
    api_.reset();
    return false;
  }
  return true;
}

bool Xbee::send(const char* text) {
  if (text == nullptr) {
    last_error_ = "text is null";
    return false;
  }

  return send(reinterpret_cast<const uint8_t*>(text), strlen(text));
}

bool Xbee::send(const uint8_t* data, size_t length) {
  if (data == nullptr) {
    last_error_ = "data is null";
    return false;
  }
  if (length == 0) {
    last_error_ = "data length is zero";
    return false;
  }

  if (managed_) {
    if (!api_) return false;
    last_error_ = nullptr;
    return api_->send(data, length);
  }
  if (serial_.write(data, length) != length) {
    last_error_ = "failed to send all data";
    return false;
  }

  last_error_ = nullptr;
  return true;
}

int Xbee::receive(char* buffer,
                  size_t buffer_size,
                  uint32_t timeout_ms) {
  if (buffer == nullptr) {
    last_error_ = "buffer is null";
    return XBEE_ERROR;
  }
  if (buffer_size == 0) {
    last_error_ = "buffer size is zero";
    return XBEE_ERROR;
  }

  buffer[0] = '\0';
  if (buffer_size == 1) {
    last_error_ = "buffer has no data capacity";
    return XBEE_ERROR;
  }

  int received = receive(reinterpret_cast<uint8_t*>(buffer),
                         buffer_size - 1,
                         timeout_ms);
  if (received < 0) {
    return received;
  }

  buffer[received] = '\0';
  return received;
}

int Xbee::receive(uint8_t* buffer,
                  size_t buffer_size,
                  uint32_t timeout_ms) {
  if (buffer == nullptr) {
    last_error_ = "buffer is null";
    return XBEE_ERROR;
  }
  if (buffer_size == 0) {
    last_error_ = "buffer size is zero";
    return XBEE_ERROR;
  }

  size_t received = 0;
  const uint32_t start_ms = millis();

  while (received < buffer_size) {
    while (available() && received < buffer_size) {
      int value = read_byte();
      if (value >= 0) {
        buffer[received++] = static_cast<uint8_t>(value);
      }
    }

    if (timeout_ms == 0 ||
        static_cast<uint32_t>(millis() - start_ms) >= timeout_ms) {
      break;
    }
    delay(1);
  }

  last_error_ = nullptr;
  return static_cast<int>(received);
}

int Xbee::receive_until(char terminator,
                        char* buffer,
                        size_t buffer_size,
                        uint32_t timeout_ms) {
  if (buffer == nullptr) {
    last_error_ = "buffer is null";
    return XBEE_ERROR;
  }
  if (buffer_size == 0) {
    last_error_ = "buffer size is zero";
    return XBEE_ERROR;
  }

  buffer[0] = '\0';
  size_t received = 0;
  const uint32_t start_ms = millis();

  while (true) {
    if (available()) {
      int value = read_byte();
      if (value < 0) {
        continue;
      }
      if (static_cast<char>(value) == terminator) {
        buffer[received] = '\0';
        last_error_ = nullptr;
        return static_cast<int>(received);
      }
      if (received >= buffer_size - 1) {
        buffer[buffer_size - 1] = '\0';
        last_error_ = "receive buffer overflow";
        return XBEE_BUFFER_OVERFLOW;
      }
      buffer[received++] = static_cast<char>(value);
      buffer[received] = '\0';
      continue;
    }

    if (timeout_ms == 0 ||
        static_cast<uint32_t>(millis() - start_ms) >= timeout_ms) {
      buffer[received] = '\0';
      last_error_ = "receive timeout";
      return 0;
    }
    delay(1);
  }
}

bool Xbee::available(void) {
  if (managed_) return api_ && api_->peek_byte() >= 0;
  return serial_.available() > 0;
}

int Xbee::read_byte(void) {
  if (managed_) return api_ ? api_->read_byte() : -1;
  return serial_.read();
}

size_t Xbee::write_byte(uint8_t data) {
  if (managed_) return send(&data, 1) ? 1 : 0;
  return serial_.write(data);
}

bool Xbee::enter_command_mode(void) {
  if (managed_) return false;
  clear_input();
  delay(XBEE_GUARD_TIME_MS);

  static const uint8_t command[] = {'+', '+', '+'};
  if (!send(command, sizeof(command))) {
    return false;
  }

  delay(XBEE_GUARD_TIME_MS);
  return wait_for_response("OK", XBEE_COMMAND_TIMEOUT_MS);
}

bool Xbee::exit_command_mode(void) {
  char response[16];
  if (!send_at_command("ATCN\r",
                       response,
                       sizeof(response),
                       XBEE_COMMAND_TIMEOUT_MS)) {
    return false;
  }
  if (strstr(response, "OK") == nullptr) {
    last_error_ = "ATCN did not return OK";
    return false;
  }

  last_error_ = nullptr;
  return true;
}

bool Xbee::send_at_command(const char* command,
                           char* response,
                           size_t response_size,
                           uint32_t timeout_ms) {
  if (managed_) return false;
  if (command == nullptr) {
    last_error_ = "AT command is null";
    return false;
  }
  if (response == nullptr) {
    last_error_ = "response buffer is null";
    return false;
  }
  if (response_size == 0) {
    last_error_ = "response buffer size is zero";
    return false;
  }

  response[0] = '\0';
  clear_input();
  if (!send(command)) {
    return false;
  }

  return read_response(response, response_size, timeout_ms) >= 0;
}

bool Xbee::set_transparent_mode(bool save) {
  return set_mode("ATAP0\r", save);
}

bool Xbee::set_api_mode(bool save) {
  return set_mode("ATAP1\r", save);
}

const char* Xbee::last_error(void) const {
  if (last_error_) return last_error_;
  return api_ ? api_->error() : "";
}

void Xbee::clear_input(void) {
  while (serial_.available() > 0) {
    serial_.read();
  }
}

bool Xbee::wait_for_response(const char* expected, uint32_t timeout_ms) {
  if (expected == nullptr) {
    last_error_ = "expected response is null";
    return false;
  }

  char response[32];
  int received = read_response(response, sizeof(response), timeout_ms);
  if (received < 0) {
    return false;
  }
  if (strstr(response, expected) == nullptr) {
    last_error_ = "unexpected XBee response";
    return false;
  }

  last_error_ = nullptr;
  return true;
}

int Xbee::read_response(char* buffer,
                        size_t buffer_size,
                        uint32_t timeout_ms) {
  if (buffer == nullptr) {
    last_error_ = "response buffer is null";
    return XBEE_ERROR;
  }
  if (buffer_size == 0) {
    last_error_ = "response buffer size is zero";
    return XBEE_ERROR;
  }

  buffer[0] = '\0';
  size_t received = 0;
  const uint32_t start_ms = millis();
  uint32_t last_byte_ms = start_ms;

  while (static_cast<uint32_t>(millis() - start_ms) < timeout_ms) {
    if (serial_.available() > 0) {
      int value = serial_.read();
      if (value < 0) {
        continue;
      }
      if (received >= buffer_size - 1) {
        buffer[buffer_size - 1] = '\0';
        last_error_ = "response buffer overflow";
        return XBEE_BUFFER_OVERFLOW;
      }

      buffer[received++] = static_cast<char>(value);
      buffer[received] = '\0';
      last_byte_ms = millis();
    } else {
      if (received > 0 &&
          static_cast<uint32_t>(millis() - last_byte_ms) >= 20) {
        last_error_ = nullptr;
        return static_cast<int>(received);
      }
      delay(1);
    }
  }

  if (received > 0) {
    last_error_ = nullptr;
    return static_cast<int>(received);
  }

  last_error_ = "response timeout";
  return XBEE_ERROR;
}

bool Xbee::set_mode(const char* command, bool save) {
  if (!enter_command_mode()) {
    return false;
  }

  char response[16];
  if (!send_at_command(command,
                       response,
                       sizeof(response),
                       XBEE_COMMAND_TIMEOUT_MS) ||
      strstr(response, "OK") == nullptr) {
    if (last_error_ == nullptr) {
      last_error_ = "ATAP did not return OK";
    }
    const char* mode_error = last_error_;
    exit_command_mode();
    last_error_ = mode_error;
    return false;
  }

  if (save) {
    if (!send_at_command("ATWR\r",
                         response,
                         sizeof(response),
                         XBEE_COMMAND_TIMEOUT_MS) ||
        strstr(response, "OK") == nullptr) {
      if (last_error_ == nullptr) {
        last_error_ = "ATWR did not return OK";
      }
      const char* write_error = last_error_;
      exit_command_mode();
      last_error_ = write_error;
      return false;
    }
  }

  return exit_command_mode();
}
