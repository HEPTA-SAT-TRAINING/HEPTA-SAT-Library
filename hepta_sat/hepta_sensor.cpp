#include "hepta_sensor.h"


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

  // ADC → voltage at pin (12-bit, Vref = 3.3 V)
  float adc_volt = analogRead(_temp_pin) * 3.3f / 4095.0f;

  // undo voltage divider to recover signal voltage
  // V_adc = raw_volt * R_1 / (R_1 + R_2)  →  raw_volt = V_adc * (R_1 + R_2) / R_1
  float raw_volt = adc_volt * (R_1 + R_2) / R_1;

  // back-calculate Pt100 resistance from op-amp output
  float Rth = (raw_volt - offset) / gain + R3;

  // convert resistance to temperature
  float temp = (Rth - Pt) / (ce * Pt);

  return temp;
}
