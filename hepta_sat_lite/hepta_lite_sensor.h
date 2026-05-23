/**
 * @file hepta_lite_sensor.h
 * @brief HEPTA-SAT Lite sensor: BNO055 IMU + Lite-specific temperature formula.
 */

#ifndef HEPTA_LITE_SENSOR_H
#define HEPTA_LITE_SENSOR_H

#include "../common/hepta_sensor_base.h"


class HeptaLiteSensor : public HeptaSensorBase {
  public:
    HeptaLiteSensor() : HeptaSensorBase(/*temp_pin=*/13) {}
    float get_temperature(void);
};


#endif /* HEPTA_LITE_SENSOR_H */
