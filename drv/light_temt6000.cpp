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

  _adc = adc;
  _channel = channel;
  _initialized = true;
  return true;
}

bool LightTemt6000::begin(uint8_t cs_pin, uint8_t channel, float ref_vol) {
  _owned_adc.begin(cs_pin, ref_vol);
  return begin(&_owned_adc, channel);
}

uint16_t LightTemt6000::get_raw(void) {
  if (!_initialized || _adc == NULL) {
    return 0;
  }
  return _adc->get_raw_data(_channel);
}

float LightTemt6000::get_voltage(void) {
  if (!_initialized || _adc == NULL) {
    return 0.0f;
  }
  return _adc->get_voltage(_channel);
}

float LightTemt6000::get_lux(void) {
  // TEMT6000 output is not calibrated lux; this is a coarse training estimate.
  return get_voltage() * 200.0f;
}
