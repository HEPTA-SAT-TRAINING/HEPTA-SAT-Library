# Getting started

## 1. Install the library

Copy (or symlink) this repository into your Arduino `libraries/` folder so that
the entry headers are visible to the IDE:

```
Arduino/
  libraries/
    HEPTA-SAT-Library/
      HeptaSat.h
      HeptaSatLite.h
      ...
```

The library targets an Arduino core that provides `SoftwareSerial`, `SD`, `SPI`,
and `Wire`. It is developed against the RP2040 Arduino core used by the
HEPTA-Sat kit.

## 2. Choose your board

Include **one** entry header, matching your hardware:

| Board | Include | Classes |
|-------|---------|---------|
| HEPTA-SAT (full) | `#include <HeptaSat.h>` | `HeptaCdh`, `HeptaCom`, `HeptaEps`, `HeptaSensor` |
| HEPTA-SAT Lite   | `#include <HeptaSatLite.h>` | `HeptaLiteCdh`, `HeptaLiteCom`, `HeptaLiteEps`, `HeptaLiteSensor` |

Do not include both headers, and do not mix classes from different boards &mdash;
the pin assignments differ.

## 3. First sketch

```cpp
#include <HeptaSat.h>

HeptaCdh    cdh;
HeptaEps    eps;
HeptaSensor sensor;

void setup() {
  cdh.begin();        // Serial + SD card
  eps.init();         // power subsystem
  sensor.begin();     // BNO055 IMU (+ GPS/camera on the full board)

  cdh.println("HEPTA-Sat booted");
}

void loop() {
  float ax, ay, az;
  sensor.get_acceleration(&ax, &ay, &az);
  cdh.printf("acc = %.2f, %.2f, %.2f\n", ax, ay, az);

  float bus = eps.get_bus_voltage();
  cdh.printf("Bus = %.2f V\n", bus);

  delay(1000);
}
```

## Where to go next

- [architecture.md](architecture.md) &mdash; how the layers fit together.
- [boards/](boards/) &mdash; the exact pin map for your board.
- [modules/](modules/) &mdash; the full API for each subsystem.
- [drivers/](drivers/README.md) &mdash; the underlying hardware drivers.
