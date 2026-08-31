#include "hepta_storage.h"

#include <SPI.h>


// RP2040 SDFS keeps its SdFat instance alive when filesystem mounting fails,
// but only exposes it as a protected member and its end() implementation does
// not call SdFat::end().  Formatting must reuse that initialized card session;
// opening a second session causes CMD0 failures on the same SPI device.
class HeptaSDFSAccess : public sdfs::SDFSImpl {
  public:
    SdFat* fs(void) { return getFs(); }
};

class HeptaFSAccess : public fs::FS {
  public:
    fs::FSImplPtr impl(void) { return getImpl(); }
};

static SdFat* sdfs_fat_instance(void) {
  fs::FSImplPtr fsImpl = reinterpret_cast<HeptaFSAccess*>(&SDFS)->impl();
  sdfs::SDFSImpl* impl = static_cast<sdfs::SDFSImpl*>(fsImpl.get());
  return reinterpret_cast<HeptaSDFSAccess*>(impl)->fs();
}


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

  if (SD.remove(path) || !SD.exists(path)) {
    return true;
  }

  // A failed operation can leave SdFat mounted with stale cache state.
  // Force a real remount before retrying instead of only toggling our flag.
  SD.end(false);
  invalidate();
  if (!begin()) {
    return false;
  }
  if (SD.remove(path) || !SD.exists(path)) {
    return true;
  }

  // Deleting a file with a damaged cluster chain may fail while opening and
  // truncating it still succeeds.  This fallback is only used for an explicit
  // delete request, and prevents a corrupt large file from becoming undeletable.
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  bool truncated = file.truncate(0);
  file.close();
  if (!truncated) {
    return false;
  }

  return SD.remove(path) || !SD.exists(path);
}

bool HeptaStorage::format(void) {
  _format_error_stage = 0;
  _format_error_code = 0;
  _format_error_data = 0;
  SdFat* activeFs = sdfs_fat_instance();
  SdCard* card = activeFs == nullptr ? nullptr : activeFs->card();
  if (card == nullptr || card->errorCode()) {
    _format_error_stage = 1;
    if (card != nullptr) {
      _format_error_code = card->errorCode();
      _format_error_data = card->errorData();
    }
    begin();
    return false;
  }

  uint8_t sectorBuffer[512] __attribute__((aligned(4)));
  FsFormatter formatter;
  if (!formatter.format(card, sectorBuffer, nullptr)) {
    _format_error_stage = 2;
    _format_error_code = card->errorCode();
    _format_error_data = card->errorData();
    begin();
    return false;
  }

  activeFs->end();
  SD.end(false);
  invalidate();
  delay(250);
  if (!begin()) {
    _format_error_stage = 3;
    return false;
  }
  return true;
}

bool HeptaStorage::list_files(bool (*callback)(const char* name, uint32_t size, void* ctx),
                                void* ctx) {
  if (callback == nullptr || !ensure_ready()) {
    return false;
  }

  File root = SD.open("/");
  if (!root) {
    invalidate();
    if (!begin()) {
      return false;
    }
    root = SD.open("/");
    if (!root) {
      return false;
    }
  }

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }

    if (!entry.isDirectory()) {
      if (!callback(entry.name(), entry.size(), ctx)) {
        entry.close();
        root.close();
        return false;
      }
    }
    entry.close();
  }

  root.close();
  return true;
}

bool HeptaStorage::ensure_ready(void) {
  return _initialized || begin();
}
