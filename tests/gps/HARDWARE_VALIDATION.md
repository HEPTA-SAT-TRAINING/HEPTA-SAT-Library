# Indoor RP2350 hardware validation

Date: 2026-11-17 (Asia/Tokyo). Production code tested: `30eaf84` on
`feat/gps-dma-latest` (PR #6). No production source changes were needed.

## Setup and scope

- Connected HEPTA-SAT kit, Raspberry Pi Pico 2 / RP2350, 150 MHz.
- GP-1818MK on RX GP13 / TX GP2, 9600 baud 8N1; USB CDC to the PC.
- Arduino-Pico 6.0.0, Arduino CLI 1.0.4.
- Indoors: all observed GGA fixes were quality 0, all RMC statuses were V.
- The owner confirmed that **IMU and XBee modules were not fitted**. The harness
  reported `imu=0`, as expected. Its `xbee=1` indicates only software UART/PIO
  initialization, not detection of or communication with an XBee module.
- No radio payload/AT command or SD write was issued by the harness.

## Results

| Check | Observation |
|---|---|
| Unchanged Lab4-05 raw reception, 20 s | 4060 bytes, 120 NMEA sentences, 0 checksum errors |
| Message types | 20 each of GPGGA, GPGLL, GPGSA, GPGSV, GPRMC and GPVTG |
| No-fix snapshot handling | GGA/RMC sequences continued updating independently; position/velocity remained invalid; receiver Ready |
| 100 Hz latest-snapshot polling, 180 s | 17,999 calls, 0 missed 10 ms slots, no RX/parse errors or unintentional overrun |
| GPS call time in that 180 s interval | Maximum 148 us, reported mean 11 us |
| Scheduler lateness in that interval | Maximum 13 us, without IMU workload |
| Heap in that interval | 505,000 bytes free throughout |
| 8 s without GPS reads | No overrun; next get_latest took 5,362 us and caught up |
| 15 s without GPS reads | One overrun event detected; next get_latest took 6,710 us; sentence updates resumed with 0 RX/parse errors |
| 50 GPS end/begin cycles | 0 cache/state/resource failures; free heap 505,000 -> 505,000; reception resumed |
| Existing GGA/RMC/get_all methods | Returned false for no fix; all caller sentinel values remained unchanged |
| Unchanged Lab4-06 on the kit, 12 s | 12 GGA and 12 RMC no-fix output lines |
| Standard 03_gps_latest example, 12 s | 12 no-fix output lines; this example was left running on the kit |

The legacy method waits in that check were 895 ms (GGA),
133 ms (RMC), and 866 ms (get_all). These
are observations of the deliberately blocking API, not get_latest timings.

The soak's first/last GGA sequences were 54 / 233; RMC
was 54 / 233, across 180 one-second reports. Repeated
100 Hz reads did not fabricate 100 Hz GPS updates. Intentional pause/legacy
commands reset the scheduler statistics afterward; pauses are **not** counted
as successful uninterrupted 100 Hz operation.

Measured normal-poll maxima in the other short sessions were up to 194 us.
The larger 5.362 / 6.710 ms costs demonstrate the variable cost of draining a
backlog; regular polling keeps the normal case short.

## Reproduction

The exact diagnostic sketch is in
`tests/gps/hardware/gps_hardware_validation/gps_hardware_validation.ino`.
Copy its directory to a temporary sketch location and put this library in
that sketch's `src/`, excluding docs/tests/examples. Build/upload for
`rp2040:rp2040:rpipico2`. Commands and host capture instructions are in
`tests/gps/hardware/README.md`.

The sketch's GPS calls run on core 0. It does not use core 1 or a background
NMEA parser. DMA IRQ bookkeeping continues during the deliberate pauses.

## Limits

These are bounded indoor tests, not an exhaustive hardware acceptance test.
The current stream averaged about 203 bytes/s, not a saturated 9600-baud line.
No conclusion is made about:

- outdoor valid-fix position/velocity/heading or full-fix parsing latency;
- a real IMU sampled at 100 Hz, XBee traffic, SD writes or camera transfers;
- actual RP2040 hardware (that target was compiled only);
- hours/days of operation, electrical fault injection, or delayed/coalesced IRQs;
- a hard worst-case execution-time guarantee.

IMU/XBee installation and a power cycle were not necessary for the GPS-only
checks. Their physical coexistence tests remain separate work.

## Local trace fingerprints

Raw logs are retained locally, not published as telemetry. SHA-256 fingerprints:

| Trace | SHA-256 |
|---|---|
| raw-lab4-05.log | `5fa54c4ba0799fdcb5476a085658a59388e695494ca4b258acd8bd9a791c8bb1` |
| initial-harness.log | `7b7b2d6b1ad67afa84832fea60a5214adca7bbb842ce69e2a4a241e89d29ccc7` |
| backlog-8s.log | `7ca5bb52a8b2261ae33938e063e1abc32aa196b6dc7c5c73a2b4553b4dc16550` |
| overrun-15s.log | `652d984020966ebc2127a01b0deac1eb72a21e115e034b6af267e1fbb6b30c44` |
| restart-50.log | `45fe033200ebc1e4c7176a13f254a0e2e81352f082c10ca13d25fca23df4c762` |
| legacy-api.log | `ce783be0e3beea06264a17aea6f042c16373837fe86db6dd707f70fc97423309` |
| soak-180s.log | `aa3c66bd3727e86cddd7e3d00a6ed327ad86d39f4eaeb00385c93ac4ed9b448e` |
| lab4-06-unchanged.log | `f641c4cc901471df557cd5282151c4e628926e4977708424074f70e9962a4869` |
| final-example.log | `e6d45665eb93ca9520b3024204053c01423b11539636b01eec157e9d1cd86e09` |
