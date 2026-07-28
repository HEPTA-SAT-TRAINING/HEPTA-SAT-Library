/**
 * @file camera_arducam_2mp.cpp
 * @brief Minimal Arducam Mini 2MP Plus driver (OV2640 + ArduChip).
 */

#include "camera_arducam_2mp.h"

#include <SPI.h>
#include <Wire.h>

CameraArducam2mp::CameraArducam2mp(uint8_t cs_pin) : _cs_pin(cs_pin) {}

bool CameraArducam2mp::begin(ArducamJpegSize jpeg_size) {
  if (_initialized && _jpeg_size == jpeg_size) {
    return true;
  }

  pinMode(_cs_pin, OUTPUT);
  _cs_high();

  Wire.begin();
  Wire.setClock(100000);

  // CPLD soft-reset used by Arducam Mini 2MP Plus examples.
  _spi_write_reg(CPLD_RESET_REG, 0x80);
  delay(100);
  _spi_write_reg(CPLD_RESET_REG, 0x00);
  delay(100);

  // ArduChip GPIO 0x06: sensor LDO on, out of reset, not in PWDN.
  const uint8_t gpio = _spi_read_reg(ARDUCHIP_GPIO);
  _spi_write_reg(ARDUCHIP_GPIO,
                 static_cast<uint8_t>((gpio | GPIO_PWREN_MASK | GPIO_RESET_MASK) &
                                      static_cast<uint8_t>(~GPIO_PWDN_MASK)));
  delay(100);

  if (!_probe_spi()) {
    Serial.println("Arducam: SPI probe failed.");
    invalidate();
    return false;
  }
  if (!_probe_sensor()) {
    Serial.println("Arducam: OV2640 not found.");
    invalidate();
    return false;
  }

  // Official ArduCAM InitCAM: soft-reset sensor bank, wait, then load tables.
  _sensor_write(0xFF, 0x01);
  _sensor_write(0x12, 0x80);
  delay(100);

  if (!_load_jpeg_tables(jpeg_size)) {
    Serial.println("Arducam: JPEG init failed.");
    invalidate();
    return false;
  }

  // Re-assert JPEG mode after size tables (OV2640_JPEG apply sequence).
  _sensor_write(0xFF, 0x00);
  _sensor_write(0xE0, 0x14);
  _sensor_write(0xDA, 0x10);
  _sensor_write(0xE0, 0x00);

  // Official ArduCAM examples settle ~1s after size tables.
  delay(1000);

  _jpeg_size = jpeg_size;
  _initialized = true;
  _fifo_len = 0;
  _fifo_remaining = 0;
  return true;
}

uint32_t CameraArducam2mp::take_picture(void) {
  if (!_initialized) {
    return 0;
  }

  _fifo_len = 0;
  _fifo_remaining = 0;

  _spi_write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
  delay(1);
  _spi_write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);

  if (!_wait_capture_done()) {
    Serial.println("Arducam: capture timeout.");
    return 0;
  }

  const uint32_t len = _read_fifo_length();
  if (len == 0 || len >= MAX_FIFO_SIZE) {
    Serial.print("Arducam: bad FIFO length ");
    Serial.println(len);
    _spi_write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    return 0;
  }

  _fifo_len = len;
  _fifo_remaining = len;
  return len;
}

int CameraArducam2mp::get_image_data_packet(uint8_t* buf, size_t max_size) {
  if (buf == nullptr || max_size == 0) {
    return -1;
  }
  if (_fifo_remaining == 0) {
    return 0;
  }

  size_t to_read = max_size;
  if (to_read > _fifo_remaining) {
    to_read = _fifo_remaining;
  }

  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  _cs_low();
  // Re-issue burst on every chunk so CS can go high between SD writes.
  SPI.transfer(BURST_FIFO_READ);

  for (size_t i = 0; i < to_read; i++) {
    buf[i] = SPI.transfer(0x00);
  }

  _cs_high();
  SPI.endTransaction();

  _fifo_remaining -= to_read;
  if (_fifo_remaining == 0) {
    _spi_write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
  }
  return static_cast<int>(to_read);
}

void CameraArducam2mp::invalidate(void) {
  _initialized = false;
  _fifo_len = 0;
  _fifo_remaining = 0;
  pinMode(_cs_pin, OUTPUT);
  _cs_high();
}

void CameraArducam2mp::_cs_low(void) {
  digitalWrite(_cs_pin, LOW);
}

