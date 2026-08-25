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
| 3V3 switch (active high) | 20 |
| MCP3208 ADC CS | 17 |

Rail voltages and the solar/charge currents are read through an external
**MCP3208** ADC (see [drivers/adc-mcp3208.md](../drivers/adc-mcp3208.md));
the bus current comes straight from the MCU ADC (GP28).

### OBC (MCU board)
| Signal | Pin |
|--------|----:|
| Status LED 1 (`HEPTA_OBC_LED1`) | 25 |
| Status LED 2 (`HEPTA_OBC_LED2`) | 29 |
| Status LED 3 (`HEPTA_OBC_LED3`) | 24 |

Include [hepta_obc_pins.h](../../hepta_sat/hepta_obc_pins.h) via `HeptaSat.h` for the
`HEPTA_OBC_LED*` constants and the `HEPTA_OBC_LEDS[]` array.

### Sensor
| Signal | Pin |
|--------|----:|
| Temperature ADC | 27 |
| BNO055 IMU | I2C (addr 0x28) |
| GPS GP-1818MK | SoftwareSerial RX 13 / TX 2 |
| Camera (Arducam Mini 2MP Plus) | CS = GP0 (shared SPI); OV2640 on I2C |

> **V4.1.1:** GP0 is `CAMERA_CS` for the Arducam Mini 2MP Plus; GP1 is
> `XBEE_RESET`. `HeptaCdh::begin()` drives CAMERA_CS high so the camera stays
> off the shared SPI bus until `CameraArducam2mp` selects it. JPEG capture uses
> SPI (ArduChip FIFO) and I2C (OV2640), sharing the bus with the SD card /
> MCP3208 and the BNO055 respectively.

## API

See the per-subsystem pages:

- [modules/cdh.md](../modules/cdh.md)
- [modules/com.md](../modules/com.md)
- [modules/eps.md](../modules/eps.md) &mdash; full-board `HeptaEps` adds 5V/3V3/SAP
  rail voltages, solar/charge current via the MCP3208, and bus current
  on the MCU ADC.
- [modules/sensor.md](../modules/sensor.md) &mdash; full-board `HeptaSensor` adds
  GPS and camera on top of the shared IMU/temperature API.
