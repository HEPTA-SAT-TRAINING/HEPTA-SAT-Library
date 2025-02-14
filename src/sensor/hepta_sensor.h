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

#include "imu9axis_icm20948.h"
#include "adc_mcp3208.h"
#include "camera_c1098.h"

class HeptaSensor :public Icm20948, public CameraC1098 {
  public:
    HeptaSensor();
    float get_temperature(void);

  private:
    //resistance
    const float R1 = 2500.0;
    const float R2 = 2500.0;
    const float R3 = 110.0;
    const float R4 = 1000.0;
    const float R5 = 68000.0;
    const float Pt = 100.0;
    const float R_1 = 3.0;
    const float R_2 = 2.0;

    //current
    const float I = 0.001;

    //voltage
    const float Vref = 2.5;

    //temperature coefficient
    const float ce = 0.003851;
  
    // pin for A/D
    int _pin = 14;
};


#endif /* HEPTA_SENSOR_H */
