#include <imu9axis_icm20948.h>
#include "src/cdh/hepta_cdh.h"

Icm20948 icm;

void setup() {
  Serial.begin(9600);
  icm.begin();
}

void loop() {
  Serial.println("--------------------------");
  icm.print_accel();
  icm.print_gyro();
  icm.print_mag();
  Serial.println("--------------------------");

  delay(1000);
}
