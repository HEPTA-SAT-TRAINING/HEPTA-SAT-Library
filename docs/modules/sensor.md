# Sensor

Classes: `HeptaSensor` (full) / `HeptaLiteSensor` (Lite), both deriving from
`HeptaSensorBase` ([common/hepta_sensor_base.h](../../common/hepta_sensor_base.h)).

The base provides the **BNO055 9-axis IMU**. The full-board `HeptaSensor`
adds analog **temperature** (Pt100), **GPS** (GP-1818MK), and a **camera**
(Arducam Mini 2MP Plus). Lite has IMU plus an onboard **BME280** for
temperature, humidity, and pressure &mdash; no GPS or camera.

## Shared API (`HeptaSensorBase`)

| Method | Description |
|--------|-------------|
| `bool begin(void)` | Initialize the BNO055 IMU; returns whether it responded. |
| `bool get_acceleration(float* ax, float* ay, float* az)` | Acceleration in m/s². Reinitializes after an I2C failure. |
| `bool get_gyro(float* gx, float* gy, float* gz)` | Angular rate in deg/s. Reinitializes after an I2C failure. |
| `bool get_magnetometer(float* mx, float* my, float* mz)` | Magnetic field in µT. Reinitializes after an I2C failure. |
| `void print_acceleration(void)` / `print_gyro(void)` / `print_magnetometer(void)` | Print to Serial. |

See [drivers/imu-bno055.md](../drivers/imu-bno055.md) for IMU details.

## Full board &mdash; `HeptaSensor`

Adds GPS and camera on top of the shared API. Temperature is a Pt100 bridge
on ADC pin 27 (`get_temperature()`). The GPS methods are thin wrappers over
the [GP-1818MK driver](../drivers/gps-gp1818mk.md).

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

Onboard **BME280** on the same `Wire` bus as the BNO055 (GP4/GP5, address
`0x76`, SDO=GND). No GPS, no camera, no analog temperature formula.

`begin()` initializes the IMU first, then the BME280. The return value is
IMU success (same as Full). If the BME280 is missing, environmental getters
return `NAN`.

`get_temperature()` / `get_humidity()` / `get_pressure()` share one
`Bme280::read()` and cache the latest sample. Failure returns `NAN`.

| Method | Description |
|--------|-------------|
| `bool begin(void)` | IMU then BME280; returns IMU success. |
| `float get_temperature(void)` | Temperature in °C from the BME280. |
| `float get_humidity(void)` | Relative humidity in %. |
| `float get_pressure(void)` | Pressure in hPa. |

See [drivers/bme280-bosch.md](../drivers/bme280-bosch.md).

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

## Example (Lite board)

```cpp
#include <HeptaSatLite.h>
HeptaLiteSensor sensor;

void setup() {
  sensor.begin();
}

void loop() {
  float ax, ay, az;
  sensor.get_acceleration(&ax, &ay, &az);

  float t = sensor.get_temperature();
  float h = sensor.get_humidity();
  float p = sensor.get_pressure();
  delay(1000);
}
```
