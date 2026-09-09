#include "hepta_lite_sensor.h"

#include <Wire.h>


bool HeptaLiteSensor::begin(void) {
  // IMU first (same Wire GP4/5 as the onboard BME280). Return the IMU
  // result even if the environmental sensor fails; getters then return NAN.
  bool ok = HeptaSensorBase::begin();
  _bme_ok = bme.begin(0x76, 4, 5, &Wire);
  _env_valid = false;
  return ok;
}

bool HeptaLiteSensor::_sample_env(void) {
  if (!_bme_ok) {
    return false;
  }

  // Reuse a sample taken in the last 50 ms so T/H/P getters share one
  // Bme280::read() when called in sequence.
  const unsigned long now = millis();
  if (_env_valid && (now - _env_millis) < 50UL) {
    return true;
  }

  _env_valid = bme.read(&_temp_c, &_hum_pct, &_press_hpa);
  _env_millis = now;
  if (!_env_valid) {
    _temp_c = NAN;
    _hum_pct = NAN;
    _press_hpa = NAN;
  }
  return _env_valid;
}

float HeptaLiteSensor::get_temperature(void) {
  if (!_sample_env()) {
    return NAN;
  }
  return _temp_c;
}

float HeptaLiteSensor::get_humidity(void) {
  if (!_sample_env()) {
    return NAN;
  }
  return _hum_pct;
}

float HeptaLiteSensor::get_pressure(void) {
  if (!_sample_env()) {
    return NAN;
  }
  return _press_hpa;
}
