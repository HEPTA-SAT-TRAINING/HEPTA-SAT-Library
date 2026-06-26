/**
 * @file air_quality_mp503.cpp
 * @author Masaki Naito
 * @brief Grove Air Quality Sensor (Winsen MP503) via MCP3208
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright UNISEC all rights reserved.
 */

#include "air_quality_mp503.h"

bool AirQualityMp503::begin(AdcMcp3208 *adc, uint8_t channel) {
  if (adc == NULL || channel > 7) {
    return false;
  }

  _adc = adc;
  _channel = channel;
  _initialized = false;

  delay(20000);

  uint16_t init_voltage = _read_adc();
  if (init_voltage <= INIT_MIN || init_voltage >= INIT_MAX) {
    return false;
  }

  _current_voltage = init_voltage;
  _last_voltage = init_voltage;
  _standard_voltage = init_voltage;
  _last_std_vol_updated = millis();
  _voltage_sum = 0;
  _vol_sum_count = 0;
  _initialized = true;
  return true;
}

bool AirQualityMp503::begin(uint8_t cs_pin, uint8_t channel, float ref_vol) {
  _owned_adc.begin(cs_pin, ref_vol);
  return begin(&_owned_adc, channel);
}

AirQualityMp503::QualityLevel AirQualityMp503::slope(void) {
  if (!_initialized) {
    return FRESH_AIR;
  }

  _last_voltage = _current_voltage;
  _current_voltage = _read_adc();

  _voltage_sum += _current_voltage;
  _vol_sum_count += 1;
  _update_standard_voltage();

  if (_current_voltage - _last_voltage > THRESH_FORCE_DELTA ||
      _current_voltage > THRESH_FORCE_LEVEL) {
    return FORCE_SIGNAL;
  }
  if ((_current_voltage - _last_voltage > THRESH_FORCE_DELTA &&
       _current_voltage < THRESH_FORCE_LEVEL) ||
      _current_voltage > _standard_voltage + THRESH_HIGH_STD) {
    return HIGH_POLLUTION;
  }
  if ((_current_voltage - _last_voltage > THRESH_LOW_DELTA &&
       _current_voltage < THRESH_FORCE_LEVEL) ||
      _current_voltage > _standard_voltage + THRESH_LOW_STD) {
    return LOW_POLLUTION;
  }
  return FRESH_AIR;
}

uint16_t AirQualityMp503::get_raw(void) const {
  return _current_voltage;
}

void AirQualityMp503::_update_standard_voltage(void) {
  if (millis() - _last_std_vol_updated > 500000UL) {
    if (_vol_sum_count > 0) {
      _standard_voltage = static_cast<uint16_t>(_voltage_sum / _vol_sum_count);
    }
    _last_std_vol_updated = millis();
    _voltage_sum = 0;
    _vol_sum_count = 0;
  }
}

uint16_t AirQualityMp503::_read_adc(void) {
  if (_adc == NULL) {
    return 0;
  }
  return _adc->get_raw_data(_channel);
}
