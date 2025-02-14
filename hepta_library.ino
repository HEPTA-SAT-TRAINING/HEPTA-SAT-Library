#include "src/cdh/hepta_cdh.h"
#include "src/eps/hepta_eps.h"
#include "src/sensor/hepta_sensor.h"
#include "src/com/hepta_com.h"

#include <camera_c1098.h>

HeptaSensor sensor;
CameraC1098 cam;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Camera");
  if(cam.begin(C1098_BAUD_RATE_115200, C1098_JPEG_SIZE_VGA)) {
    cam.take_picture();
  }

  delay(1000);
}
