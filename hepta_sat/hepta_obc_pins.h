/**
 * @file hepta_obc_pins.h
 * @brief HEPTA-SAT OBC (V4.1.1) MCU board signals outside CDH/COM/EPS/Sensor.
 *
 * Onboard status LEDs use the same pins as the Lab1 Blinky exercises
 * (GP25 / GP29 / GP24 on the HEPTA-SAT OBC).
 */

#ifndef HEPTA_OBC_PINS_H
#define HEPTA_OBC_PINS_H

#include <Arduino.h>
#include <stdint.h>

/** OBC status LED 1 (GP25). */
constexpr uint8_t HEPTA_OBC_LED1 = 25;
/** OBC status LED 2 (GP29). */
constexpr uint8_t HEPTA_OBC_LED2 = 29;
/** OBC status LED 3 (GP24). */
constexpr uint8_t HEPTA_OBC_LED3 = 24;

constexpr uint8_t HEPTA_OBC_LED_COUNT = 3;

static const uint8_t HEPTA_OBC_LEDS[HEPTA_OBC_LED_COUNT] = {
  HEPTA_OBC_LED1,
  HEPTA_OBC_LED2,
  HEPTA_OBC_LED3,
};

#endif /* HEPTA_OBC_PINS_H */
