/**
 * @file uv_guva_s12sd.cpp
 * @author Masaki Naito
 * @brief Grove GUVA-S12SD UV sensor via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#include "uv_guva_s12sd.h"

const float UvGuvaS12sd::ILLUMINATION_SCALE = 307.0f;
const float UvGuvaS12sd::UV_INDEX_DIVISOR = 200.0f;

bool UvGuvaS12sd::begin(AdcMcp3208 *adc, uint8_t channel) {
  if (adc == NULL || channel > 7) {
    return false;
  }

  _use_direct_adc = false;
  _adc = adc;
  _channel = channel;
  _ref_voltage = 3.3f;
  _initialized = true;
  return true;
}

bool UvGuvaS12sd::begin(uint8_t cs_pin, uint8_t channel, float ref_vol) {
  _owned_adc.begin(cs_pin, ref_vol);
  _ref_voltage = ref_vol;
  return begin(&_owned_adc, channel);
}

bool UvGuvaS12sd::begin(bool use_mcp3208, uint8_t direct_adc_pin, uint8_t channel,
                        uint8_t cs_pin, float ref_vol) {
  if (use_mcp3208) {
    return begin(cs_pin, channel, ref_vol);
  }

  if (direct_adc_pin < 26 || direct_adc_pin > 29) {
    return false;
  }

  _use_direct_adc = true;
  _adc = NULL;
  _direct_adc_pin = direct_adc_pin;
  _ref_voltage = ref_vol;
  _initialized = true;

  analogReadResolution(12);
  pinMode(_direct_adc_pin, INPUT);
  return true;
}

uint16_t UvGuvaS12sd::get_raw(void) {
  if (!_initialized) {
    return 0;
  }
  if (_use_direct_adc) {
    return static_cast<uint16_t>(analogRead(_direct_adc_pin));
  }
  if (_adc == NULL) {
    return 0;
  }
  return _adc->get_raw_data(_channel);
}

float UvGuvaS12sd::get_voltage(void) {
  return _average_voltage();
}

float UvGuvaS12sd::get_illumination_mw_m2(void) {
  return ILLUMINATION_SCALE * _average_voltage();
}

float UvGuvaS12sd::get_uv_index(void) {
  return get_illumination_mw_m2() / UV_INDEX_DIVISOR;
}

float UvGuvaS12sd::_average_voltage(void) {
  if (!_initialized) {
    return 0.0f;
  }

  uint32_t raw_sum = 0;
  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    if (_use_direct_adc) {
      raw_sum += analogRead(_direct_adc_pin);
    } else if (_adc != NULL) {
      raw_sum += _adc->get_raw_data(_channel);
    }
  }

  float average_raw = static_cast<float>(raw_sum) / SAMPLE_COUNT;
  return (average_raw * _ref_voltage) / 4096.0f;
}
