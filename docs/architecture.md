# Architecture

The library is organized in three layers so that the two boards
(HEPTA-SAT and HEPTA-SAT Lite) can share one implementation while keeping their
board-specific pin assignments and conversion formulas separate.

```
  entry header        HeptaSat.h                 HeptaSatLite.h
                          │                            │
                          ▼                            ▼
  board layer        hepta_sat/                  hepta_sat_lite/
  (pins fixed)       HeptaCdh                    HeptaLiteCdh
                     HeptaCom                    HeptaLiteCom
                     HeptaEps                    HeptaLiteEps
                     HeptaSensor                 HeptaLiteSensor
                          │                            │
                          └────────────┬───────────────┘
                                       ▼
  base layer                       common/
  (shared logic)   HeptaCdhBase · HeptaComBase · HeptaEpsBase · HeptaSensorBase
                                       │
                                       ▼
  driver layer                       drv/
             AdcMcp3208 · CameraC1098 · Gps1818mk · Bno055 · Xbee · UnitRoller
```

## Layer responsibilities

### Entry header
[HeptaSat.h](../HeptaSat.h) and [HeptaSatLite.h](../HeptaSatLite.h) are the only
headers a sketch includes. Each one pulls in exactly the classes that exist for
that board &mdash; nothing else. Including both, or mixing classes across boards,
is not supported.

### Board layer (`hepta_sat/`, `hepta_sat_lite/`)
Thin classes whose only job is to **fix the hardware configuration** of the
board: SD/SPI pins, ADC pins, the temperature pin, XBee RX/TX, etc. They pass
those values to the base-class constructor and, where the boards differ, add a
board-specific method (e.g. each board's own `get_temperature()` conversion).

### Base layer (`common/`)
Where the real work lives. `HeptaCdhBase`, `HeptaComBase`, `HeptaEpsBase`, and
`HeptaSensorBase` implement the subsystem logic against *parameters* (pins,
gains) supplied by the board class. Sketches never include these headers
directly &mdash; the protected constructors enforce that they are only reachable
through a board subclass.

### Driver layer (`drv/`)
Self-contained drivers for individual chips/modules. They know nothing about
HEPTA-SAT; they just talk to hardware (SPI/I2C/UART). Base and board classes
compose them (e.g. `HeptaSensor` owns a `CameraC1098` and a `Gps1818mk`, while
`HeptaComBase` owns an `Xbee`).

## Why this shape

- **One implementation, two boards.** Bug fixes in `common/` benefit both boards.
- **Pins live in one obvious place.** To re-wire a board, edit its constructor in
  the board layer; no logic changes.
- **Drivers are reusable and testable in isolation** from the satellite stack.

## COM

Both boards expose the shared education-facing API in `HeptaComBase`.
`HeptaCom` and `HeptaLiteCom` fix the board wiring to SoftwareSerial RX pin 15
and TX pin 14. `HeptaComBase` delegates UART communication, XBee command mode,
and AT commands to the `Xbee` driver in `drv/`.
