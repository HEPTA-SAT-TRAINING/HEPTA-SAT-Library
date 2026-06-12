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
| **COM**    | XBee telemetry over SoftwareSerial         | `HeptaCom` | `HeptaLiteCom` |
| **EPS**    | Battery / rail voltages, current sense     | `HeptaEps` | `HeptaLiteEps` |
| **Sensor** | BNO055 IMU, temperature, GPS, camera¹      | `HeptaSensor` | `HeptaLiteSensor` |

¹ GPS (GP-1818MK) and camera (C1098) are only wired on the full `HeptaSensor`.

## XBee communication

HeptaCom v0.1 is a high-level wrapper for an XBee operating in AT /
Transparent mode. Both `HeptaCom` and `HeptaLiteCom` use SoftwareSerial with
RX pin 15 and TX pin 14.

Before using the library, configure the XBee with settings appropriate for the
kit:

- `AP=0`
- a matching PAN ID
- `DH` / `DL` for the peer XBee
- `BD=5` (38400 baud)

The library always opens the XBee UART at 38400 baud. The baud rate is not
specified by sketches.

`send()` transmits exactly the bytes supplied. It does not append a newline:

```cpp
#include <HeptaSat.h>

HeptaCom com;

void setup() {
  Serial.begin(115200);
  com.begin();
  com.send("hello\n");
}
```

`set_transparent_mode()` and `set_api_mode()` enter XBee command mode and
change `AP` from the microcontroller. Entering command mode requires guard
times before and after `+++`, so a mode change takes several seconds.

Both functions default to `save=false` and therefore do not send `ATWR`:

```cpp
com.set_transparent_mode();      // Temporary AP=0
com.set_transparent_mode(true);  // AP=0 and write to nonvolatile memory
```

Specify `save=true` only when the setting must survive a power cycle. `ATWR`
writes nonvolatile memory and should not be called frequently. The standard
state for a reused kit is AT / Transparent mode.

`set_api_mode()` only changes the XBee to `AP=1`. HeptaCom v0.1 does not
implement API frame transmission or reception. After switching to API mode,
calling `send("hello\n")` does not work as Transparent-mode communication.

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

## Examples

| Example | Contents |
|---------|----------|
| [01_basic_transparent](examples/01_basic_transparent/01_basic_transparent.ino) | Send and receive text in Transparent mode |
| [02_switch_transparent_mode](examples/02_switch_transparent_mode/02_switch_transparent_mode.ino) | Temporarily restore `AP=0` without `ATWR` |

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
