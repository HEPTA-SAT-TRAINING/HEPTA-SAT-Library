# Sensor

Classes: `HeptaSensor` (full) / `HeptaLiteSensor` (Lite), both deriving from
`HeptaSensorBase` ([common/hepta_sensor_base.h](../../common/hepta_sensor_base.h)).

The base provides the **BNO055 9-axis IMU** and an analog **temperature** reading.
The full-board `HeptaSensor` additionally exposes **GPS** (GP-1818MK) and a
**camera** (Arducam Mini 2MP Plus). The Lite board has IMU + temperature only.

## Shared API (`HeptaSensorBase`)

| Method | Description |
|--------|-------------|
| `bool begin(void)` | Initialize the BNO055 IMU; returns whether it responded. |
| `bool get_acceleration(float* ax, float* ay, float* az)` | Acceleration in m/s². Reinitializes after an I2C failure. |
| `bool get_gyro(float* gx, float* gy, float* gz)` | Angular rate in deg/s. Reinitializes after an I2C failure. |
| `bool get_magnetometer(float* mx, float* my, float* mz)` | Magnetic field in µT. Reinitializes after an I2C failure. |
| `void print_acceleration(void)` / `print_gyro(void)` / `print_magnetometer(void)` | Print to Serial. |

Each board adds its own `get_temperature(void)` returning degrees Celsius, using
its board-specific conversion formula (temperature ADC: pin 27 full, pin 13 Lite).

See [drivers/imu-bno055.md](../drivers/imu-bno055.md) for IMU details.

## Full board &mdash; `HeptaSensor`

Adds GPS and camera on top of the shared API. The GPS methods are thin wrappers
over the [GP-1818MK driver](../drivers/gps-gp1818mk.md).

### GPS

| Method | Description |
|--------|-------------|
| `bool gps_get_position(float* lat, float* lon, float* alt)` | Position from GPGGA. |
| `bool gps_get_velocity(float* velocity, float* heading)` | Speed (m/s) + heading from GPRMC. |
| `bool gps_get_all(float* lat, float* lon, float* alt, float* velocity, float* heading)` | Both in one call. |
| `bool gps_get_gpgga(GpggaData* out)` | Full GPGGA (adds fix quality, sat count, HDOP). |
| `bool gps_get_gprmc(GprmcData* out)` | Full GPRMC (adds UTC time, date). |
| `bool gps_is_data_available(void)` | True if a raw NMEA byte is waiting. |
| `int  gps_read_byte(void)` | Non-blocking raw NMEA byte (`-1` if none). |

Latitude/longitude are decimal degrees (negative = South/West); altitude is
meters above mean sea level. `GpggaData` / `GprmcData` are defined in
[drv/gps_gp1818mk.h](../../drv/gps_gp1818mk.h).

### Camera

| Method | Description |
|--------|-------------|
| `bool camera_snapshot(const char* filename = "picture.jpg", ArducamJpegSize jpeg_size = ARDUCAM_JPEG_VGA)` | Capture a JPEG to the SD card at the chosen resolution (default VGA). Camera and SD initialization are retried after failure. |
| `void camera_invalidate(void)` | Drop cached camera setup so the next snapshot re-probes SPI / I2C. |

`camera_snapshot()` invalidates stale camera state and retries automatically.
`camera_invalidate()` remains available for an explicit power-cycle workflow.
The camera VCC is not on an MCU-controllable rail, so some hardware faults still
need an operator power-cycle. See
[drivers/camera-arducam-2mp.md](../drivers/camera-arducam-2mp.md).

## Lite board &mdash; `HeptaLiteSensor`

Shared IMU/temperature API only, with the Lite temperature formula. No GPS, no
camera.

## Example (full board)

```cpp
#include <HeptaSat.h>
HeptaSensor sensor;

void setup() {
  sensor.begin();
}

void loop() {
  float ax, ay, az;
  sensor.get_acceleration(&ax, &ay, &az);

  float lat, lon, alt;
  if (sensor.gps_get_position(&lat, &lon, &alt)) {
    // valid fix
  }

  sensor.camera_snapshot("img.jpg");  // VGA by default
  sensor.camera_snapshot("small.jpg", ARDUCAM_JPEG_QVGA);
  delay(1000);
}
```
