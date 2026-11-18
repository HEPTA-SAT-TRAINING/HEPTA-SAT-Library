# AirQualityMp503 &mdash; Grove Air Quality (Winsen MP503)

Source: [drv/air_quality_mp503.h](../../drv/air_quality_mp503.h) ·
[drv/air_quality_mp503.cpp](../../drv/air_quality_mp503.cpp)

Driver for the **Seeed Grove Air Quality Sensor v1.3** (semiconductor gas sensor
**MP503** from Winsen). The analog output is read through the board **MCP3208**
USER payload channels (V4.1.1: USER1/2/3 = CH5/6/7).

## Hardware

| Item | Value |
|------|-------|
| Sensor | Winsen MP503 |
| Module | Seeed Grove Air Quality Sensor v1.3 |
| Interface | Analog via MCP3208 USER1 / USER2 / USER3 (CS = GP17 on HEPTA-SAT) |
| Warm-up | 20 s in `begin()` unless `skip_warmup = true` |

Classification follows the Seeed `slope()` algorithm. Thresholds from the
original 10-bit Arduino examples are scaled ×4 for 12-bit MCP3208 readings.

## Low-level driver API (`AirQualityMp503`)

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 5, bool skip_warmup = false)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3, bool skip_warmup = false)` | Create a local MCP3208 on `cs_pin`. |
| `QualityLevel slope()` | Returns `FORCE_SIGNAL`, `HIGH_POLLUTION`, `LOW_POLLUTION`, or `FRESH_AIR`. |
| `uint16_t get_raw()` | Latest 12-bit ADC value. |

## HEPTA-SAT example

`aq.begin(USER1)` includes a 20-second warm-up. The Lab7 sketch prints its own
20-second countdown after power-on, then calls `begin_after_warmup(USER1)`.

```cpp
#include "src/HeptaSat.h"

HeptaCdh cdh;
HeptaEps eps;
HeptaAirQualityMp503 aq;

void setup() {
  cdh.begin();
  eps.init();
  eps.switch_3V3_on();
  aq.begin(USER1);
}

void loop() {
  HeptaAirQualityMp503::QualityLevel q = aq.slope();
}
```
