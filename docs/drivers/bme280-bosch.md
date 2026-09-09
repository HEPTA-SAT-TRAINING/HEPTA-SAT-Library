# Bme280 &mdash; Bosch BME280 environmental sensor

Source: [drv/bme280_bosch.h](../../drv/bme280_bosch.h) ·
[drv/bme280_bosch.cpp](../../drv/bme280_bosch.cpp)

Driver for the **Bosch BME280** combined temperature, humidity, and barometric
pressure sensor over I2C. Compensation uses the Bosch datasheet formulas; no
external sensor library is required.

The driver accepts any `TwoWire` bus. Default remains **Wire1** (Full payload
compatible). Lite onboard BME280 shares **Wire** with the BNO055.

## Hardware

| Item | Value |
|------|-------|
| Interface | I2C (`TwoWire`; default **Wire1**) |
| Address | `0x76` or `0x77` (auto-detected when `addr` is 0) |
| Chip ID | `0x60` |
| HEPTA-SAT Lite onboard | **Wire**, SDA = GP4, SCL = GP5, addr `0x76` (SDO=GND) |
| HEPTA-SAT Full payload | **Wire1**, SDA = GP6, SCL = GP7 |

## API

| Method | Description |
|--------|-------------|
| `bool begin(uint8_t addr = 0, uint8_t sda_pin = 6, uint8_t scl_pin = 7, TwoWire *wire = &Wire1)` | Configure I2C pins, probe the chip, load calibration, and start normal mode. |
| `bool read(float* temp_c, float* hum_pct, float* press_hpa)` | Read compensated temperature [°C], humidity [%], and pressure [hPa]. |

## Example (Full payload, Wire1 GP6/7)

```cpp
#include "drv/bme280_bosch.h"

Bme280 bme;

void setup() {
  bme.begin();  // Wire1 GP6/GP7, auto address
}

void loop() {
  float t, h, p;
  if (bme.read(&t, &h, &p)) {
    // use readings
  }
}
```

## Example (Lite onboard, Wire GP4/5)

```cpp
#include "drv/bme280_bosch.h"
#include <Wire.h>

Bme280 bme;

void setup() {
  bme.begin(0x76, 4, 5, &Wire);
}

void loop() {
  float t, h, p;
  if (bme.read(&t, &h, &p)) {
    // use readings
  }
}
```
