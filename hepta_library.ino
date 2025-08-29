#include "src/cdh/hepta_cdh.h"
#include "src/eps/hepta_eps.h"
#include "component_test.h"

HeptaCdh cdh;
HeptaEps eps;

void setup() {
  cdh.begin(); // Initialize the command and data handling system
}

void loop() {
  cmd_t cmd = cdh.get_command();

  switch (cmd) {
    case 0: // No command
      break;

    case 1: // SD card test
      Serial.println("SD Card Test");
      test_sd();
      break;

    case 2: // MCP3208 test
      Serial.println("MCP3208 Test");
      test_mcp3208();
      break;

    case 3: // BNO055 test
      Serial.println("BNO055 Test");
      test_bno055();
      break;

    case 4: // GPS GP1818MK test
      Serial.println("GPS GP1818MK Test");
      test_gps();
      break;

    case 5: // Camera C1098 test
      Serial.println("Camera C1098 Test");
      test_camera();
      break;

    case 10:
      eps.init();
      break;

    case 11: // Switch 3.3V on
      Serial.println("Switching 3.3V on");
      eps.switch_3V3_on();
      break;

    case 12: // Switch 3.3V off
      Serial.println("Switching 3.3V off");
      eps.switch_3V3_off();
      break;

    case 13:
      Serial.print("Battery Voltage: ");
      Serial.println(eps.get_battery_voltage());
      break;

    case 99:
      Serial.println("All components test");
      test_sd();
      test_mcp3208();
      test_bno055();
      test_gps();
      test_camera();
      break;

    default:
      Serial.println("Unknown command");
      break;
  }

  delay(1000);
}
