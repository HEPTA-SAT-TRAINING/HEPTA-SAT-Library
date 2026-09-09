# AdcMcp3208 &mdash; MCP3208 SPI ADC

Source: [drv/adc_mcp3208.h](../../drv/adc_mcp3208.h) ·
[drv/adc_mcp3208.cpp](../../drv/adc_mcp3208.cpp)

Driver for the **MCP3208**, an 8-channel 12-bit SPI ADC. Used by
[EPS](../modules/eps.md) on both boards to read rail voltages and current
sense (Lite has no charge-current channel).

## API

| Method | Description |
|--------|-------------|
| `void begin(uint8_t cs_pin, float ref_vol = 3.3)` | Initialize the ADC on the given SPI chip-select pin, with the given reference voltage. |
| `uint16_t get_raw_data(uint8_t channel)` | Raw 12-bit reading from channel 0&ndash;7. |
| `float get_voltage(uint8_t channel)` | Voltage on channel 0&ndash;7 (scaled by `ref_vol`). |

Reads are averaged internally for noise reduction.

## Example

```cpp
#include "drv/adc_mcp3208.h"
AdcMcp3208 adc;

void setup() {
  adc.begin(/*cs_pin=*/17, /*ref_vol=*/3.3);
}

void loop() {
  float v = adc.get_voltage(0);
}
```
