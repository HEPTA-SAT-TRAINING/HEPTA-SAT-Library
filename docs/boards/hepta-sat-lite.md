# HEPTA-SAT Lite (Ver4.2.0)

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

GP0 is **USER** on Ver4.2.0 (there is no camera). `HeptaLiteCdh` does not drive GP0.

### COM (XBee)
| Signal | Pin |
|--------|----:|
| XBee RX | 15 |
| XBee TX | 14 |
| XBee RESET (active low) | 1 |

### EPS
| Signal | Pin |
|--------|----:|
| Bus voltage ADC | 26 |
| Bus current ADC | 28 |
| 3V3 switch (active high) | 20 |
| MCP3208 ADC CS | 17 |

Rail voltages and solar current are read through an external **MCP3208** ADC
(see [drivers/adc-mcp3208.md](../drivers/adc-mcp3208.md)); the bus current
comes straight from the MCU ADC (GP28). Dividers and the MAX4372T (gain 20 /
20 mΩ shunt) match Full V4.1.1.

MCP3208 channel map: CH0 = 5V, CH1 = 3V3, CH2 = SAP voltage, CH3 = solar
current, **CH4 = USER** (no charge-current sense). There is no
`get_current_charge()`.

### Sensor
| Signal | Pin |
|--------|----:|
| BNO055 IMU | I2C Wire (addr 0x28), GP4/GP5 |
| BME280 T/H/P | I2C Wire (addr 0x76, SDO=GND), GP4/GP5 |

Lite has IMU + BME280 only &mdash; **no GPS, no camera**, and no analog
temperature pin (the old GP13 formula is gone).

## API

See the per-subsystem pages:

- [modules/cdh.md](../modules/cdh.md)
- [modules/com.md](../modules/com.md) &mdash; Lite `HeptaLiteCom` drives
  XBEE_RESET on GP1, same as Full.
- [modules/eps.md](../modules/eps.md) &mdash; Lite `HeptaLiteEps` exposes the
  same rail / solar / bus-current API as Full, without charge current.
- [modules/sensor.md](../modules/sensor.md) &mdash; Lite `HeptaLiteSensor`
  adds BME280 temperature, humidity, and pressure on top of the shared IMU.
