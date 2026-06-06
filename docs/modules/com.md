# COM &mdash; Communications (XBee)

Class: `HeptaLiteCom` ([hepta_sat_lite/hepta_lite_com.h](../../hepta_sat_lite/hepta_lite_com.h)),
deriving from `HeptaComBase` ([common/hepta_com_base.h](../../common/hepta_com_base.h)).

COM is a thin wrapper over a `SoftwareSerial` port wired to an XBee radio. On the
Lite board it uses RX pin 15 / TX pin 14.

> **In development.** COM currently ships on the Lite board only; the full
> HEPTA-SAT board does not have a `HeptaCom` class yet, but one is planned.
> See [architecture.md](../architecture.md#com-status-in-development).

## API

| Method | Description |
|--------|-------------|
| `void begin(uint16_t baud_rate)` | Start the XBee SoftwareSerial port. |
| `char get_char(void)` | Receive a single character. |
| `void send_char(const char c)` | Transmit a single character. |
| `String get_text(void)` | Receive a line/string. |
| `void send_text(const String text)` | Transmit a string. |

## Example

```cpp
#include <HeptaSatLite.h>
HeptaLiteCom com;

void setup() {
  com.begin(9600);
  com.send_text("HEPTA-Sat online");
}

void loop() {
  String msg = com.get_text();
  if (msg.length() > 0) {
    com.send_text("ack: " + msg);
  }
}
```
