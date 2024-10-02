#include "hepta_temperature.h"
#include "hepta_9axis.h"

HeptaTemp temp_sensor;
Hepta9Axis imu;

void setup() {
  imu.setup();
  float temp = temp_sensor.get_temperature();
}

void loop() {

}
