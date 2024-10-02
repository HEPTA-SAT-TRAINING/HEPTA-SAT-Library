/**
 * @file hepta_temperature.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef HEPTA_TEMPERATURE_H
#define HEPTA_TEMPERATURE_H

#include <Arduino.h>

class HeptaTemp {
  public:
    HeptaTemp();
    float get_temperature(void);
    // void temp_sense_u16(char* temp_u16);
    void temp_sensing_vol(float* voltage);

  private:
    //resistance
    float R1,R2,R3,R4,R5;
    float Pt;
    float R_1,R_2;

    //current
    float I;

    //voltage
    float Vref;

    //temperature coefficient
    float ce;
  
    // pin for A/D
    int _pin = 14;
};

#endif /* HEPTA_TEMPERATURE_H */
