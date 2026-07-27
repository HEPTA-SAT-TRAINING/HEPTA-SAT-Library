# HEPTA-SAT (full board, V4.1.1)

Entry header: [HeptaSat.h](../../HeptaSat.h) &mdash; `#include <HeptaSat.h>`

## Classes

| Class | Base | Source |
|-------|------|--------|
| `HeptaCdh`    | `HeptaCdhBase`    | [hepta_sat/hepta_cdh.h](../../hepta_sat/hepta_cdh.h) |
| `HeptaCom`    | `HeptaComBase`    | [hepta_sat/hepta_com.h](../../hepta_sat/hepta_com.h) |
| `HeptaEps`    | `HeptaEpsBase`    | [hepta_sat/hepta_eps.h](../../hepta_sat/hepta_eps.h) |
| `HeptaSensor` | `HeptaSensorBase` | [hepta_sat/hepta_sensor.h](../../hepta_sat/hepta_sensor.h) |

## Pin map

Pins are fixed in each board class's constructor.

### COM (XBee)
| Signal | Pin |
|--------|----:|
| XBee RX | 15 |
| XBee TX | 14 |
| XBee RESET (active low) | 1 |

### CDH (SD card / SPI)
| Signal | Pin |
|--------|----:|
| SD CS  | 3  |
| SD TX (MOSI) | 19 |
| SD RX (MISO) | 16 |
| SD SCK | 18 |
| Camera CS (held inactive at `begin`) | 0 |

### EPS
| Signal | Pin |
|--------|----:|
| Bus voltage ADC | 26 |
| Bus current ADC | 28 |
| 3V3 switch | 20 |
| MCP3208 ADC CS | 17 |

Rail voltages and the solar/charge currents are read through an external
**MCP3208** ADC (see [drivers/adc-mcp3208.md](../drivers/adc-mcp3208.md));
the bus current comes straight from the MCU ADC (GP28).

### Sensor
| Signal | Pin |
|--------|----:|
| Temperature ADC | 27 |
| BNO055 IMU | I2C (addr 0x28) |
| GPS GP-1818MK | SoftwareSerial RX 13 / TX 2 |
| Camera C1098 | `Serial1` (**not wired on V4.1.1**; SPI camera support is a separate phase) |

> **V4.1.1 note:** the UART camera (C1098) nets were removed from the board;
> GP0 is now CAMERA_CS for a future SPI camera and GP1 is XBEE_RESET.
> `HeptaCdh::begin()` drives CAMERA_CS high so a connected Arducam stays off the
> shared SPI bus until a camera driver is added. The C1098 driver is kept for
> older boards only &mdash; camera sketches do not work on V4.1.1 hardware.

## API

See the per-subsystem pages:

- [modules/cdh.md](../modules/cdh.md)
- [modules/com.md](../modules/com.md)
- [modules/eps.md](../modules/eps.md) &mdash; full-board `HeptaEps` adds 5V/3V3/SAP
  rail voltages, solar/charge current via the MCP3208, and bus current
  on the MCU ADC.
- [modules/sensor.md](../modules/sensor.md) &mdash; full-board `HeptaSensor` adds
  GPS and camera on top of the shared IMU/temperature API.
