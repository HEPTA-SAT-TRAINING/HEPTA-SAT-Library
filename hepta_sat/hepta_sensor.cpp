#include "hepta_sensor.h"

bool HeptaSensor::begin(void) {
  // Run the shared sensor init (ADC resolution, temp pin, BNO055) first,
  // then bring up the GPS SoftwareSerial port.
  bool ok = HeptaSensorBase::begin();
  gps.begin();
  return ok;
}

bool HeptaSensor::gps_get_position(float* lat, float* lon, float* alt) {
  return gps.get_position(lat, lon, alt);
}

bool HeptaSensor::gps_get_velocity(float* velocity, float* heading) {
  return gps.get_velocity(velocity, heading);
}

bool HeptaSensor::gps_get_all(float* lat, float* lon, float* alt,
                              float* velocity, float* heading) {
  return gps.get_all(lat, lon, alt, velocity, heading);
}

bool HeptaSensor::gps_get_gpgga(GpggaData* out) {
  return gps.get_gpgga(out);
}

bool HeptaSensor::gps_get_gprmc(GprmcData* out) {
  return gps.get_gprmc(out);
}

bool HeptaSensor::gps_is_data_available(void) {
  return gps.is_data_available();
}

int HeptaSensor::gps_read_byte(void) {
  return gps.read_byte();
}

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

bool HeptaSensor::camera_snapshot(const char* filename,
                                  ArducamJpegSize jpeg_size) {
  // SPI camera shares the bus with the SD card; CS stays high except during
  // short ArduChip transactions so SD access between chunks remains safe.
  uint32_t data_len = 0;
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (cam.begin(jpeg_size)) {
      data_len = cam.take_picture();
    }
    if (data_len > 0) {
      break;
    }
    cam.invalidate();
    delay(100);
  }
  if (data_len == 0) {
    Serial.println("No picture data available.");
    return false;
  }

  // O_TRUNC truncates the file to zero length on open so each snapshot
  // starts fresh. FILE_WRITE cannot be used here because it implies O_APPEND.
  File file = storage_.open(filename, O_WRITE | O_CREAT | O_TRUNC);
  if (!file) {
    Serial.println("Failed to open file for writing.");
    cam.invalidate();
    return false;
  }

  uint8_t buf[512];
  uint32_t total_written = 0;
  bool read_error = false;
  bool have_soi = false;
  bool have_eoi = false;
  uint8_t prev = 0;
  while (true) {
    int read_size = cam.get_image_data_packet(buf, sizeof(buf));
    if (read_size < 0) {        // transfer fault
      read_error = true;
      break;
    }
    if (read_size == 0) break;  // all data received
    for (int i = 0; i < read_size; i++) {
      const uint8_t b = buf[i];
      if (!have_soi && prev == 0xFF && b == 0xD8) {
        have_soi = true;
      }
      // FIFO often has padding after JPEG; accept EOI anywhere in the stream.
      if (!have_eoi && prev == 0xFF && b == 0xD9) {
        have_eoi = true;
      }
      prev = b;
    }
    size_t write_size = file.write(buf, read_size);
    total_written += write_size;
    if (write_size != static_cast<size_t>(read_size)) {
      read_error = true;
      storage_.invalidate();
      break;
    }
  }
  file.close();

  bool jpeg_ok = have_soi && have_eoi;

  if (read_error || total_written == 0 || total_written > data_len || !jpeg_ok) {
    cam.invalidate();
    Serial.print("Capture invalid: expected ");
    Serial.print(data_len);
    Serial.print(" bytes, wrote ");
    Serial.print(total_written);
    Serial.print(", SOI=");
    Serial.print(have_soi ? "ok" : "NG");
    Serial.print(" EOI=");
    Serial.println(have_eoi ? "ok" : "NG");
    return false;
  }

  Serial.print("Picture saved successfully. Total bytes: ");
  Serial.println(total_written);
  return true;
}

void HeptaSensor::camera_invalidate(void) {
  // Drop the cached setup so the next camera_snapshot() re-probes the ArduChip
  // and OV2640. Call after a camera failure so that once the operator power-cycles
  // the camera the driver starts clean instead of assuming stale state.
  cam.invalidate();
}
