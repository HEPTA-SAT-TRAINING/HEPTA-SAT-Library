# HEPTA-Sat Library

Arduino library for the HEPTA-SAT and HEPTA-SAT Lite educational satellite kits.

## Getting started

- [Install the library and run your first sketch](docs/getting-started.md)
- Board guides and pin maps: [HEPTA-SAT](docs/boards/hepta-sat.md) · [HEPTA-SAT Lite](docs/boards/hepta-sat-lite.md)

## Subsystems

| Guide | Functions |
|-------|-----------|
| [CDH](docs/modules/cdh.md) | USB serial, SD storage, and command handling |
| [COM](docs/modules/com.md) | XBee radio setup, telemetry, and commands |
| [EPS](docs/modules/eps.md) | Payload power control, voltage, and current measurement |
| [Sensor](docs/modules/sensor.md) | IMU, temperature, and board-specific sensors |

## Examples and reference

- [Lab7 payload connectors and sensor wiring](docs/boards/hepta-sat.md#lab7-payload-connectors)
- XBee examples: [send and receive text](docs/modules/com.md#example) · [switch to Transparent mode](docs/modules/com.md#switching-to-transparent-mode)
- [GPS latest-reading sketch (.ino)](examples/03_gps_latest/03_gps_latest.ino)
- [Hardware drivers](docs/drivers/README.md)
- [Library architecture](docs/architecture.md)

## License

UNISEC. All rights reserved.
