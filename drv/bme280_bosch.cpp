/**
 * @file bme280_bosch.cpp
 * @author Masaki Naito
 * @brief Bosch BME280 temperature / humidity / pressure driver (I2C)
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#include "bme280_bosch.h"

#include <Wire.h>

enum Bme280Reg : uint8_t {
  BME280_REG_CHIP_ID = 0xD0,
  BME280_REG_RESET = 0xE0,
  BME280_REG_CTRL_HUM = 0xF2,
  BME280_REG_STATUS = 0xF3,
  BME280_REG_CTRL_MEAS = 0xF4,
  BME280_REG_CONFIG = 0xF5,
  BME280_REG_PRESS_MSB = 0xF7,
  BME280_REG_TEMP_MSB = 0xFA,
  BME280_REG_HUM_MSB = 0xFD,
  BME280_REG_CALIB00 = 0x88,
  BME280_REG_CALIB26 = 0xE1,
};

static int16_t sign_extend_humidity_cal(int16_t value) {
  if (value > 0x7FF) {
    value -= 0x1000;
  }
  return value;
}

bool Bme280::begin(uint8_t addr, uint8_t sda_pin, uint8_t scl_pin) {
  Wire1.setSDA(sda_pin);
  Wire1.setSCL(scl_pin);
  Wire1.begin();

  _initialized = false;
  _i2c_addr = 0;

  if (addr != 0) {
    if (!_probe_address(addr)) {
      return false;
    }
  } else if (!_probe_address(0x76) && !_probe_address(0x77)) {
    return false;
  }

  if (!_write_reg(BME280_REG_RESET, 0xB6)) {
    return false;
  }

  {
    const uint32_t timeout_ms = 100;
    const uint32_t start_ms = millis();
    while (true) {
      uint8_t status = 0;
      if (!_read_reg(BME280_REG_STATUS, &status)) {
        return false;
      }
      if ((status & 0x01) == 0) {
        break;
      }
      if (millis() - start_ms >= timeout_ms) {
        return false;
      }
      delay(1);
    }
  }

  if (!_load_calibration() || !_configure_sensor()) {
    return false;
  }

  _initialized = true;
  return true;
}

bool Bme280::read(float *temp_c, float *hum_pct, float *press_hpa) {
  if (!_initialized || temp_c == NULL || hum_pct == NULL || press_hpa == NULL) {
    return false;
  }

  int32_t raw_temp = 0;
  int32_t raw_press = 0;
  int32_t raw_hum = 0;
  if (!_read_raw(&raw_temp, &raw_press, &raw_hum)) {
    return false;
  }

  *temp_c = _compensate_temperature(raw_temp);
  *press_hpa = static_cast<float>(_compensate_pressure(raw_press)) / 256.0f / 100.0f;
  *hum_pct = static_cast<float>(_compensate_humidity(raw_hum)) / 1024.0f;
  return true;
}

bool Bme280::_probe_address(uint8_t addr) {
  uint8_t chip_id = 0;
  _i2c_addr = addr;
  if (!_read_reg(BME280_REG_CHIP_ID, &chip_id) || chip_id != CHIP_ID_VALUE) {
    _i2c_addr = 0;
    return false;
  }
  return true;
}

bool Bme280::_load_calibration(void) {
  uint8_t calib[26] = {0};

  if (!_read_bytes(BME280_REG_CALIB00, calib, 26)) {
    return false;
  }

  _dig_T1 = static_cast<uint16_t>(calib[0]) | (static_cast<uint16_t>(calib[1]) << 8);
  _dig_T2 = static_cast<int16_t>(calib[2]) | (static_cast<int16_t>(calib[3]) << 8);
  _dig_T3 = static_cast<int16_t>(calib[4]) | (static_cast<int16_t>(calib[5]) << 8);
  _dig_P1 = static_cast<uint16_t>(calib[6]) | (static_cast<uint16_t>(calib[7]) << 8);
  _dig_P2 = static_cast<int16_t>(calib[8]) | (static_cast<int16_t>(calib[9]) << 8);
  _dig_P3 = static_cast<int16_t>(calib[10]) | (static_cast<int16_t>(calib[11]) << 8);
  _dig_P4 = static_cast<int16_t>(calib[12]) | (static_cast<int16_t>(calib[13]) << 8);
  _dig_P5 = static_cast<int16_t>(calib[14]) | (static_cast<int16_t>(calib[15]) << 8);
  _dig_P6 = static_cast<int16_t>(calib[16]) | (static_cast<int16_t>(calib[17]) << 8);
  _dig_P7 = static_cast<int16_t>(calib[18]) | (static_cast<int16_t>(calib[19]) << 8);
  _dig_P8 = static_cast<int16_t>(calib[20]) | (static_cast<int16_t>(calib[21]) << 8);
  _dig_P9 = static_cast<int16_t>(calib[22]) | (static_cast<int16_t>(calib[23]) << 8);
  _dig_H1 = calib[25];

  uint8_t calib_h[7] = {0};
  if (!_read_bytes(BME280_REG_CALIB26, calib_h, 7)) {
    return false;
  }

  _dig_H2 = static_cast<int16_t>(calib_h[0]) | (static_cast<int16_t>(calib_h[1]) << 8);
  _dig_H3 = calib_h[2];
  _dig_H4 = sign_extend_humidity_cal(
      static_cast<int16_t>(calib_h[3] << 4) | static_cast<int16_t>(calib_h[4] & 0x0F));
  _dig_H5 = sign_extend_humidity_cal(
      static_cast<int16_t>(calib_h[5] << 4) | static_cast<int16_t>(calib_h[4] >> 4));
  _dig_H6 = static_cast<int8_t>(calib_h[6]);
  return true;
}

bool Bme280::_configure_sensor(void) {
  if (!_write_reg(BME280_REG_CTRL_HUM, 0x02)) {
    return false;
  }
  if (!_write_reg(BME280_REG_CONFIG, 0x10)) {
    return false;
  }
  if (!_write_reg(BME280_REG_CTRL_MEAS, 0x57)) {
    return false;
  }

  delay(100);
  return true;
}

bool Bme280::_read_raw(int32_t *raw_temp, int32_t *raw_press, int32_t *raw_hum) {
  uint8_t data[8] = {0};
  if (!_read_bytes(BME280_REG_PRESS_MSB, data, 8)) {
    return false;
  }

  *raw_press = (static_cast<int32_t>(data[0]) << 12) |
               (static_cast<int32_t>(data[1]) << 4) |
               (static_cast<int32_t>(data[2]) >> 4);
  *raw_temp = (static_cast<int32_t>(data[3]) << 12) |
              (static_cast<int32_t>(data[4]) << 4) |
              (static_cast<int32_t>(data[5]) >> 4);
  *raw_hum = (static_cast<int32_t>(data[6]) << 8) | static_cast<int32_t>(data[7]);
  return true;
}

float Bme280::_compensate_temperature(int32_t adc_T) {
  int32_t var1 = ((((adc_T >> 3) - static_cast<int32_t>(_dig_T1 << 1))) *
                 static_cast<int32_t>(_dig_T2)) >>
                11;
  int32_t var2 = (((((adc_T >> 4) - static_cast<int32_t>(_dig_T1)) *
                    ((adc_T >> 4) - static_cast<int32_t>(_dig_T1))) >>
                   12) *
                  static_cast<int32_t>(_dig_T3)) >>
                 14;
  _t_fine = var1 + var2;
  return static_cast<float>((_t_fine * 5 + 128) >> 8) / 100.0f;
}

uint32_t Bme280::_compensate_pressure(int32_t adc_P) {
  int64_t var1 = static_cast<int64_t>(_t_fine) - 128000;
  int64_t var2 = var1 * var1 * static_cast<int64_t>(_dig_P6);
  var2 = var2 + ((var1 * static_cast<int64_t>(_dig_P5)) << 17);
  var2 = var2 + (static_cast<int64_t>(_dig_P4) << 35);
  var1 = ((var1 * var1 * static_cast<int64_t>(_dig_P3)) >> 8) +
         ((var1 * static_cast<int64_t>(_dig_P2)) << 12);
  var1 = (((static_cast<int64_t>(1) << 47) + var1) * static_cast<int64_t>(_dig_P1)) >> 33;
  if (var1 == 0) {
    return 0;
  }

  int64_t pressure = 1048576 - adc_P;
  pressure = (((pressure << 31) - var2) * 3125) / var1;
  var1 = (static_cast<int64_t>(_dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
  var2 = (static_cast<int64_t>(_dig_P8) * pressure) >> 19;
  pressure = ((pressure + var1 + var2) >> 8) +
             (static_cast<int64_t>(_dig_P7) << 4);
  return static_cast<uint32_t>(pressure);
}

uint32_t Bme280::_compensate_humidity(int32_t adc_H) {
  int32_t var1 = _t_fine - static_cast<int32_t>(76800);
  int32_t v_x1_u32r = (((((adc_H << 14) - (static_cast<int32_t>(_dig_H4) << 20) -
                          (static_cast<int32_t>(_dig_H5) * var1)) +
                         static_cast<int32_t>(16384)) >>
                        15) *
                       (((((((var1 * static_cast<int32_t>(_dig_H6)) >> 10) *
                            (((var1 * static_cast<int32_t>(_dig_H3)) >> 11) +
                             static_cast<int32_t>(32768))) >>
                           10) +
                          static_cast<int32_t>(2097152)) *
                             static_cast<int32_t>(_dig_H2) +
                         static_cast<int32_t>(16384)) >>
                        15));

  v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                            static_cast<int32_t>(_dig_H1)) >>
                           4);
  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  return static_cast<uint32_t>(v_x1_u32r >> 12);
}

bool Bme280::_write_reg(uint8_t reg, uint8_t value) {
  Wire1.beginTransmission(_i2c_addr);
  Wire1.write(reg);
  Wire1.write(value);
  return Wire1.endTransmission() == 0;
}

bool Bme280::_read_reg(uint8_t reg, uint8_t *value) {
  return _read_bytes(reg, value, 1);
}

bool Bme280::_read_bytes(uint8_t reg, uint8_t *data, size_t length) {
  if (data == NULL || length == 0) {
    return false;
  }

  Wire1.beginTransmission(_i2c_addr);
  Wire1.write(reg);
  if (Wire1.endTransmission(false) != 0) {
    return false;
  }

  size_t received = Wire1.requestFrom(_i2c_addr, static_cast<uint8_t>(length));
  if (received != length) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    data[i] = Wire1.read();
  }
  return true;
}
