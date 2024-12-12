#include "hepta_9axis.h"
#include "hepta_rtc.h"
#include <Wire.h>

Hepta9Axis imu;
RealTimeClock rtc;

void setup() {
  Serial.begin(9600);
  imu.begin();
  rtc.begin();
  Serial.println("start");
  rtc.set_time_unix(1733967585);
}

void loop() {
  rtc.print_time();
  Serial.println("--------------------------");
  imu.print_acc();
  imu.print_gyro();
  imu.print_mag();
  Serial.println("--------------------------");

  delay(1000);
}
