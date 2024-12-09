#include "hepta_9axis.h"
#include <Wire.h>

Hepta9Axis imu;

void setup() {
  Serial.begin(9600);
  imu.begin();
  Serial.println("start");
}

void loop() {
  Serial.println("--------------------------");
  imu.print_acc();
  imu.print_gyro();
  imu.print_mag();
  Serial.println("--------------------------");

  delay(250);  
}
