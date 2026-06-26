/**
 * @file light_temt6000.cpp
 * @author Masaki Naito
 * @brief SparkFun TEMT6000 ambient light sensor via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#include "light_temt6000.h"

bool LightTemt6000::begin(AdcMcp3208 *adc, uint8_t channel) {
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

bool LightTemt6000::begin(uint8_t cs_pin, uint8_t channel, float ref_vol) {
  _owned_adc.begin(cs_pin, ref_vol);
  _ref_voltage = ref_vol;
  return begin(&_owned_adc, channel);
}

bool LightTemt6000::begin(bool use_mcp3208, uint8_t direct_adc_pin, uint8_t channel,
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

uint16_t LightTemt6000::get_raw(void) {
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

float LightTemt6000::get_voltage(void) {
  if (!_initialized) {
    return 0.0f;
  }
  if (_use_direct_adc) {
    return (static_cast<float>(get_raw()) * _ref_voltage) / 4096.0f;
  }
  if (_adc == NULL) {
    return 0.0f;
  }
  return _adc->get_voltage(_channel);
}

float LightTemt6000::get_lux(void) {
  // TEMT6000 output is not calibrated lux; this is a coarse training estimate.
  return get_voltage() * 200.0f;
}
