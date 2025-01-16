/**
 * @file icm20948.h
 * @author Masaki Naito
 * @brief Arduino Library for ICM-20948
 * @version 0.1
 * @date 2025-01-16
 * 
 * @copyright UNISEC all rights reserved.
 */


#ifndef ICM20948_H_H
#define ICM20948_H_H

#include <Arduino.h>

class Icm20948 {
  public:
    /**
     * @brief Begin transmittion
     */
    void begin(void);

    /**
     * @brief Get acceleration data
     * @note unit is m/s^2, range is +-4G
     */
    void sen_acc(float *ax, float *ay, float *az);

    /**
     * @brief Get gyro data
     * @note unit is deg/s, range is +-125deg/s
     */
    void sen_gyro(float *gx, float *gy, float *gz);

    /**
     * @brief Get magnetometer data
     * @note unit is uT
     */
    void sen_mag(float *mx, float *my, float *mz);

    void print_acc(void);
    void print_gyro(void);
    void print_mag(void);

  private: 
    uint8_t data[8];

    // 7bit address
    const uint8_t ICM20948_I2C_ADDR = 0x68;

    const uint8_t REG_WHO_AM_I = 0x00;
};


#endif /* ICM20948_H_H */
