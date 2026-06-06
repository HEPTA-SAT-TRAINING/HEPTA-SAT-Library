# HEPTA-SAT Lite

Entry header: [HeptaSatLite.h](../../HeptaSatLite.h) &mdash; `#include <HeptaSatLite.h>`

## Classes

| Class | Base | Source |
|-------|------|--------|
| `HeptaLiteCdh`    | `HeptaCdhBase`    | [hepta_sat_lite/hepta_lite_cdh.h](../../hepta_sat_lite/hepta_lite_cdh.h) |
| `HeptaLiteCom`    | `HeptaComBase`    | [hepta_sat_lite/hepta_lite_com.h](../../hepta_sat_lite/hepta_lite_com.h) |
| `HeptaLiteEps`    | `HeptaEpsBase`    | [hepta_sat_lite/hepta_lite_eps.h](../../hepta_sat_lite/hepta_lite_eps.h) |
| `HeptaLiteSensor` | `HeptaSensorBase` | [hepta_sat_lite/hepta_lite_sensor.h](../../hepta_sat_lite/hepta_lite_sensor.h) |

## Pin map

Pins are fixed in each board class's constructor.

### CDH (SD card / SPI)
| Signal | Pin |
|--------|----:|
| SD CS  | 3  |
| SD TX (MOSI) | 19 |
| SD RX (MISO) | 16 |
| SD SCK | 18 |

### COM (XBee)
| Signal | Pin |
|--------|----:|
| XBee RX | 15 |
| XBee TX | 14 |

### EPS
| Signal | Pin |
|--------|----:|
| Battery voltage ADC | 26 |
| Discharge current ADC | 27 |
| Charge current ADC | 28 |

The Lite board reads current straight off the MCU's ADC (no MCP3208).

### Sensor
| Signal | Pin |
|--------|----:|
| Temperature ADC | 13 |
| BNO055 IMU | I2C (addr 0x28) |

The Lite `HeptaSensor` has IMU + temperature only &mdash; no GPS or camera.

## API

See the per-subsystem pages:

- [modules/cdh.md](../modules/cdh.md)
- [modules/com.md](../modules/com.md)
- [modules/eps.md](../modules/eps.md) &mdash; Lite `HeptaLiteEps` adds charge/discharge
  current from the raw ADC.
- [modules/sensor.md](../modules/sensor.md) &mdash; Lite `HeptaLiteSensor` provides
  the shared IMU/temperature API with its own temperature formula.
