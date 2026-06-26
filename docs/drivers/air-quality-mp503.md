# AirQualityMp503 &mdash; Grove Air Quality (Winsen MP503)

Source: [drv/air_quality_mp503.h](../../drv/air_quality_mp503.h) ·
[drv/air_quality_mp503.cpp](../../drv/air_quality_mp503.cpp)

Driver for the **Seeed Grove Air Quality Sensor v1.3** (semiconductor gas sensor
**MP503** from Winsen). The analog output is read through the board **MCP3208**
ADC on the user payload channel.

## Hardware

| Item | Value |
|------|-------|
| Sensor | Winsen MP503 |
| Module | Seeed Grove Air Quality Sensor v1.3 |
| Interface | Analog via MCP3208 channel 7 (CS = GP17 on HEPTA-SAT) |
| Warm-up | 20 s in `begin()` |

Classification follows the Seeed `slope()` algorithm. Thresholds from the
original 10-bit Arduino examples are scaled ×4 for 12-bit MCP3208 readings.

## API

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 7)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 7, float ref_vol = 3.3)` | Create a local MCP3208 on `cs_pin`. |
| `QualityLevel slope()` | Returns `FORCE_SIGNAL`, `HIGH_POLLUTION`, `LOW_POLLUTION`, or `FRESH_AIR`. |
| `uint16_t get_raw()` | Latest 12-bit ADC value. |

## Example

```cpp
#include "drv/air_quality_mp503.h"

AirQualityMp503 aq;

void setup() {
  aq.begin(/*cs_pin=*/17, /*channel=*/7);
}

void loop() {
  AirQualityMp503::QualityLevel q = aq.slope();
}
```
