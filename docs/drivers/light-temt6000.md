# LightTemt6000 &mdash; SparkFun TEMT6000 ambient light sensor

Source: [drv/light_temt6000.h](../../drv/light_temt6000.h) ·
[drv/light_temt6000.cpp](../../drv/light_temt6000.cpp)

Driver for the **SparkFun TEMT6000** ambient light sensor. The phototransistor
output is read through the board **MCP3208** on the user payload channel, or
directly from an MCU ADC pin (GP26–GP29).

## Hardware

| Item | Value |
|------|-------|
| Sensor | TEMT6000 (visible-light phototransistor) |
| Interface | Analog via MCP3208 channel 5 / USER1 (CS = GP17 on HEPTA-SAT, V4.1.1) **or** MCU GP28 (ADC2) |
| Supply | 3.3 V payload rail |

## API

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 5)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3)` | Create a local MCP3208 on `cs_pin`. |
| `bool begin(bool use_mcp3208, uint8_t direct_adc_pin, uint8_t channel, uint8_t cs_pin, float ref_vol = 3.3)` | MCP3208 or direct MCU ADC (GP26–GP29). |
| `uint16_t get_raw()` | 12-bit ADC reading. |
| `float get_voltage()` | Channel voltage [V]. Averaged via `AdcMcp3208` on MCP3208 path; single `analogRead` on direct ADC path. |
| `float get_lux()` | Approximate lux (`voltage * 200`; not factory-calibrated). |

## Example

```cpp
#include "drv/light_temt6000.h"

LightTemt6000 light;

constexpr bool kUseMcp3208 = true;
constexpr uint8_t kMcp3208CsPin = 17;
constexpr uint8_t kMcp3208Channel = 5;
constexpr uint8_t kDirectAdcPin = 28;

void setup() {
  light.begin(kUseMcp3208, kDirectAdcPin, kMcp3208Channel, kMcp3208CsPin);
}

void loop() {
  float lux = light.get_lux();
}
```
