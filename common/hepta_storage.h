/**
 * @file hepta_storage.h
 * @brief Recoverable SD-card access shared by CDH, COM, and camera capture.
 */

#ifndef HEPTA_STORAGE_H
#define HEPTA_STORAGE_H

#include <Arduino.h>
#include <SD.h>


class HeptaStorage {
  public:
    HeptaStorage(uint8_t cs, uint8_t tx, uint8_t rx, uint8_t sck)
      : _cs_pin(cs), _tx_pin(tx), _rx_pin(rx), _sck_pin(sck) {}

    bool begin(void);
    bool is_available(void) const;
    void invalidate(void);

    File open(const char* path, int mode = FILE_READ);
    bool exists(const char* path);
    bool remove(const char* path);
    bool format(void);
    uint8_t format_error_stage(void) const { return _format_error_stage; }
    uint8_t format_error_code(void) const { return _format_error_code; }
    uint8_t format_error_data(void) const { return _format_error_data; }
    bool list_files(bool (*callback)(const char* name, uint32_t size, void* ctx),
                    void* ctx = nullptr);
    bool rename(const char* from_path, const char* to_path);

  private:
    bool ensure_ready(void);

    const uint8_t _cs_pin;
    const uint8_t _tx_pin;
    const uint8_t _rx_pin;
    const uint8_t _sck_pin;
    bool _initialized = false;
    uint8_t _format_error_stage = 0;
    uint8_t _format_error_code = 0;
    uint8_t _format_error_data = 0;
};


#endif /* HEPTA_STORAGE_H */
