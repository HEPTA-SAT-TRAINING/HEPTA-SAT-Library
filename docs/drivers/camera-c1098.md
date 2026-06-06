# CameraC1098 &mdash; C1098 JPEG camera

Source: [drv/camera_c1098.h](../../drv/camera_c1098.h) ·
[drv/camera_c1098.cpp](../../drv/camera_c1098.cpp)

Driver for the **C1098** UART JPEG camera. It talks over the hardware `Serial1`
port (`CAM_SERIAL`), syncs at 14400 baud, then captures a JPEG and streams it in
fixed 512-byte packets. Wrapped by the full-board [Sensor](../modules/sensor.md)
(`camera_snapshot()` / `camera_invalidate()`).

## Configuration enums

| Enum | Values |
|------|--------|
| `C1098_BAUD_RATE` | 14400 / 28800 / 57600 / 115200 / 230400 / 460800 |
| `C1098_JPEG_SIZE` | `QVGA` (320×240), `VGA` (640×480) |

## API

| Method | Description |
|--------|-------------|
| `bool begin(C1098_BAUD_RATE baud_rate, C1098_JPEG_SIZE size)` | SYNC + INITIAL handshake; sets baud and JPEG size. Returns success. |
| `uint32_t take_picture(void)` | Trigger a snapshot; returns the JPEG byte length. |
| `uint16_t get_packet_size(void)` | Fixed transfer packet size (512 bytes). |
| `int get_image_data_packet(uint8_t* buf, size_t max_size)` | Fetch one packet into `buf`. Returns bytes written, `0` when complete, or `-1` on error. |
| `void invalidate(void)` | Drop cached setup so the next `begin()` does a full re-sync at 14400 baud. |

## Robustness notes

- The driver hunts for the `0xAA` start byte when reading command frames, so a
  stray/noise byte cannot permanently desync the 6-byte framing.
- Packets are checksummed with retry; the JPEG is validated before it is accepted.
- After a camera power-cycle, call `invalidate()` (or `HeptaSensor::camera_invalidate()`)
  so the driver re-syncs instead of assuming the camera is still initialized. The
  camera VCC is not on an MCU-controllable rail, so recovery is operator-driven.

## Reading a full image

```cpp
#include "drv/camera_c1098.h"
CameraC1098 cam;

void capture() {
  if (!cam.begin(C1098_BAUD_RATE_115200, C1098_JPEG_SIZE_VGA)) return;

  uint32_t len = cam.take_picture();
  uint8_t buf[512];
  int n;
  while ((n = cam.get_image_data_packet(buf, sizeof(buf))) > 0) {
    // write n bytes of buf to the SD card
  }
  // n == 0: done, n < 0: error
}
```
