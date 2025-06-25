#include "component_test.h"

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  test_bno055();
  delay(1000);
}
