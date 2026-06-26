/**
 * @file light_temt6000.h
 * @author Masaki Naito
 * @brief SparkFun TEMT6000 ambient light sensor via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef LIGHT_TEMT6000_H
#define LIGHT_TEMT6000_H

#include <Arduino.h>

#include "adc_mcp3208.h"

class LightTemt6000 {
  public:
    bool begin(AdcMcp3208 *adc, uint8_t channel = 7);
    bool begin(uint8_t cs_pin, uint8_t channel = 7, float ref_vol = 3.3f);

    uint16_t get_raw(void);
    float get_voltage(void);

    /**
     * @brief Approximate illuminance in lux.
     * @note Rough conversion: lux ~= voltage [V] * 200 (module-dependent).
     */
    float get_lux(void);

  private:
    AdcMcp3208 *_adc = NULL;
    AdcMcp3208 _owned_adc;
    uint8_t _channel = 7;
    bool _initialized = false;
};

#endif /* LIGHT_TEMT6000_H */
