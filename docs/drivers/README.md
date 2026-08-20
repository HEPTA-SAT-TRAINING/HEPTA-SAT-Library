# Drivers

Self-contained drivers for the individual chips and modules used by HEPTA-Sat.
They live in [drv/](../../drv/) and know nothing about the satellite stack; the
base/board classes compose them.

| Driver | Class | Hardware | Bus |
|--------|-------|----------|-----|
| [adc-mcp3208.md](adc-mcp3208.md)   | `AdcMcp3208`  | MCP3208 8-ch 12-bit ADC | SPI |
| [air-quality-mp503.md](air-quality-mp503.md) | `AirQualityMp503` | Grove Air Quality (MP503) | MCP3208 ch5 |
| [bme280-bosch.md](bme280-bosch.md) | `Bme280`      | BME280 temp/humidity/pressure | I2C (0x76/0x77) |
| [camera-arducam-2mp.md](camera-arducam-2mp.md) | `CameraArducam2mp` | Arducam Mini 2MP Plus (OV2640) | SPI + I2C |
| [gps-gp1818mk.md](gps-gp1818mk.md) | `Gps1818mk`   | GP-1818MK GPS | SoftwareSerial (NMEA 0183) |
| [imu-bno055.md](imu-bno055.md)     | `Bno055`      | BNO055 9-axis IMU | I2C (0x28) |
| [light-temt6000.md](light-temt6000.md) | `LightTemt6000` | TEMT6000 light sensor | MCP3208 ch5 |
| [uv-guva-s12sd.md](uv-guva-s12sd.md) | `UvGuvaS12sd` | GUVA-S12SD UV sensor | MCP3208 ch5 |
| [xbee.md](xbee.md)                 | `Xbee`        | Digi XBee radio | SoftwareSerial |
| [unit-roller.md](unit-roller.md)   | `UnitRollerI2C` | M5 Unit Roller485 | I2C |

The hardware drivers are used by HEPTA-Sat subsystems:

- `AdcMcp3208` &rarr; full-board [EPS](../modules/eps.md)
- `CameraArducam2mp`, `Gps1818mk` &rarr; full-board [Sensor](../modules/sensor.md)
- `Bno055` &rarr; [Sensor](../modules/sensor.md) on both boards
- `Xbee` &rarr; [COM](../modules/com.md) on both boards

`UnitRollerI2C` is a vendored third-party driver (see its page).
