# LightTemt6000 &mdash; SparkFun TEMT6000 ambient light sensor

Source: [drv/light_temt6000.h](../../drv/light_temt6000.h) ·
[drv/light_temt6000.cpp](../../drv/light_temt6000.cpp)

Driver for the **SparkFun TEMT6000** ambient light sensor. The phototransistor
output is read through the board **MCP3208** USER payload channels
(V4.1.1: USER1/2/3 = CH5/6/7).

## Hardware

| Item | Value |
|------|-------|
| Sensor | TEMT6000 (visible-light phototransistor) |
| Interface | Analog via MCP3208 USER1 / USER2 / USER3 (CS = GP17 on HEPTA-SAT) |
| Supply | 3.3 V payload rail |

## API

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 5)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3)` | Create a local MCP3208 on `cs_pin`. |
| `uint16_t get_raw()` | 12-bit ADC reading. |
| `float get_voltage()` | Channel voltage [V] (averaged via `AdcMcp3208`). |
| `float get_lux()` | Approximate lux (`voltage * 200`; not factory-calibrated). |

## Example

```cpp
#include "drv/light_temt6000.h"

LightTemt6000 light;

constexpr uint8_t kUserChannel = 1;  // 1=USER1, 2=USER2, 3=USER3
constexpr uint8_t kMcp3208CsPin = 17;

void setup() {
  light.begin(kMcp3208CsPin, static_cast<uint8_t>(4 + kUserChannel));
}

void loop() {
  float lux = light.get_lux();
}
```
