/**
 * @file hepta_sensor_base.h
 * @brief Shared sensor implementation (BNO055 IMU + analog temperature ADC).
 *
 * Board-specific classes (HeptaSensor, HeptaLiteSensor) inherit from this base,
 * supply the temperature ADC pin via the protected constructor, and add their
 * own get_temperature() with the board's conversion formula.
 * Sketches do not include this header directly.
 */

#ifndef HEPTA_SENSOR_BASE_H
#define HEPTA_SENSOR_BASE_H

#include <Arduino.h>

#include "../drv/imu9axis_bno055.h"


class HeptaSensorBase {
  public:
    bool begin(void);
    void get_acceleration(float *ax, float *ay, float *az);
    void get_gyro(float *gx, float *gy, float *gz);
    void get_magnetometer(float *mx, float *my, float *mz);
    void print_acceleration(void);
    void print_gyro(void);
    void print_magnetometer(void);

  protected:
    HeptaSensorBase(uint8_t temp_pin) : _temp_pin(temp_pin) {}
    float read_temp_voltage(void);

    Bno055 bno055;
    const uint8_t _temp_pin;
};


#endif /* HEPTA_SENSOR_BASE_H */
