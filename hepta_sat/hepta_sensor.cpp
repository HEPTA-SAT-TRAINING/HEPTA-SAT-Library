#include "hepta_sensor.h"

#include <SPI.h>
#include <SD.h>


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

bool HeptaSensor::camera_snapshot(const char* filename) {
  // Single capture attempt. A camera hung by ESD/noise cannot be recovered in
  // software because its VCC is not on an MCU/EPS-controllable rail; the operator
  // must physically power-cycle the camera. On failure the caller invalidates the
  // driver so the next attempt re-syncs at 14400 baud.
  // 57600 baud (vs the max 115200) trades a little speed for far better noise/ESD
  // immunity on the UART, which is the main trigger for the camera getting stuck.
  if (!cam.begin(C1098_BAUD_RATE_57600, C1098_JPEG_SIZE_VGA)) {
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
  bool read_error = false;
  bool have_soi = false;       // JPEG SOI (FF D8) at the very start
  const size_t TAIL_N = 64;    // rolling window to find the EOI marker in
  uint8_t tail[TAIL_N];
  size_t  tail_len = 0;
  while (true) {
    int read_size = cam.get_image_data_packet(buf, sizeof(buf));
    if (read_size < 0) {        // transfer fault (timeout / bad packet)
      read_error = true;
      break;
    }
    if (read_size == 0) break;  // all data received
    if (total_written == 0 && read_size >= 2) {
      have_soi = (buf[0] == 0xFF && buf[1] == 0xD8);
    }
    file.write(buf, read_size);
    total_written += read_size;
    // Keep the last TAIL_N bytes of the whole stream for the EOI search.
    for (int i = 0; i < read_size; i++) {
      if (tail_len < TAIL_N) {
        tail[tail_len++] = buf[i];
      } else {
        memmove(tail, tail + 1, TAIL_N - 1);
        tail[TAIL_N - 1] = buf[i];
      }
    }
  }
  file.close();

  // A correct length does not guarantee a valid image, so verify the JPEG markers.
  // The C1098 appends a few padding bytes after the EOI (FF D9), so search the tail
  // for the marker rather than requiring it to be the exact last two bytes.
  bool have_eoi = false;
  for (size_t i = 0; i + 1 < tail_len; i++) {
    if (tail[i] == 0xFF && tail[i + 1] == 0xD9) { have_eoi = true; break; }
  }
  bool jpeg_ok = have_soi && have_eoi;

  if (read_error || total_written != data_len || !jpeg_ok) {
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
  // Drop the cached setup so the next camera_snapshot() runs a full SYNC + INITIAL
  // at 14400 baud. Call after a camera failure so that once the operator physically
  // power-cycles the camera (which boots back to 14400) the driver re-syncs instead
  // of assuming the stale negotiated-baud configuration is still valid.
  cam.invalidate();
}
