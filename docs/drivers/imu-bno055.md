# Bno055 &mdash; BNO055 9-axis IMU

Source: [drv/imu9axis_bno055.h](../../drv/imu9axis_bno055.h) ·
[drv/imu9axis_bno055.cpp](../../drv/imu9axis_bno055.cpp)

Driver for the **BNO055** 9-axis absolute-orientation IMU, used over I2C (address
`0x28`) in NDOF fusion mode. Wrapped by [Sensor](../modules/sensor.md) on both
boards.

## API

| Method | Description |
|--------|-------------|
| `void begin(void)` | Start I2C and put the BNO055 in NDOF mode. |
| `void sen_acc(float* ax, float* ay, float* az)` | Acceleration in m/s² (±4 G range). |
| `void sen_gyro(float* gx, float* gy, float* gz)` | Angular rate in deg/s (±125 deg/s). |
| `void sen_mag(float* mx, float* my, float* mz)` | Magnetic field in µT. |
| `void print_acc(void)` / `print_gyro(void)` / `print_mag(void)` | Print to Serial. |

> In normal use, access the IMU through `HeptaSensor` / `HeptaLiteSensor` rather
> than instantiating `Bno055` directly &mdash; the sensor classes own the instance.

## Example

```cpp
#include "drv/imu9axis_bno055.h"
Bno055 imu;

void setup() {
  imu.begin();
}

void loop() {
  float ax, ay, az;
  imu.sen_acc(&ax, &ay, &az);
}
```
