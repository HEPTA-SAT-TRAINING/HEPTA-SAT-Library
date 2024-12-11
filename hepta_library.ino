#include "hepta_9axis.h"
#include "hepta_rtc.h"
#include <Wire.h>

Hepta9Axis imu;
RealTimeClock rtc;


void setup() {
  Serial.begin(9600);
  imu.begin();
  rtc.begin();
  rtc.set_time_unix_ms(100000);
  Serial.println("start");
}

void loop() {
  Serial.print("Time: ");
  Serial.println(rtc.get_time_unix_ms());

  Serial.println("--------------------------");
  imu.print_acc();
  imu.print_gyro();
  imu.print_mag();
  Serial.println("--------------------------");

  delay(250);  
}
