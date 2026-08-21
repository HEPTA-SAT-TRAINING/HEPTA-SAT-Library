/**
 * @file hc_sr04.cpp
 * @brief HC-SR04 ultrasonic distance sensor (GPIO TRIG / ECHO)
 * @version 0.1
 * @date 2026-08-21
 *
 * @copyright UNISEC all rights reserved.
 */

#include "hc_sr04.h"

// Speed of sound at ~20 °C: 343 m/s = 0.0343 cm/us (one-way uses / 2).
static constexpr float kCmPerUs = 0.0343f;

bool HcSr04::begin(uint8_t trig_pin, uint8_t echo_pin, uint32_t timeout_us) {
  if (timeout_us == 0) {
    return false;
  }

  _trig_pin = trig_pin;
  _echo_pin = echo_pin;
  _timeout_us = timeout_us;

  pinMode(_trig_pin, OUTPUT);
  pinMode(_echo_pin, INPUT);
  digitalWrite(_trig_pin, LOW);

  _initialized = true;
  return true;
}

void HcSr04::trigger_pulse(void) {
  digitalWrite(_trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trig_pin, LOW);
}

bool HcSr04::get_pulse_us(uint32_t *pulse_us) {
  if (!_initialized || pulse_us == NULL) {
    return false;
  }

  trigger_pulse();

  // Wait for Echo to go HIGH, then measure how long it stays HIGH.
  const uint32_t high_us = pulseIn(_echo_pin, HIGH, _timeout_us);
  if (high_us == 0) {
    return false;
  }

  *pulse_us = high_us;
  return true;
}

bool HcSr04::get_distance_cm(float *distance_cm) {
  if (distance_cm == NULL) {
    return false;
  }

  uint32_t pulse_us = 0;
  if (!get_pulse_us(&pulse_us)) {
    return false;
  }

  *distance_cm = (static_cast<float>(pulse_us) * kCmPerUs) / 2.0f;
  return true;
}

bool HcSr04::get_distance_m(float *distance_m) {
  if (distance_m == NULL) {
    return false;
  }

  float distance_cm = 0.0f;
  if (!get_distance_cm(&distance_cm)) {
    return false;
  }

  *distance_m = distance_cm / 100.0f;
  return true;
}
