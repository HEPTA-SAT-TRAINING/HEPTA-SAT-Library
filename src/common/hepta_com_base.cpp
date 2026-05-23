#include "hepta_com_base.h"


void HeptaComBase::begin(uint16_t baud_rate) {
  // Start the SoftwareSerial communication at the specified baud rate
  XbeeSerial.begin(baud_rate);
}

char HeptaComBase::get_char(void) {
  return XbeeSerial.read();
}

void HeptaComBase::send_char(const char c) {
  XbeeSerial.write(c);
}

void HeptaComBase::send_text(String text) {
  XbeeSerial.print(text);
}

String HeptaComBase::get_text(void) {
  String received_text = "";

  // Check if data is available to read
  while (XbeeSerial.available()) {
    char c = XbeeSerial.read();
    received_text += c; // Append the character to the string
  }

  return received_text; // Return the complete string
}
