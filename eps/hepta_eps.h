/**
 * @file hepta_eps.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef HEPTA_EPS_H
#define HEPTA_EPS_H


#include <Arduino.h>


class HeptaEps {
  public:
    /**
     * @brief Initialize the EPS
     */
    void init(void);

    /**
     * @brief Get the battery voltage
     * @return Battery line voltage in volts
     */
    float get_battery_voltage(void);

    /**
     * @brief Get the raw battery voltage reading from the ADC
     * @return Raw ADC value for battery voltage
     * 
     */
    uint16_t get_battery_voltage_raw(void);

    /**
     * @brief Get the current discharge
     * @return Current discharge in amperes
     */
    float get_current_discharge(void);

    /**
     * @brief Get the current charge
     * @return Current charge in amperes
     */
    float get_current_charge(void);

  private:
    const uint8_t _bat_vol_pin = 26;
    const uint8_t _current_discharge_pin = 27;
    const uint8_t _current_charge_pin = 28;

    const float _adc_ref_voltage = 3.3;
    const uint16_t _adc_max_value = 4096;

    const float galvano_gain = 20;
    const float galvano_resistance = 0.02;
};


#endif /* HEPTA_EPS_H */
