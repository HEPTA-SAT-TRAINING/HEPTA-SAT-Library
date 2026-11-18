#include "hepta_lab_usb.h"
#include <stdio.h>
#include <string.h>

static bool parse_hex(const char* text, size_t max_digits, uint64_t& value) {
  if (!text || !*text || strlen(text) > max_digits) return false;
  value = 0;
  for (; *text; ++text) {
    char c = *text;
    if (c >= 'a' && c <= 'f') c -= 'a' - 'A';
    int digit = c >= '0' && c <= '9' ? c - '0' :
                c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
    if (digit < 0) return false;
    value = (value << 4) | digit;
  }
  return true;
}

void HeptaLabUsb::poll(Stream& usb, HeptaComBase& com) {
  while (usb.available()) {
    int c = usb.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (!overflow_) { line_[used_] = 0; handle(usb,com); }
      overflow_ = false;
      used_ = 0;
    } else if (c > 0 && used_ < sizeof(line_) - 1) {
      line_[used_++] = char(c);
    } else overflow_ = true;
  }
}

void HeptaLabUsb::handle(Stream& usb, HeptaComBase& com) {
  char* ctx = nullptr;
  char* prefix = strtok_r(line_, " ", &ctx);
  if (!prefix || strcmp(prefix,"HEPTA1")) return;
  char* op = strtok_r(nullptr, " ", &ctx);
  char* arg = strtok_r(nullptr, " ", &ctx);
  HeptaZigbee* radio = com.zigbee();
  if (!radio || !op || !arg) { usb.println("HEPTA1 ERR unavailable"); return; }
  uint64_t value;
  if (!strcmp(op,"GET") && !strtok_r(nullptr," ",&ctx) && radio->query(arg,value)) {
    char result[48];
    snprintf(result,sizeof(result),"HEPTA1 OK %016llX",(unsigned long long)value);
    usb.println(result);
    return;
  }
  if (!strcmp(op,"NET")) {
    char* sc = strtok_r(nullptr," ",&ctx);
    char* ce = strtok_r(nullptr," ",&ctx);
    uint64_t mask, role;
    if (parse_hex(arg,16,value) && parse_hex(sc,4,mask) && parse_hex(ce,1,role) &&
        role <= 1 && !strtok_r(nullptr," ",&ctx) && radio->configure_network(value,mask,role)) {
      usb.println("HEPTA1 OK"); return;
    }
  }
  if (!strcmp(op,"PING") && parse_hex(arg,16,value) && !strtok_r(nullptr," ",&ctx)) {
    if (radio->ping(value)) usb.println("HEPTA1 OK");
    else {
      XbeeTxResult tx = radio->result();
      if (tx.state == XbeeTxResult::Failed) {
        char detail[24];
        snprintf(detail,sizeof(detail),"HEPTA1 ERR TX %02X",tx.delivery);
        usb.println(detail);
      } else usb.println("HEPTA1 ERR loopback");
    }
    return;
  }
  usb.println("HEPTA1 ERR");
}
