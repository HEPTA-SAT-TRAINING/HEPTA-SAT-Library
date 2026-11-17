# Gps1818mk &mdash; GP-1818MK GPS

Source: [drv/gps_gp1818mk.h](../../drv/gps_gp1818mk.h) ·
[drv/gps_gp1818mk.cpp](../../drv/gps_gp1818mk.cpp)

Driver for the **GP-1818MK** GPS module, receiving NMEA 0183 over PIO + DMA
at 9600 baud (8N1) on Arduino-Pico RP2040 / RP2350. Wrapped by the full-board [Sensor](../modules/sensor.md) (the
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
| `void begin(void)` | Start PIO + DMA reception at 9600 baud. Safe to call again after `end()`. |
| `void end(void)` | Stop reception, release DMA channels and the PIO state machine, and reset the cache. |
| `bool get_latest(GpsSnapshot* out)` | Parse received bytes and copy the latest GGA/RMC snapshot without waiting for reception. |
| `int read_byte(void)` | Non-blocking raw NMEA byte (0&ndash;255), or `-1` if none. |
| `bool get_position(float* lat, float* lon, float* alt)` | Position from GPGGA. |
| `bool get_velocity(float* velocity, float* heading)` | Speed + heading from GPRMC. |
| `bool get_all(float* lat, float* lon, float* alt, float* velocity, float* heading)` | GPGGA then GPRMC. |
| `bool get_gpgga(GpggaData* out)` | Full GPGGA into a caller-owned struct. |
| `bool get_gprmc(GprmcData* out)` | Full GPRMC into a caller-owned struct. |
| `bool is_data_available(void)` | True if a byte is waiting in the RX buffer. |

The legacy parsed getters return `false` on no fix / timeout and leave outputs unchanged. Their signatures and data units are unchanged. `get_latest()` has the separate contract below.
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


## On-demand latest snapshot

```cpp
GpsSnapshot snapshot;
if (sensor.gps_get_latest(&snapshot)) {
  if (snapshot.position_valid) {
    // snapshot.gga.lat / lon in decimal degrees; alt in metres above MSL.
  }
}
```

`Gps1818mk::get_latest()` and `HeptaSensor::gps_get_latest()` do the same work:

1. Capture the current end of the DMA receive stream.
2. Parse no more than the 2048 UART bytes already buffered at entry.
3. Retain the last complete, checksum-valid GPGGA and GPRMC independently.
4. Copy the snapshot to caller-owned storage. Partial sentences wait for a later
   call; the function itself never waits for the rest of a sentence or a fix.

There is no public update call, background parser, second-core reservation,
timer, or user callback. DMA completion interrupts perform buffer bookkeeping
only. Parsing takes CPU time proportional to the buffered data; this API is
not a fixed-time or hard-real-time operation.

### Return value and snapshot

- `true`: at least one valid GGA/RMC sentence has been parsed since `begin()`.
  This does **not** mean a valid fix, a new update, or a fresh position.
- `false`: neither sentence has been parsed, the receiver is stopped, or
  `out == nullptr`. A null call consumes nothing. Non-null `out` is always
  written, even on false, so receiver errors are observable before a fix.
- `has_gga` / `has_rmc`: a sentence of that type has been parsed.
- `position_valid`: the most recently parsed GGA reports a fix.
- `velocity_valid`: the most recently parsed RMC is active with valid speed.
- `heading_valid`: that active RMC also contains a valid course; an empty course
  is allowed and its numeric value is zero. Check this flag before using it.
- `gga_sequence` / `rmc_sequence`: increment for every accepted sentence of that
  type, including no-fix/void updates. Unchanged when no new sentence arrives;
  compare for inequality to suppress duplicate log rows. Reset by begin/end.
- `gga_parsed_ms` / `rmc_parsed_ms`: processing times in `millis()` units, not
  byte-arrival times. A buffered old sentence does not become a fresh fix merely
  because it was parsed now. Use the receiver UTC fields and application update
  monitoring; there is no guaranteed receive-age measurement in this API.
- `receiver_state`: `Stopped`, `Ready`, or `ResourceError`. `Ready` means the
  receiver was configured, not that the module is connected or transmitting.
- `rx_overrun_count`: detected data-loss events, not an exact lost-byte count.
- `rx_error_count`: detected PIO stalls or framing/BREAK markers.
- `parse_error_count`: malformed, overlong or bad-checksum target sentences.

Unfixed/void updates replace that type's fields with the newly parsed values
(unavailable numbers are zero) and clear its validity flags. A malformed sentence
does not replace the previous accepted cache. GGA and RMC may have different UTC
values; no automatic same-epoch fusion is performed. Unplugging a module does not
automatically invalidate a previous fix; applications must monitor new updates.

### Buffering, ownership and compatibility

The RX buffer holds 2048 UART bytes (~2.13 s at continuous 9600-baud 8N1).
Poll comfortably faster than this; actual NMEA output is often bursty. On
overrun the partial parser state is discarded and decoding resumes at the next
`$`. If completion interrupts were coalesced across whole DMA laps, the ambiguous
buffer is discarded rather than assigning an invented order to its contents.
An overrun can therefore leave an older cache until another complete sentence
is processed. Check counters and sequence numbers.

All methods for an instance, including begin/end, must run on the core that
started it, with interrupts enabled. They are not reentrant or interrupt-safe.
The library does not make application-level multicore snapshots thread-safe.

Legacy blocking getters use the same parser and update the cache as they consume
sentences, but never succeed from a previous cache alone. Their initial-data wait
(1 s), target-header search (5 s), and target-body gap timeout (200 ms) remain.
They now reject malformed/checksum-invalid sentences rather than publishing
partially decoded fields. `get_all()` still takes GGA then RMC and leaves all
outputs unchanged if either fails. A snapshot after a failed legacy no-fix read
can still report the received no-fix sentence.

Raw reads consume the same stream without parsing. Do not interleave raw and
parsed consumers if you need complete sentences: a consumed raw byte is not
duplicated for the parser. Lab4-05 raw reading and Lab4-06 blocking reading remain
supported without sketch edits; Lab4-07's processed output is unchanged.

`begin()` / `end()` clear the partial sentence and cache. Beginning again retries
resource allocation. The backend uses one RX PIO state machine and two chained
DMA channels. RX-only DMA uses byte reads of the high FIFO byte. TX has no public
send API; its configured pin is held at UART idle HIGH without a TX state machine
(`-1` or `NOPIN` disables it). The shared DMA_IRQ_1 handler only counts completed
halves. Exclusive ownership of that IRQ, unavailable DMA/PIO, invalid pins or
allocation failure or a DMA bus error produce `ResourceError`. It does not replace another IRQ
handler. PIO instructions remain in Arduino-Pico's shared program cache; state
machines and DMA channels are released on end, including failed starts.

See `examples/03_gps_latest/03_gps_latest.ino`. Like the Labs, place the library in
the sketch's `src/` directory. Host verification is in `tests/gps/README.md`.

### Hardware verification still required

Offline tests cannot verify electrical reception, DMA/PIO behaviour on silicon,
clock tolerance, long-run loss, IRQ coexistence, or worst-case parsing latency.
On RP2040 and RP2350 check continuous back-to-back reception, buffer wrap/overrun,
end/begin cycles with XBee active, bad framing, SD/camera traffic, and execution
time with a full buffer before relying on the API in a sampling/control deadline.
