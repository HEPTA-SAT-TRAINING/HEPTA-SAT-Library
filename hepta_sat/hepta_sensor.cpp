#include "hepta_sensor.h"

#include <SPI.h>
#include <SD.h>


float HeptaSensor::get_temperature(void) {
  // resistance [Ω]
  const float R3  = 110.0f;
  const float R4  = 1000.0f;
  const float R5  = 68000.0f;
  const float Pt  = 100.0f;   // Pt100 nominal resistance at 0°C [Ω]
  const float R_1 = 3.0f;     // voltage divider (GND side)
  const float R_2 = 2.0f;     // voltage divider (signal side)

  // excitation current [A]
  const float I = 0.001f;

  // reference voltage [V]
  const float Vref = 2.5f;

  // temperature coefficient of Pt100 [1/°C]
  const float ce = 0.003851f;

  // op-amp gain and offset
  const float gain   = -R5 * I / R4;
  const float offset = Vref + I * R3;

  // ADC → voltage at pin.
  // Delegates to the base-class helper, which relies on analogReadResolution(12)
  // having been set in HeptaSensorBase::begin(). Consistent with HeptaLiteSensor.
  float adc_volt = read_temp_voltage();

  // undo voltage divider to recover signal voltage
  // V_adc = raw_volt * R_1 / (R_1 + R_2)  →  raw_volt = V_adc * (R_1 + R_2) / R_1
  float raw_volt = adc_volt * (R_1 + R_2) / R_1;

  // back-calculate Pt100 resistance from op-amp output
  float Rth = (raw_volt - offset) / gain + R3;

  // convert resistance to temperature
  float temp = (Rth - Pt) / (ce * Pt);

  return temp;
}

bool HeptaSensor::camera_snapshot(const char* filename) {
  if (!cam.begin(C1098_BAUD_RATE_115200, C1098_JPEG_SIZE_VGA)) {
    return false;
  }

  uint32_t data_len = cam.take_picture();
  if (data_len == 0) {
    Serial.println("No picture data available.");
    return false;
  }

  // O_TRUNC truncates the file to zero length on open so each snapshot
  // starts fresh. FILE_WRITE cannot be used here because it implies O_APPEND.
  File file = SD.open(filename, O_WRITE | O_CREAT | O_TRUNC);
  if (!file) {
    Serial.println("Failed to open file for writing.");
    return false;
  }

  uint8_t buf[512];  // must be at least cam.get_packet_size() (= 512) bytes
  uint32_t total_written = 0;
  while (true) {
    int read_size = cam.get_image_data_packet(buf, sizeof(buf));
    if (read_size <= 0) break;
    file.write(buf, read_size);
    total_written += read_size;
  }
  file.close();

  if (total_written != data_len) {
    Serial.print("Warning: expected ");
    Serial.print(data_len);
    Serial.print(" bytes, but wrote ");
    Serial.println(total_written);
    return false;
  }

  Serial.print("Picture saved successfully. Total bytes: ");
  Serial.println(total_written);
  return true;
}
