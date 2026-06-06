# HEPTA-SAT (full board)

Entry header: [HeptaSat.h](../../HeptaSat.h) &mdash; `#include <HeptaSat.h>`

## Classes

| Class | Base | Source |
|-------|------|--------|
| `HeptaCdh`    | `HeptaCdhBase`    | [hepta_sat/hepta_cdh.h](../../hepta_sat/hepta_cdh.h) |
| `HeptaEps`    | `HeptaEpsBase`    | [hepta_sat/hepta_eps.h](../../hepta_sat/hepta_eps.h) |
| `HeptaSensor` | `HeptaSensorBase` | [hepta_sat/hepta_sensor.h](../../hepta_sat/hepta_sensor.h) |

> COM is **in development**: the full board has no `HeptaCom` class yet (COM
> currently ships on the Lite board only). A full-board `HeptaCom` is planned.
> See [architecture.md](../architecture.md#com-status-in-development).

## Pin map

Pins are fixed in each board class's constructor.

### CDH (SD card / SPI)
| Signal | Pin |
|--------|----:|
| SD CS  | 3  |
| SD TX (MOSI) | 19 |
| SD RX (MISO) | 16 |
| SD SCK | 18 |

### EPS
| Signal | Pin |
|--------|----:|
| Battery voltage ADC | 26 |
| 3V3 switch | 20 |
| MCP3208 ADC CS | 17 |

Rail voltages and currents are read through an external **MCP3208** ADC
(see [drivers/adc-mcp3208.md](../drivers/adc-mcp3208.md)).

### Sensor
| Signal | Pin |
|--------|----:|
| Temperature ADC | 27 |
| BNO055 IMU | I2C (addr 0x28) |
| GPS GP-1818MK | SoftwareSerial RX 13 |
| Camera C1098 | `Serial1` |

## API

See the per-subsystem pages:

- [modules/cdh.md](../modules/cdh.md)
- [modules/eps.md](../modules/eps.md) &mdash; full-board `HeptaEps` adds 5V/3V3/SAP
  rail voltages and charge/discharge current via the MCP3208.
- [modules/sensor.md](../modules/sensor.md) &mdash; full-board `HeptaSensor` adds
  GPS and camera on top of the shared IMU/temperature API.
