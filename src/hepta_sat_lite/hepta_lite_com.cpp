#include "hepta_lite_com.h"


HeptaLiteCom::HeptaLiteCom()
    : XbeeSerial(_rx_pin, _tx_pin) { // Initialize SoftwareSerial with RX and TX pins
}

void HeptaLiteCom::begin(uint16_t baud_rate) {
  // Start the SoftwareSerial communication at the specified baud rate
  XbeeSerial.begin(baud_rate);
}

char HeptaLiteCom::get_char(void) {
  return XbeeSerial.read();
}

void HeptaLiteCom::send_char(const char c) {
  XbeeSerial.write(c);
}

void HeptaLiteCom::send_text(String text) {
  XbeeSerial.print(text);
}

String HeptaLiteCom::get_text(void) {
  String received_text = "";

  // Check if data is available to read
  while (XbeeSerial.available()) {
    char c = XbeeSerial.read();
    received_text += c; // Append the character to the string
  }

  return received_text; // Return the complete string
}
