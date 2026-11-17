# GPS offline checks

Run from any directory with Python 3 and a C++17 host compiler:

```sh
python tests/gps/run.py
python tests/gps/test_pio.py
```

`CXX` selects a compiler command (default `c++`). Alternatively set `ZIG` to the
path of a Zig executable; the runner invokes `zig c++`. No third-party test
framework is needed. Outputs go to a temporary directory. Host-only `.cpp`
files are guarded by `HEPTA_GPS_HOST_TEST` / `HEPTA_GPS_DMA_TEST` so they do not define fake drivers or
`main()` when this repository is included under a Lab sketch's `src/` directory.

The production `Gps1818mk` and `GpsNmeaParser` are built with a fake transport and
clock. Checks cover all sentence split positions, units/hemispheres, no-fix and
void data, optional course, checksum and field rejection, malformed/overlong
input, resynchronisation, independent receivers, fixed get_latest endpoint,
cache/sequences, simulated overrun/error, restart/resource failure, raw byte
order, blocking waits/timeouts and preservation of outputs on legacy failure.

A second binary links the production DMA transport against an SDK register
model, checking chained half-buffers across many laps, byte order, frozen read
limits, overrun/coalesced interrupts, framing/stall/bus error reporting,
unrelated IRQ preservation, allocation failures without touching unconfigured
GPIOs, and repeated resource release. Coordinate tests include negative values
that underflow the float representation.
This model does not prove the SDK model's timing or hardware behaviour.

`test_pio.py` executes the actual opcodes from the generated PIO header against
modelled UART bits. It checks all byte values back-to-back, three baud ratios,
high-byte placement, stop-bit rejection and BREAK recovery. It assumes immediate
FIFO drainage: it is not a DMA, electrical, IRQ-latency or silicon test.

To regenerate the committed header using the Arduino-Pico bundled assembler:

```sh
pioasm -o c-sdk drv/gps_uart_rx.pio drv/gps_uart_rx.pio.h
```

## Sketch compatibility

Compile copies of the unchanged Lab4-05, Lab4-06, System-Check and Lab99 sketches,
replacing only each copy's `src/` with this library. Compile the new
`examples/03_gps_latest` similarly. Use `rp2040:rp2040:rpipico2` and compile Lab99
again with `compiler.cpp.extra_flags=-DHEPTA_BOARD_LITE`. Also compile a GPS
sketch for `rp2040:rp2040:rpipico` to cover RP2040 SDK differences. Keep original
Lab files and submodule pointers unchanged.

These checks do not measure target parsing time or establish DMA reception on
hardware. Required board checks are listed in the GPS driver documentation.
