/**
 * @file hc_sr04.h
 * @brief HC-SR04 ultrasonic distance sensor (GPIO TRIG / ECHO)
 * @version 0.1
 * @date 2026-08-21
 *
 * @copyright UNISEC all rights reserved.
 */

#ifndef HC_SR04_H
#define HC_SR04_H

#include <Arduino.h>

class HcSr04 {
  public:
    /**
     * @brief Configure TRIG/ECHO pins.
     * @param trig_pin Trigger output pin
     * @param echo_pin Echo input pin
     * @param timeout_us Max wait for echo [us] (default ~4 m at 343 m/s)
     */
    bool begin(uint8_t trig_pin, uint8_t echo_pin,
               uint32_t timeout_us = 25000UL);

    /**
     * @brief Measure round-trip echo pulse width.
     * @param pulse_us Out: high-time of Echo [us]
     * @return true on success, false on timeout / not initialized
     */
    bool get_pulse_us(uint32_t *pulse_us);

    /**
     * @brief Measure distance in centimetres.
     * @param distance_cm Out: one-way distance [cm]
     * @return true on success, false on timeout / not initialized
     */
    bool get_distance_cm(float *distance_cm);

    /**
     * @brief Measure distance in metres.
     * @param distance_m Out: one-way distance [m]
     * @return true on success, false on timeout / not initialized
     */
    bool get_distance_m(float *distance_m);

  private:
    uint8_t _trig_pin = 0;
    uint8_t _echo_pin = 0;
    uint32_t _timeout_us = 25000UL;
    bool _initialized = false;

    void trigger_pulse(void);
};

#endif /* HC_SR04_H */
