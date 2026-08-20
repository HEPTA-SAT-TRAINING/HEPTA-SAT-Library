/**
 * @file uv_guva_s12sd.h
 * @author Masaki Naito
 * @brief Grove GUVA-S12SD UV sensor via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef UV_GUVA_S12SD_H
#define UV_GUVA_S12SD_H

#include <Arduino.h>

#include "adc_mcp3208.h"

class UvGuvaS12sd {
  public:
    bool begin(AdcMcp3208 *adc, uint8_t channel = 5);
    bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3f);

    /**
     * @brief Initialize using MCP3208 or the MCU ADC pin (GP26–GP29).
     */
    bool begin(bool use_mcp3208, uint8_t direct_adc_pin, uint8_t channel, uint8_t cs_pin,
               float ref_vol = 3.3f);

    uint16_t get_raw(void);
    float get_voltage(void);

    /** @brief UV illumination in mW/m^2 (16-sample average). */
    float get_illumination_mw_m2(void);

    /** @brief UV index derived from illumination / 200. */
    float get_uv_index(void);

  private:
    float _average_voltage(void);

    AdcMcp3208 *_adc = NULL;
    AdcMcp3208 _owned_adc;
    uint8_t _channel = 5;
    uint8_t _direct_adc_pin = 28;
    float _ref_voltage = 3.3f;
    bool _use_direct_adc = false;
    bool _initialized = false;

    static const uint8_t SAMPLE_COUNT = 16;
    static const float ILLUMINATION_SCALE;
    static const float UV_INDEX_DIVISOR;
};

#endif /* UV_GUVA_S12SD_H */
