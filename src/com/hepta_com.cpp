#include "hepta_com.h"
#include <stdarg.h>

HeptaCom::HeptaCom(uint16_t baud_rate) {
  Serial1.begin(baud_rate);
}

void HeptaCom::send_text(const char *format, ... ) {
  char s[100];
  va_list args;

  va_start(args, format);
  vsnprintf(s, 100, format, args);
  va_end(args);

  Serial1.write(s);
}
