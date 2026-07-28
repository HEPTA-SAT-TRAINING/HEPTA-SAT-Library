# CameraArducam2mp &mdash; Arducam Mini 2MP Plus

Source: [drv/camera_arducam_2mp.h](../../drv/camera_arducam_2mp.h) ·
[drv/camera_arducam_2mp.cpp](../../drv/camera_arducam_2mp.cpp) ·
[drv/ov2640_regs.h](../../drv/ov2640_regs.h)

Minimal driver for the **Arducam Mini 2MP Plus** (OV2640 sensor + ArduChip FIFO).
It uses Arduino `SPI` (commands / JPEG burst) and `Wire` (sensor registers) only
— no external ArduCAM library. Sketches normally go through
`HeptaSensor::camera_snapshot()` / `camera_invalidate()`.

## Hardware

| Item | Value |
|------|-------|
| Camera CS | GP0 (shared SPI with SD + MCP3208) |
| SPI | Mode 0, ≤ 8 MHz (driver uses 4 MHz) |
| OV2640 I2C | 7-bit address `0x30` (same `Wire` bus as BNO055) |
| JPEG sizes | QQVGA 160×120, QVGA 320×240, VGA 640×480 (default), SVGA 800×600, UXGA 1600×1200 |

`HeptaCdh::begin()` drives CAMERA_CS high before any SPI traffic so the camera
stays deselected until this driver claims the bus.

## API

| Method | Description |
|--------|-------------|
| `CameraArducam2mp(uint8_t cs_pin = 0)` | Construct with chip-select pin. |
| `bool begin(ArducamJpegSize jpeg_size = ARDUCAM_JPEG_VGA)` | Probe SPI + OV2640, load JPEG tables. |
| `uint32_t take_picture(void)` | Capture into FIFO; returns byte length (0 on failure). |
| `int get_image_data_packet(uint8_t* buf, size_t max_size)` | Read next FIFO chunk; `0` when done, `-1` on error. |
| `void invalidate(void)` | Drop cached setup so the next `begin()` re-probes. |

## Notes

- SPI must already be configured (normally by CDH / `HeptaStorage`) before
  `begin()`.
- Chunked FIFO reads release CS between packets so the SD card can share the bus.
- OV2640 JPEG register tables in `ov2640_regs.h` are adapted from the open-source
  ArduCAM Arduino library (QQVGA / QVGA / VGA / SVGA / UXGA).
- UXGA produces large JPEGs; prefer VGA or smaller when downlinking over XBee.

```cpp
#include "drv/camera_arducam_2mp.h"
CameraArducam2mp cam(/*cs=*/0);

void setup() {
  // SPI already started by HeptaCdh / HeptaStorage
  if (!cam.begin(ARDUCAM_JPEG_VGA)) return;
  uint32_t len = cam.take_picture();
  uint8_t buf[512];
  while (true) {
    int n = cam.get_image_data_packet(buf, sizeof(buf));
    if (n <= 0) break;
    // write buf[0..n) to SD / serial
  }
}
```
