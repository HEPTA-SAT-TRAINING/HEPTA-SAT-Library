# Drivers

Self-contained drivers for the individual chips and modules used by HEPTA-Sat.
They live in [drv/](../../drv/) and know nothing about the satellite stack; the
base/board classes compose them.

| Driver | Class | Hardware | Bus |
|--------|-------|----------|-----|
| [adc-mcp3208.md](adc-mcp3208.md)   | `AdcMcp3208`  | MCP3208 8-ch 12-bit ADC | SPI |
| [camera-c1098.md](camera-c1098.md) | `CameraC1098` | C1098 JPEG camera | UART (`Serial1`) |
| [gps-gp1818mk.md](gps-gp1818mk.md) | `Gps1818mk`   | GP-1818MK GPS | SoftwareSerial (NMEA 0183) |
| [imu-bno055.md](imu-bno055.md)     | `Bno055`      | BNO055 9-axis IMU | I2C (0x28) |
| [unit-roller.md](unit-roller.md)   | `UnitRollerI2C` | M5 Unit Roller485 | I2C |

The first four are used by HEPTA-Sat subsystems:

- `AdcMcp3208` &rarr; full-board [EPS](../modules/eps.md)
- `CameraC1098`, `Gps1818mk` &rarr; full-board [Sensor](../modules/sensor.md)
- `Bno055` &rarr; [Sensor](../modules/sensor.md) on both boards

`UnitRollerI2C` is a vendored third-party driver (see its page).
