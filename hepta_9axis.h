/**
 * @file hepta_9axis.h
 * @author Masaki Naito
 * @brief Arduino Library for BMX055
 * @version 0.1
 * @date 2024-12-09
 * 
 * @copyright UNISEC all rights reserved.
 */


#ifndef HEPTA_9AXIS_H
#define HEPTA_9AXIS_H

#include <Arduino.h>

class Hepta9Axis {
  public:
    /**
     * @brief Begin transmittion with BMX055
     */
    void begin(void);

    /**
     * @brief Get acceleration data from BMX055
     * @note unit is m/s^2, range is +-4G
     */
    void sen_acc(float *ax, float *ay, float *az);

    /**
     * @brief Get gyro data from BMX 055
     * @note unit is deg/s, range is +-125deg/s
     */
    void sen_gyro(float *gx, float *gy, float *gz);

    /**
     * @brief Get magnetometer data from BMX055
     * @note unit is uT
     */
    void sen_mag(float *mx, float *my, float *mz);

    void print_acc(void);
    void print_gyro(void);
    void print_mag(void);

  private: 
    uint8_t data[8];
    const uint8_t I2C_ADDR_ACC = 0x19;
    const uint8_t I2C_ADDR_GYRO = 0x69;
    const uint8_t I2C_ADDR_MAG = 0x13;
};


#endif /* HEPTA_9AXIS_H */
