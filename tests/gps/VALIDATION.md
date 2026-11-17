# GPS DMA / latest snapshot validation

Date: 2026-11-17 (Asia/Tokyo). Branch: `feat/gps-dma-latest`.
Base: `cc0923a` (`develop`). No board was attached; no upload was performed.

## Implementation and compatibility

- RX-only PIO UART, two chained DMA channels, 2048 effective UART bytes.
- On-demand `gps_get_latest(GpsSnapshot*)`; no receive wait, timer parser or second core.
- All nine pre-existing public driver method signatures retained.
- Original GpggaData/GprmcData field types and order retained.
- Legacy valid-data waits, units, no-fix failures and unchanged outputs tested.
- Legacy parsing now rejects bad checksums/malformed required fields; this is an intentional validation change.
- Original Lab sketch files and library submodule pointers were not edited.

## Host execution

Windows x86-64, Python 3.12, Zig 0.13.0 (`zig c++`, C++17, `-Wall -Wextra -Werror -O1 -g -UNDEBUG`).

- PASS: 151 parser/public-driver checks against fake RX and time.
- PASS: 43,207 DMA transport/integration checks against an SDK register model. Most checks verify byte position/order over repeated laps.
- PASS: PIO instruction model, all 256 byte values back-to-back at three modelled baud ratios, bad-stop and BREAK recovery.
- PASS: existing signatures/struct definitions checked against the base revision.
- PASS: firmware source copies match final production code apart from comments/whitespace.

The DMA model links the production transport, parser and public driver. It checks ring wrap, overwritten data, coalesced interrupts, fixed read endpoints, unrelated IRQ preservation, framing/stall/bus errors, failed allocation and repeated release.

Reproduce host checks with `python tests/gps/run.py` and `python tests/gps/test_pio.py`; see README.md for compiler selection.

## Firmware builds

Arduino CLI 1.0.4, Arduino-Pico 6.0.0. Isolated copies combine unchanged sketches with the new library; each original Lab .ino was compared by SHA-256.

| Case | Board | Result | Flash bytes | Static RAM bytes |
|---|---|---|---:|---:|
| lab4-05 | `rp2040:rp2040:rpipico2` | PASS | 104472 | 12936 |
| lab4-06 | `rp2040:rp2040:rpipico2` | PASS | 107136 | 13080 |
| system-check | `rp2040:rp2040:rpipico2` | PASS | 119776 | 17640 |
| check-kit-full | `rp2040:rp2040:rpipico2` | PASS | 125360 | 17636 |
| check-kit-lite | `rp2040:rp2040:rpipico2` | PASS | 120320 | 17320 |
| gps-latest | `rp2040:rp2040:rpipico2` | PASS | 106856 | 13088 |
| gps-latest-rp2040 | `rp2040:rp2040:rpipico` | PASS | 111244 | 11640 |

The Lite case sets `compiler.cpp.extra_flags=-DHEPTA_BOARD_LITE`. The latest-snapshot example also includes guarded host-test files under src/tests to check that they do not enter embedded builds.

Static RAM figures exclude heap allocation: the aligned RX buffer allocation is 3071 bytes plus the small receiver hardware object. Target parsing latency was not measured.

## Unchanged original sketches

| Sketch | SHA-256 |
|---|---|
| Lab4-05_Read_GPS_Rawdata | `0d44113340dc3ba7d77e580c43cc96efb8029661060480334668a21630d5db01` |
| Lab4-06_Parse_GPGGA_and_GPRMC | `7470b7a1c5e21e1a3ace14c791e2d43471d07025c5f088a36ffc6a1dffa07d58` |
| Lab99_Check_Kit | `5be1657066b03db1a2db4f759a5f343d0eb4f8192f70ab93142fca05e7dd9689` |
| System-Check | `ad7f141b8d12954d648352a6ee47072264e56b9618a1800962c0f8d26b37d092` |

## Not verified without hardware

- Electrical UART reception, DMA/PIO execution on silicon and clock tolerance.
- Long-run reception, real buffer/IRQ timing and recovery under heavy load.
- GPS/XBee resource coexistence through repeated stop/start on the actual board.
- SD/camera/IMU interference and worst-case target parsing time with a full buffer.

The register and instruction models are software checks, not evidence of successful hardware reception. Perform the hardware checks in the driver documentation before treating this as a validated firmware release.
