/**
 * @file air_quality_mp503.h
 * @author Masaki Naito
 * @brief Grove Air Quality Sensor (Winsen MP503) via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef AIR_QUALITY_MP503_H
#define AIR_QUALITY_MP503_H

#include <Arduino.h>

#include "adc_mcp3208.h"

class AirQualityMp503 {
  public:
    enum QualityLevel : int8_t {
      FORCE_SIGNAL = 0,
      HIGH_POLLUTION = 1,
      LOW_POLLUTION = 2,
      FRESH_AIR = 3,
    };

    /**
     * @brief Initialize with an existing MCP3208 driver instance.
     * @param adc Pointer to a configured AdcMcp3208 (e.g. shared with EPS)
     * @param channel MCP3208 channel (V4.1.1 USER1/2/3 = CH5/6/7)
     * @param skip_warmup Skip the built-in 20 s warm-up delay when already done externally
     */
    bool begin(AdcMcp3208 *adc, uint8_t channel = 5, bool skip_warmup = false);

    /**
     * @brief Initialize a local MCP3208 on the given chip-select pin.
     * @param channel MCP3208 channel (V4.1.1 USER1/2/3 = CH5/6/7)
     */
    bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3f,
               bool skip_warmup = false);

    /**
     * @brief Classify air quality using the Seeed slope() algorithm.
     * Thresholds are scaled for 12-bit MCP3208 readings.
     */
    QualityLevel slope(void);

    /** @brief Latest raw ADC value (12-bit). */
    uint16_t get_raw(void) const;

  private:
    void _update_standard_voltage(void);
    uint16_t _read_adc(void);

    AdcMcp3208 *_adc = NULL;
    AdcMcp3208 _owned_adc;
    uint8_t _channel = 5;
    bool _initialized = false;

    uint16_t _current_voltage = 0;
    uint16_t _last_voltage = 0;
    uint16_t _standard_voltage = 0;
    uint32_t _voltage_sum = 0;
    uint32_t _vol_sum_count = 0;
    uint32_t _last_std_vol_updated = 0;

    // Seeed 10-bit thresholds scaled x4 for MCP3208 (12-bit).
    static const uint16_t THRESH_FORCE_DELTA = 1600;
    static const uint16_t THRESH_FORCE_LEVEL = 2800;
    static const uint16_t THRESH_HIGH_STD = 600;
    static const uint16_t THRESH_LOW_DELTA = 800;
    static const uint16_t THRESH_LOW_STD = 200;
    static const uint16_t INIT_MIN = 40;
    static const uint16_t INIT_MAX = 3192;
};

#endif /* AIR_QUALITY_MP503_H */
