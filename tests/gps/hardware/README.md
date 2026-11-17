# GPS hardware validation harness

This is a diagnostic sketch, not a replacement for the Lab examples. The
recorded indoor RP2350 results are in `../HARDWARE_VALIDATION.md`.

## Build and capture

Copy `gps_hardware_validation/` to a temporary sketch directory. Put this
library in that sketch's `src/`, excluding `.git`, docs, tests and examples.
Compile and upload with Arduino CLI for `rp2040:rp2040:rpipico2` (the recorded
test used Arduino-Pico 6.0.0). Upload replaces the current firmware.

Host capture scripts require Python 3 and `pyserial`. They select exactly one
USB serial device with VID 0x2e8a / PID 0x000f, as on the tested Pico 2. Modify
the selection if testing another USB configuration. Close other serial monitors
before running a capture. Outputs are written beside the scripts.

The tested sequence was:

1. Upload an unchanged Lab4-05 with the current library, then run:

   ```sh
   python capture.py --seconds 20 --name raw-lab4-05
   ```

2. Upload the diagnostic sketch, then run these sessions in order:

   ```sh
   python session.py --seconds 20 --command INFO --command RESET_STATS --name initial-harness
   python session.py --seconds 20 --command "PAUSE 8000" --name backlog-8s
   python session.py --seconds 23 --command "PAUSE 15000" --name overrun-15s
   python session.py --seconds 12 --command "RESTART 50" --name restart-50
   python session.py --seconds 10 --command LEGACY --name legacy-api
   python session.py --seconds 180 --command RESET_STATS --name soak-180s
   ```

3. Upload unchanged Lab4-06, then capture:

   ```sh
   python session.py --seconds 12 --name lab4-06-unchanged
   ```

4. Upload `examples/03_gps_latest`, then capture:

   ```sh
   python session.py --seconds 12 --name final-example
   python analyse.py
   ```

The analyser asserts the recorded no-fix scenario. It expects the module's
six-sentence, 1 Hz default stream; adjust pause lengths if the configured output
rate differs. Fifteen seconds exceeded the 2048-byte buffer in the tested
~203-byte/s stream. A positive fix would require different assertions.

## Commands and interpretation

- `INFO` / `SNAP`: software initialization status and cached GPS state.
- `RESET_STATS`: restart timing counters without resetting the GPS cache.
- `PAUSE <ms>`: deliberately stop foreground polling (up to 30 s), then time
  one catch-up call. DMA bookkeeping interrupts remain enabled. Scheduler
  statistics reset afterward: the pause is not successful 100 Hz operation.
- `RESTART <count>`: up to 100 GPS end/begin cycles, checking stopped cache,
  receiver state and free heap. Other initialized software UARTs remain allocated.
- `LEGACY`: invoke GGA/RMC/get_all and check sentinel preservation when no fix.
- `RAW` / `NORMAL`: switch between direct byte output and on-demand parsing;
  restart GPS when switching so consumers do not splice partial sentences.

`STATS` reports 10 ms scheduling, observed call times and free heap. `STATE`
reports independent GGA/RMC sequences, validity and loss/error counters.
Reported times are observations of that workload, not worst-case guarantees.

The harness attempts IMU initialization and initializes the XBee software UART
but never transmits a radio payload/AT command or writes to SD. In the recorded
test **IMU and XBee modules were absent**: `imu=0` was expected; `xbee=1` only
indicates the driver's software initialization return, not module presence or
radio success. The 100 Hz measurement was GPS polling without IMU workload.

For actual IMU/XBee coexistence, power off before fitting modules and repeat
with appropriate workload; the recorded GPS-only run does not establish that.
Outdoor fix accuracy, saturated UART throughput, RP2040 silicon, peripheral
traffic and electrical fault handling remain separate tests.
