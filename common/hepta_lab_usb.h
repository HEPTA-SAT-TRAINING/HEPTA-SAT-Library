#pragma once
#include "hepta_com_base.h"

// Call only from sketches opting into the USB lab protocol.
class HeptaLabUsb {
public:
  void poll(Stream& usb, HeptaComBase& com);
private:
  char line_[96] = {};
  size_t used_ = 0;
  bool overflow_ = false;
  void handle(Stream& usb, HeptaComBase& com);
};
