# HEPTA-Sat Library

Arduino library for the next-generation **HEPTA-Sat** educational CanSat/CubeSat
kit. It provides the on-board software (CDH / COM / EPS / Sensor) for two boards
that share a common code base:

- **HEPTA-SAT** &mdash; the full board (`HeptaSat.h`)
- **HEPTA-SAT Lite** &mdash; the reduced board (`HeptaSatLite.h`)

A sketch includes exactly one of the two entry headers and uses the matching set
of classes. Everything else (shared base classes, hardware drivers) is pulled in
automatically.

## At a glance

| Subsystem | What it does | HEPTA-SAT | HEPTA-SAT Lite |
|-----------|--------------|:---------:|:--------------:|
| **CDH**    | SD card, serial logging, command handling | `HeptaCdh` | `HeptaLiteCdh` |
| **COM**    | XBee telemetry over SoftwareSerial         | &mdash;¹   | `HeptaLiteCom` |
| **EPS**    | Battery / rail voltages, current sense     | `HeptaEps` | `HeptaLiteEps` |
| **Sensor** | BNO055 IMU, temperature, GPS, camera²      | `HeptaSensor` | `HeptaLiteSensor` |

¹ COM is still in development. It currently ships on the Lite board only
(`HeptaLiteCom`); a full-board `HeptaCom` is planned.
² GPS (GP-1818MK) and camera (C1098) are only wired on the full `HeptaSensor`.

## Quick start

```cpp
#include <HeptaSatLite.h>

HeptaLiteCdh cdh;

void setup() {
  cdh.begin();

  File file = cdh.create_file("test.txt");
  if (file) {
    cdh.write_file(file, "Hello, HEPTA-Sat\n");
    file.close();
  }

  file = cdh.open_file("test.txt");
  while (file && file.available()) {
    Serial.write(cdh.read_file(file));
  }
  file.close();
}

void loop() {}
```

## Documentation

| Document | Contents |
|----------|----------|
| [docs/getting-started.md](docs/getting-started.md) | Install, board selection, first sketch |
| [docs/architecture.md](docs/architecture.md)       | The 3-layer design (entry &rarr; board &rarr; base &rarr; driver) |
| [docs/boards/hepta-sat.md](docs/boards/hepta-sat.md) | Full board: pin map + class list |
| [docs/boards/hepta-sat-lite.md](docs/boards/hepta-sat-lite.md) | Lite board: pin map + class list |
| [docs/modules/](docs/modules/) | Per-subsystem API: [CDH](docs/modules/cdh.md) · [COM](docs/modules/com.md) · [EPS](docs/modules/eps.md) · [Sensor](docs/modules/sensor.md) |
| [docs/drivers/](docs/drivers/README.md) | Hardware drivers: ADC, camera, GPS, IMU, Unit Roller |

## Repository layout

```
HeptaSat.h / HeptaSatLite.h   entry headers (include one in your sketch)
hepta_sat/                    full-board classes (pins fixed here)
hepta_sat_lite/               Lite-board classes (pins fixed here)
common/                       shared base classes (the actual logic)
drv/                          hardware drivers (ADC, camera, GPS, IMU, ...)
docs/                         documentation (this folder set)
```

## License

UNISEC. All rights reserved.
