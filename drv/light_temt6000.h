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
    /**
     * @brief Initialize with an existing MCP3208 driver instance.
     * @param channel MCP3208 channel (V4.1.1 USER1/2/3 = CH5/6/7)
     */
    bool begin(AdcMcp3208 *adc, uint8_t channel = 5);

    /**
     * @brief Initialize a local MCP3208 on the given chip-select pin.
     * @param channel MCP3208 channel (V4.1.1 USER1/2/3 = CH5/6/7)
     */
    bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3f);

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
    uint8_t _channel = 5;
    bool _initialized = false;
};

#endif /* LIGHT_TEMT6000_H */
