#include "hepta_rtc.h"
#include "icm20948.h"

RealTimeClock rtc;
Icm20948 icm;

void setup() {
  Serial.begin(9600);
  // rtc.begin();
  // Serial.println("start");
  // rtc.set_time_unix(1733967585);
  icm.begin();
}

void loop() {
  // rtc.print_time();

  Serial.println("--------------------------");
  icm.print_accel();
  icm.print_gyro();
  Serial.println("--------------------------");

  delay(1000);
}
