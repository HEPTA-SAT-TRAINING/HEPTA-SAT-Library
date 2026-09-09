/**
 * @file bme280_bosch.h
 * @author Masaki Naito
 * @brief Bosch BME280 temperature / humidity / pressure driver (I2C)
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef BME280_BOSCH_H
#define BME280_BOSCH_H

#include <Arduino.h>
#include <Wire.h>

class Bme280 {
  public:
    /**
     * @brief Initialize the BME280 on I2C.
     * @param addr I2C address (0x76 or 0x77). Pass 0 to try both.
     * @param sda_pin SDA pin (default GP6 on the HEPTA-SAT payload bus)
     * @param scl_pin SCL pin (default GP7 on the HEPTA-SAT payload bus)
     * @param wire I2C bus (default `&Wire1` for Full payload; Lite onboard uses `&Wire`)
     */
    bool begin(uint8_t addr = 0, uint8_t sda_pin = 6, uint8_t scl_pin = 7,
               TwoWire *wire = &Wire1);

    /**
     * @brief Read compensated temperature, humidity, and pressure.
     * @param temp_c Temperature in degrees Celsius
     * @param hum_pct Relative humidity in percent
     * @param press_hpa Pressure in hPa
     */
    bool read(float *temp_c, float *hum_pct, float *press_hpa);

  private:
    bool _probe_address(uint8_t addr);
    bool _load_calibration(void);
    bool _configure_sensor(void);
    bool _read_raw(int32_t *raw_temp, int32_t *raw_press, int32_t *raw_hum);
    float _compensate_temperature(int32_t adc_T);
    uint32_t _compensate_pressure(int32_t adc_P);
    uint32_t _compensate_humidity(int32_t adc_H);

    bool _write_reg(uint8_t reg, uint8_t value);
    bool _read_reg(uint8_t reg, uint8_t *value);
    bool _read_bytes(uint8_t reg, uint8_t *data, size_t length);

    TwoWire *_wire = &Wire1;
    uint8_t _i2c_addr = 0;
    bool _initialized = false;
    int32_t _t_fine = 0;

    uint16_t _dig_T1 = 0;
    int16_t _dig_T2 = 0;
    int16_t _dig_T3 = 0;
    uint16_t _dig_P1 = 0;
    int16_t _dig_P2 = 0;
    int16_t _dig_P3 = 0;
    int16_t _dig_P4 = 0;
    int16_t _dig_P5 = 0;
    int16_t _dig_P6 = 0;
    int16_t _dig_P7 = 0;
    int16_t _dig_P8 = 0;
    int16_t _dig_P9 = 0;
    uint8_t _dig_H1 = 0;
    int16_t _dig_H2 = 0;
    uint8_t _dig_H3 = 0;
    int16_t _dig_H4 = 0;
    int16_t _dig_H5 = 0;
    int8_t _dig_H6 = 0;

    static const uint8_t CHIP_ID_VALUE = 0x60;
};

#endif /* BME280_BOSCH_H */