void CameraArducam2mp::_cs_high(void) {
  digitalWrite(_cs_pin, HIGH);
}

void CameraArducam2mp::_spi_write_reg(uint8_t addr, uint8_t data) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  _cs_low();
  SPI.transfer(addr | 0x80);
  SPI.transfer(data);
  _cs_high();
  SPI.endTransaction();
}

uint8_t CameraArducam2mp::_spi_read_reg(uint8_t addr) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));
  _cs_low();
  SPI.transfer(addr & 0x7F);
  uint8_t value = SPI.transfer(0x00);
  _cs_high();
  SPI.endTransaction();
  return value;
}

bool CameraArducam2mp::_sensor_write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(OV2640_I2C_ADDR);
  Wire.write(reg);
  Wire.write(val);
  // ArduCAM wrSensorReg8_8 paces SCCB with 1 ms after every write.
  const bool ok = Wire.endTransmission() == 0;
  delay(1);
  return ok;
}

bool CameraArducam2mp::_sensor_read(uint8_t reg, uint8_t* val) {
  if (val == nullptr) {
    return false;
  }
  Wire.beginTransmission(OV2640_I2C_ADDR);
  Wire.write(reg);
  // Match BNO055 on this board: repeated start before the read phase.
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom(static_cast<uint8_t>(OV2640_I2C_ADDR), static_cast<uint8_t>(1)) != 1) {
    return false;
  }
  *val = Wire.read();
  delay(1);
  return true;
}

bool CameraArducam2mp::_sensor_write_list(const Ov2640Reg* list) {
  if (list == nullptr) {
    return false;
  }
  for (size_t i = 0;; i++) {
    const uint8_t reg = list[i].reg;
    const uint8_t val = list[i].val;
    if (reg == 0xFF && val == 0xFF) {
      break;
    }
    if (!_sensor_write(reg, val)) {
      return false;
    }
  }
  return true;
}

bool CameraArducam2mp::_probe_spi(void) {
  _spi_write_reg(ARDUCHIP_TEST1, 0x55);
  return _spi_read_reg(ARDUCHIP_TEST1) == 0x55;
}

bool CameraArducam2mp::_probe_sensor(void) {
  uint8_t vid = 0;
  uint8_t pid = 0;
  if (!_sensor_write(0xFF, 0x01)) {
    return false;
  }
  if (!_sensor_read(OV2640_CHIPID_HIGH, &vid) ||
      !_sensor_read(OV2640_CHIPID_LOW, &pid)) {
    return false;
  }
  // OV2640: VID=0x26, PID=0x41 or 0x42
  return (vid == 0x26) && (pid == 0x41 || pid == 0x42);
}

bool CameraArducam2mp::_load_jpeg_tables(ArducamJpegSize jpeg_size) {
  if (!_sensor_write_list(OV2640_JPEG_INIT) ||
      !_sensor_write_list(OV2640_YUV422) ||
      !_sensor_write_list(OV2640_JPEG)) {
    return false;
  }
  if (!_sensor_write(0xFF, 0x01) || !_sensor_write(0x15, 0x00)) {
    return false;
  }

  const Ov2640Reg* size_table = OV2640_640x480_JPEG;
  switch (jpeg_size) {
    case ARDUCAM_JPEG_QQVGA:
      size_table = OV2640_160x120_JPEG;
      break;
    case ARDUCAM_JPEG_QVGA:
      size_table = OV2640_320x240_JPEG;
      break;
    case ARDUCAM_JPEG_VGA:
      size_table = OV2640_640x480_JPEG;
      break;
    case ARDUCAM_JPEG_SVGA:
      size_table = OV2640_800x600_JPEG;
      break;
    case ARDUCAM_JPEG_UXGA:
      size_table = OV2640_1600x1200_JPEG;
      break;
  }
  return _sensor_write_list(size_table);
}

bool CameraArducam2mp::_wait_capture_done(void) {
  uint32_t start = millis();
  while ((millis() - start) < CAPTURE_TIMEOUT_MS) {
    if (_spi_read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK) {
      return true;
    }
    delay(2);
  }
  return false;
}

uint32_t CameraArducam2mp::_read_fifo_length(void) {
  uint32_t len1 = _spi_read_reg(FIFO_SIZE1);
  uint32_t len2 = _spi_read_reg(FIFO_SIZE2);
  uint32_t len3 = _spi_read_reg(FIFO_SIZE3) & 0x7F;
  return ((len3 << 16) | (len2 << 8) | len1) & 0x07FFFFF;
}
