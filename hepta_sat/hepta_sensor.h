/**
 * @file hepta_sensor.h
 * @brief HEPTA-SAT Sensor: BNO055 IMU + Pt100 bridge temperature circuit.
 */

#ifndef HEPTA_SENSOR_H
#define HEPTA_SENSOR_H

#include "../common/hepta_sensor_base.h"
#include "../drv/camera_c1098.h"


class HeptaSensor : public HeptaSensorBase {
  public:
    HeptaSensor() : HeptaSensorBase(/*temp_pin=*/27) {}
    float get_temperature(void);
    bool camera_snapshot(const char* filename = "picture.jpg");

    /**
     * @brief Invalidate the camera driver's cached setup so the next
     *        camera_snapshot() re-syncs from 14400 baud.
     *
     * Call after a camera failure. The camera VCC is not on an MCU-controllable
     * rail, so recovery requires the operator to physically power-cycle the camera;
     * this ensures the driver re-syncs afterwards instead of trusting stale state.
     */
    void camera_invalidate(void);

  private:
    CameraC1098 cam;
};


#endif /* HEPTA_SENSOR_H */
