# Gps1818mk &mdash; GP-1818MK GPS

Source: [drv/gps_gp1818mk.h](../../drv/gps_gp1818mk.h) ·
[drv/gps_gp1818mk.cpp](../../drv/gps_gp1818mk.cpp)

Driver for the **GP-1818MK** GPS module, reading NMEA 0183 over `SoftwareSerial`
at 9600 baud. Wrapped by the full-board [Sensor](../modules/sensor.md) (the
`gps_*` methods).

## Data structs

### `GpggaData` (position + fix quality)
| Field | Type | Meaning |
|-------|------|---------|
| `utc_time` | `float` | UTC time of fix, `hhmmss.sss` |
| `lat` | `float` | Latitude, decimal degrees (negative = South) |
| `lon` | `float` | Longitude, decimal degrees (negative = West) |
| `alt` | `float` | Altitude above mean sea level, meters |
| `fix_quality` | `uint8_t` | 0 = no fix, 1 = GPS, 2 = DGPS |
| `sat_num` | `uint8_t` | Satellites in use |
| `hdop` | `float` | Horizontal dilution of precision |

### `GprmcData` (velocity + heading)
| Field | Type | Meaning |
|-------|------|---------|
| `utc_time` | `float` | UTC time of fix, `hhmmss.sss` |
| `velocity` | `float` | Ground speed, m/s |
| `heading` | `float` | True course, degrees (0&ndash;360) |
| `date[7]` | `char` | UTC date `ddmmyy`, null-terminated |

## API

| Method | Description |
|--------|-------------|
| `Gps1818mk(pin_size_t rx_pin = 13, pin_size_t tx_pin = 2)` | Construct; `rx_pin` is the GPS TX line, `tx_pin` the GPS RX line (GPS_TX net, V4.1.1). Pass `-1` to disable TX. |
| `void begin(void)` | Open SoftwareSerial at 9600 baud. Call once in `setup()`. |
| `int read_byte(void)` | Non-blocking raw NMEA byte (0&ndash;255), or `-1` if none. |
| `bool get_position(float* lat, float* lon, float* alt)` | Position from GPGGA. |
| `bool get_velocity(float* velocity, float* heading)` | Speed + heading from GPRMC. |
| `bool get_all(float* lat, float* lon, float* alt, float* velocity, float* heading)` | GPGGA then GPRMC. |
| `bool get_gpgga(GpggaData* out)` | Full GPGGA into a caller-owned struct. |
| `bool get_gprmc(GprmcData* out)` | Full GPRMC into a caller-owned struct. |
| `bool is_data_available(void)` | True if a byte is waiting in the RX buffer. |

All read methods return `false` on no fix / timeout and leave outputs unchanged.
The driver returns parsed values / caller-owned structs &mdash; no internal buffer
is exposed.

## Examples

### Parsed position
```cpp
#include "drv/gps_gp1818mk.h"
Gps1818mk gps(/*rx_pin=*/13);

void setup() { gps.begin(); }

void loop() {
  float lat, lon, alt;
  if (gps.get_position(&lat, &lon, &alt)) {
    // valid fix
  }
}
```

### Raw NMEA passthrough
```cpp
void loop() {
  while (gps.is_data_available()) {
    int b = gps.read_byte();
    if (b >= 0) Serial.write((uint8_t)b);
  }
}
```

## Non-blocking async API

For high-rate telemetry loops, use the async parser in
[common/gps_async.h](../../common/gps_async.h) instead of the blocking
`get_gpgga()` / `get_gprmc()` methods.

| Function | Description |
|----------|-------------|
| `void gps_async_init(void)` | Initialize the mutex. Call once before multicore GPS service starts. |
| `void gps_async_begin(Gps1818mk* gps)` | Bind a driver instance (called from `HeptaSensor::begin()`). |
| `void gps_service(void)` | Drain available NMEA bytes and update the cached `GpsFix`. Non-blocking. |
| `bool gps_get_latest(GpsFix* out)` | Copy the latest cached fix. Safe from any core. Returns `true` if `has_fix`. |

`HeptaSensor` exposes thin wrappers: `gps_service()` and `gps_get_latest()`.
Run `gps_service()` on core 1 (`loop1()`) and read the cache from core 0.

See [common/gps_fix.h](../../common/gps_fix.h) for the `GpsFix` struct.
GPGGA updates set `fix_millis`; compare this value to detect new GPS rows
without blocking the main loop.
