#include "hepta_storage.h"

#include <SPI.h>


bool HeptaStorage::begin(void) {
  _initialized = false;

  SPI.setRX(_rx_pin);
  SPI.setTX(_tx_pin);
  SPI.setSCK(_sck_pin);
  SPI.begin();
  delay(100);

  for (uint8_t attempt = 0; attempt < 3; attempt++) {
    if (SD.begin(_cs_pin)) {
      _initialized = true;
      break;
    }
    delay(100);
  }
  return _initialized;
}

bool HeptaStorage::is_available(void) const {
  return _initialized;
}

void HeptaStorage::invalidate(void) {
  _initialized = false;
}

File HeptaStorage::open(const char* path, int mode) {
  if (path == nullptr || !ensure_ready()) {
    return File();
  }

  File file = SD.open(path, mode);
  if (file) {
    return file;
  }

  // The card may have been inserted after boot or temporarily disconnected.
  // Reinitialize once before reporting the operation as failed.
  invalidate();
  if (!begin()) {
    return File();
  }
  return SD.open(path, mode);
}

bool HeptaStorage::exists(const char* path) {
  if (path == nullptr || !ensure_ready()) {
    return false;
  }
  return SD.exists(path);
}

bool HeptaStorage::remove(const char* path) {
  if (path == nullptr || !ensure_ready()) {
    return false;
  }

  if (SD.remove(path)) {
    return true;
  }

  invalidate();
  return begin() && SD.remove(path);
}

bool HeptaStorage::ensure_ready(void) {
  return _initialized || begin();
}
