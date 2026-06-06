# EPS &mdash; Electrical Power System

Classes: `HeptaEps` (full) / `HeptaLiteEps` (Lite), both deriving from
`HeptaEpsBase` ([common/hepta_eps_base.h](../../common/hepta_eps_base.h)).

EPS reads the battery voltage on both boards. The two boards differ in how they
measure rail voltages and currents:

- **Full board** &mdash; an external **MCP3208** SPI ADC reads the 5V / 3V3 / SAP
  rails and the charge/discharge current sense, and can switch the 3V3 rail.
- **Lite board** &mdash; charge/discharge current is read directly off the MCU's ADC.

## Shared API (`HeptaEpsBase`)

| Method | Description |
|--------|-------------|
| `void init(void)` | Initialize the EPS. |
| `float get_battery_voltage(void)` | Battery voltage in volts. |
| `uint16_t get_battery_voltage_raw(void)` | Raw ADC reading. |

The battery voltage uses a 12k/30k divider (gain ×1.4) against a 3.3 V, 12-bit
reference. Current sensing uses a galvano gain of 50 over a 0.02 Ω shunt.

## Full board &mdash; `HeptaEps`

| Method | Description |
|--------|-------------|
| `void init(void)` | Initialize EPS + MCP3208 ADC. |
| `void switch_3V3_on(void)` / `switch_3V3_off(void)` | Toggle the 3V3 rail (pin 20). |
| `float get_5v_voltage(void)` | 5V rail voltage. |
| `float get_3v3_voltage(void)` | 3V3 rail voltage. |
| `float get_sap_voltage(void)` | Solar-array panel voltage. |
| `float get_current_discharge(void)` | Discharge current. |
| `float get_current_charge(void)` | Charge current. |

> `switch_3V3_off()` then `switch_3V3_on()` power-cycles the 3V3 rail &mdash; the
> documented way to recover the camera (then call `HeptaSensor::camera_invalidate()`).
> See [drivers/adc-mcp3208.md](../drivers/adc-mcp3208.md).

## Lite board &mdash; `HeptaLiteEps`

| Method | Description |
|--------|-------------|
| `float get_current_discharge(void)` | Discharge current (ADC pin 27). |
| `float get_current_charge(void)` | Charge current (ADC pin 28). |

## Example (full board)

```cpp
#include <HeptaSat.h>
HeptaEps eps;

void setup() {
  eps.init();
  eps.switch_3V3_on();
}

void loop() {
  float vbat = eps.get_battery_voltage();
  float i_chg = eps.get_current_charge();
  // ... log via CDH ...
}
```
