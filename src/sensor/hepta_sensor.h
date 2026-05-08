/**
 * @file hepta_sensor.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef HEPTA_SENSOR_H
#define HEPTA_SENSOR_H

#include <Arduino.h>

#include "../drv/imu9axis_bno055.h"

class HeptaSensor {
  public:
    HeptaSensor();

    bool begin(void);
    float get_temperature(void);
    void get_acceleration(float *ax, float *ay, float *az);
    void get_gyro(float *gx, float *gy, float *gz);
    void get_magnetometer(float *mx, float *my, float *mz);
    void print_acceleration(void);
    void print_gyro(void);
    void print_magnetometer(void);

  private:
    Bno055 bno055;

    const uint8_t _temp_pin = 13;
};


#endif /* HEPTA_SENSOR_H */
