/**
 * @file hepta_lite_sensor.h
 * @brief HEPTA-SAT Lite sensor: BNO055 IMU + onboard BME280 (T/H/P).
 */

#ifndef HEPTA_LITE_SENSOR_H
#define HEPTA_LITE_SENSOR_H

#include "../common/hepta_sensor_base.h"
#include "../drv/bme280_bosch.h"


class HeptaLiteSensor : public HeptaSensorBase {
  public:
    HeptaLiteSensor() : HeptaSensorBase() {}
    bool begin(void);
    float get_temperature(void);
    float get_humidity(void);
    float get_pressure(void);

  private:
    bool _sample_env(void);

    Bme280 bme;
    bool _bme_ok = false;
    bool _env_valid = false;
    unsigned long _env_millis = 0;
    float _temp_c = NAN;
    float _hum_pct = NAN;
    float _press_hpa = NAN;
};


#endif /* HEPTA_LITE_SENSOR_H */
