# UvGuvaS12sd &mdash; Grove GUVA-S12SD UV sensor

Source: [drv/uv_guva_s12sd.h](../../drv/uv_guva_s12sd.h) ·
[drv/uv_guva_s12sd.cpp](../../drv/uv_guva_s12sd.cpp)

Driver for the **Seeed Grove UV sensor** based on the **GUVA-S12SD** UV
photodiode. The analog output is read through the board **MCP3208** USER
payload channels (V4.1.1: USER1/2/3 = CH5/6/7).

## Hardware

| Item | Value |
|------|-------|
| Sensor | GUVA-S12SD |
| Module | Seeed Grove UV Sensor |
| Interface | Analog via MCP3208 USER1 / USER2 / USER3 (CS = GP17 on HEPTA-SAT) |
| Supply | 3.3 V payload rail |

Voltage is averaged over **16** raw samples before conversion.

## API

| Method | Description |
|--------|-------------|
| `bool begin(AdcMcp3208* adc, uint8_t channel = 5)` | Use an existing MCP3208 instance. |
| `bool begin(uint8_t cs_pin, uint8_t channel = 5, float ref_vol = 3.3)` | Create a local MCP3208 on `cs_pin`. |
| `uint16_t get_raw()` | Single 12-bit ADC reading. |
| `float get_voltage()` | 16-sample averaged voltage [V]. |
| `float get_illumination_mw_m2()` | `307.0 * voltage` [mW/m²]. |
| `float get_uv_index()` | `illumination / 200`. |

## Example

```cpp
#include "drv/uv_guva_s12sd.h"

UvGuvaS12sd uv;

constexpr uint8_t kUserChannel = 1;  // 1=USER1, 2=USER2, 3=USER3
constexpr uint8_t kMcp3208CsPin = 17;

void setup() {
  uv.begin(kMcp3208CsPin, static_cast<uint8_t>(4 + kUserChannel));
}

void loop() {
  float index = uv.get_uv_index();
}
```
