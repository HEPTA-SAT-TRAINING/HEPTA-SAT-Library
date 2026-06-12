#include "hepta_com_base.h"

#include "hepta_image_tx.h"

#include <SD.h>
#include <string.h>


bool HeptaComBase::begin(void) {
  return xbee_.begin();
}

bool HeptaComBase::begin(uint16_t baud_rate) {
  return xbee_.begin(baud_rate);
}

bool HeptaComBase::send(const char* text) {
  return xbee_.send(text);
}

bool HeptaComBase::send(const uint8_t* data, size_t length) {
  return xbee_.send(data, length);
}

size_t HeptaComBase::print(const String& text) {
  return print(text.c_str());
}

size_t HeptaComBase::print(const char* text) {
  if (text == nullptr) {
    return 0;
  }

  size_t length = strlen(text);
  return send(text) ? length : 0;
}

size_t HeptaComBase::println(const String& text) {
  return println(text.c_str());
}

size_t HeptaComBase::println(const char* text) {
  if (text == nullptr) {
    return 0;
  }

  size_t n = print(text);
  n += print("\n");
  return n;
}

size_t HeptaComBase::printf(const char* format, ...) {
  if (format == nullptr) {
    return 0;
  }

  va_list args;
  va_start(args, format);
  size_t n = vprintf(format, args);
  va_end(args);
  return n;
}

size_t HeptaComBase::vprintf(const char* format, va_list args) {
  if (format == nullptr) {
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
    return send(buffer) ? (size_t)needed : 0;
  }

  size_t size = (size_t)needed + 1;
  char* heap = (char*)malloc(size);
  if (heap == nullptr) {
    return send(buffer) ? sizeof(buffer) - 1 : 0;
  }

  vsnprintf(heap, size, format, args);
  size_t written = send(heap) ? (size_t)needed : 0;
  free(heap);
  return written;
}

int HeptaComBase::receive(char* buffer,
                          size_t buffer_size,
                          uint32_t timeout_ms) {
  return xbee_.receive(buffer, buffer_size, timeout_ms);
}

int HeptaComBase::receive(uint8_t* buffer,
                          size_t buffer_size,
                          uint32_t timeout_ms) {
  return xbee_.receive(buffer, buffer_size, timeout_ms);
}

int HeptaComBase::receive_until(char terminator,
                                char* buffer,
                                size_t buffer_size,
                                uint32_t timeout_ms) {
  return xbee_.receive_until(terminator,
                             buffer,
                             buffer_size,
                             timeout_ms);
}

bool HeptaComBase::available(void) {
  return xbee_.available();
}

bool HeptaComBase::enter_command_mode(void) {
  return xbee_.enter_command_mode();
}

bool HeptaComBase::exit_command_mode(void) {
  return xbee_.exit_command_mode();
}

bool HeptaComBase::send_at_command(const char* command,
                                   char* response,
                                   size_t response_size,
                                   uint32_t timeout_ms) {
  return xbee_.send_at_command(command,
                               response,
                               response_size,
                               timeout_ms);
}

bool HeptaComBase::set_transparent_mode(bool save) {
  return xbee_.set_transparent_mode(save);
}

bool HeptaComBase::set_api_mode(bool save) {
  return xbee_.set_api_mode(save);
}

const char* HeptaComBase::last_error(void) const {
  return xbee_.last_error();
}

char HeptaComBase::get_char(void) {
  return static_cast<char>(xbee_.read_byte());
}

void HeptaComBase::send_char(const char c) {
  xbee_.write_byte(static_cast<uint8_t>(c));
}

void HeptaComBase::send_text(String text) {
  send(text.c_str());
}

String HeptaComBase::get_text(void) {
  String received_text = "";

  while (xbee_.available()) {
    int value = xbee_.read_byte();
    if (value >= 0) {
      received_text += static_cast<char>(value);
    }
  }

  return received_text;
}

bool HeptaComBase::downlink_image_file(const char* filename, uint16_t image_id) {
  if (filename == nullptr) {
    send_image_error(IMAGE_ERROR_NOT_AVAILABLE);
    return false;
  }

  File file = SD.open(filename, FILE_READ);
  if (!file) {
    send_image_error(IMAGE_ERROR_NOT_AVAILABLE);
    return false;
  }

  bool ok = HeptaImageTx::send_jpeg_file(*this, file, image_id);
  file.close();
  return ok;
}

void HeptaComBase::send_image_error(uint8_t error_code) {
  HeptaImageTx::send_error(*this, error_code);
}
