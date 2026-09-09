/**
 * @file hepta_sensor_base.h
 * @brief Shared sensor implementation (BNO055 IMU; optional analog temperature ADC).
 *
 * Board-specific classes (HeptaSensor, HeptaLiteSensor) inherit from this base.
 * Full-board HeptaSensor supplies the temperature ADC pin via the protected
 * constructor. Lite uses the pin-less constructor (onboard BME280 instead).
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_SENSOR_BASE_H
#define HEPTA_SENSOR_BASE_H

#include <Arduino.h>

#include "../drv/imu9axis_bno055.h"


class HeptaSensorBase {
  public:
    bool begin(void);
    bool get_acceleration(float *ax, float *ay, float *az);
    bool get_gyro(float *gx, float *gy, float *gz);
    bool get_magnetometer(float *mx, float *my, float *mz);
    void print_acceleration(void);
    void print_gyro(void);
    void print_magnetometer(void);

  protected:
    HeptaSensorBase() : _temp_pin(0), _has_temp_pin(false) {}
    HeptaSensorBase(uint8_t temp_pin) : _temp_pin(temp_pin), _has_temp_pin(true) {}
    float read_temp_voltage(void);

    Bno055 bno055;
    const uint8_t _temp_pin;
    const bool _has_temp_pin;
};


#endif /* HEPTA_SENSOR_BASE_H */
