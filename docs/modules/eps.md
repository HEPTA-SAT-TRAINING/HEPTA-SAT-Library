# EPS &mdash; Electrical Power System

Classes: `HeptaEps` (full) / `HeptaLiteEps` (Lite), both deriving from
`HeptaEpsBase` ([common/hepta_eps_base.h](../../common/hepta_eps_base.h)).

EPS reads the main bus voltage on both boards (schematic net `VBAT_DIVIDER`;
on V4.1.1 hardware this is the spacecraft bus, not the battery cell directly).
The two boards differ in how they measure rail voltages and currents:

- **Full board (V4.1.1)** &mdash; an external **MCP3208** SPI ADC reads the
  5V / 3V3 / SAP rails and the solar/charge current sense; the bus
  current is read directly off the MCU ADC (GP28, MAX4372T output). The 3V3
  rail can be switched.
- **Lite board** &mdash; charge/discharge current is read directly off the MCU's ADC.

## Shared API (`HeptaEpsBase`)

| Method | Description |
|--------|-------------|
| `void init(void)` | Initialize the EPS. |
| `float get_bus_voltage(void)` | Main bus voltage in volts. |
| `uint16_t get_bus_voltage_raw(void)` | Raw ADC reading. |

The bus voltage uses a 1.3k/1.5k divider (gain ~1.867) against a 3.3 V,
12-bit reference. Current sensing uses a MAX4372T (gain 20) over a 0.02 Ω shunt.

## Full board &mdash; `HeptaEps`

MCP3208 channel map (Sensor_Comm V4.1.1): CH0 = 5V, CH1 = 3V3, CH2 = SAP
voltage, CH3 = solar current, CH4 = charge current, CH5&ndash;CH7 = user payload.

| Method | Description |
|--------|-------------|
| `void init(void)` | Initialize EPS + MCP3208 ADC. |
| `void switch_3V3_on(void)` / `switch_3V3_off(void)` | Toggle the 3V3 rail (pin 20). |
| `float get_5v_voltage(void)` | 5V rail voltage (MCP3208 CH0). |
| `float get_3v3_voltage(void)` | 3V3 rail voltage (MCP3208 CH1). |
| `float get_sap_voltage(void)` | Solar-array panel voltage (MCP3208 CH2, divider-corrected). |
| `float get_current_solar(void)` | Solar-array current (MCP3208 CH3). |
| `float get_current_charge(void)` | Charge current (MCP3208 CH4). |
| `float get_current_bus(void)` | Bus current (MCU ADC GP28; shunt on the main bus path). |

> `switch_3V3_off()` then `switch_3V3_on()` power-cycles the 3V3 rail.
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
  float bus = eps.get_bus_voltage();
  float i_sol = eps.get_current_solar();
  float i_chg = eps.get_current_charge();
  // ... log via CDH ...
}
```
