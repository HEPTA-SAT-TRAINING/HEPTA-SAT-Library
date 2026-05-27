/**
 * @file hepta_sensor.h
 * @brief HEPTA-SAT Sensor: BNO055 IMU + Pt100 bridge temperature circuit.
 */

#ifndef HEPTA_SENSOR_H
#define HEPTA_SENSOR_H

#include "../common/hepta_sensor_base.h"


class HeptaSensor : public HeptaSensorBase {
  public:
    HeptaSensor() : HeptaSensorBase(/*temp_pin=*/27) {}
    float get_temperature(void);
};


#endif /* HEPTA_SENSOR_H */
