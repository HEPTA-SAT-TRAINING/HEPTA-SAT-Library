# UvGuvaS12sd &mdash; Grove GUVA-S12SD UV sensor

Source: [drv/uv_guva_s12sd.h](../../drv/uv_guva_s12sd.h) ·
[drv/uv_guva_s12sd.cpp](../../drv/uv_guva_s12sd.cpp)

Driver for the **Seeed Grove UV sensor** based on the **GUVA-S12SD** UV
photodiode. The analog output is read through the board **MCP3208** on the user
payload channel, or directly from an MCU ADC pin (GP26–GP29).

## Hardware

| Item | Value |
|------|-------|
| Sensor | GUVA-S12SD |
| Module | Seeed Grove UV Sensor |
| Interface | Analog via MCP3208 channel 5 / USER1 (CS = GP17 on HEPTA-SAT, V4.1.1) **or** MCU GP28 (ADC2) |
| Supply | 3.3 V payload rail |

Voltage is averaged over **16** raw samples before conversion.

## API

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 5)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3)` | Create a local MCP3208 on `cs_pin`. |
| `bool begin(bool use_mcp3208, uint8_t direct_adc_pin, uint8_t channel, uint8_t cs_pin, float ref_vol = 3.3)` | MCP3208 or direct MCU ADC (GP26–GP29). |
| `uint16_t get_raw()` | Single 12-bit ADC reading. |
| `float get_voltage()` | 16-sample averaged voltage [V]. |
| `float get_illumination_mw_m2()` | `307.0 * voltage` [mW/m²]. |
| `float get_uv_index()` | `illumination / 200`. |

## Example

```cpp
#include "drv/uv_guva_s12sd.h"

UvGuvaS12sd uv;

constexpr bool kUseMcp3208 = true;
constexpr uint8_t kMcp3208CsPin = 17;
constexpr uint8_t kMcp3208Channel = 5;
constexpr uint8_t kDirectAdcPin = 28;

void setup() {
  uv.begin(kUseMcp3208, kDirectAdcPin, kMcp3208Channel, kMcp3208CsPin);
}

void loop() {
  float index = uv.get_uv_index();
}
```
