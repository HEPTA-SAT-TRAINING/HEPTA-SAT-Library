#include "hepta_lite_sensor.h"


float HeptaLiteSensor::get_temperature(void) {
  float voltage = read_temp_voltage();
  return (voltage - 0.6f) * 100.0f;
}
