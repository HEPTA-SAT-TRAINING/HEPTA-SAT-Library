# HcSr04 &mdash; HC-SR04 ultrasonic distance sensor

Source: [drv/hc_sr04.h](../../drv/hc_sr04.h) ·
[drv/hc_sr04.cpp](../../drv/hc_sr04.cpp)

Self-contained driver for the **HC-SR04 2020** ultrasonic ranging module
(RCWL-9206). No third-party Arduino libraries are required; the driver uses
`digitalWrite`, `delayMicroseconds`, and `pulseIn` only.

## Hardware

| Item | Value |
|------|-------|
| Sensor | HC-SR04 2020 (board marking “2020”) |
| Interface | Digital TRIG (out) / ECHO (in) |
| Typical HEPTA-SAT wiring | TRIG = USER_IO_1 (GP21), ECHO = USER_IO_2 (GP22) |
| Supply | **3.3 V** on module VCC (rated 3–5.5 V) |
| Echo level | 3.3 V when VCC = 3.3 V — wire Echo to GP22 directly |

## Timing

1. Drive TRIG low for ≥2 µs, then high for 10 µs, then low.
2. Measure Echo high time with `pulseIn` (timeout configurable).
3. Convert: `distance_cm = pulse_us * 0.0343 / 2` (≈343 m/s at 20 °C).

Default timeout is **25000 µs** (about 4 m one-way).
Module documentation recommends ≥200 ms between measurements.

## API

| Method | Description |
|--------|-------------|
| `bool begin(uint8_t trig_pin, uint8_t echo_pin, uint32_t timeout_us = 25000)` | Configure pins and timeout. |
| `bool get_pulse_us(uint32_t* pulse_us)` | Echo high time [µs]; `false` on timeout. |
| `bool get_distance_cm(float* distance_cm)` | One-way distance [cm]. |
| `bool get_distance_m(float* distance_m)` | One-way distance [m]. |

## Example

```cpp
#include "drv/hc_sr04.h"

HcSr04 ultrasonic;

constexpr uint8_t kTrigPin = 21;  // USER_IO_1
constexpr uint8_t kEchoPin = 22;  // USER_IO_2

void setup() {
  ultrasonic.begin(kTrigPin, kEchoPin);
}

void loop() {
  float distance_cm = 0.0f;
  if (ultrasonic.get_distance_cm(&distance_cm)) {
    // use distance_cm
  }
}
```
