# COM - Communications (XBee)

Classes:

- `HeptaCom` for HEPTA-SAT
- `HeptaLiteCom` for HEPTA-SAT Lite

Both derive from `HeptaComBase` and use SoftwareSerial RX pin 15 / TX pin 14.
`HeptaComBase` provides the education-facing API and delegates device-specific
communication and AT commands to the [`Xbee` driver](../drivers/xbee.md).
The initial implementation targets XBee AT / Transparent mode.

## API

| Method | Description |
|--------|-------------|
| `bool begin()` | Start the XBee SoftwareSerial port at the fixed 38400 baud rate. |
| `size_t print(...)` / `println(...)` / `printf(...)` | Formatted downlink output (same signatures as CDH). |
| `size_t write(uint8_t)` / `write(const uint8_t*, size_t)` | Raw byte output (same signatures as CDH). |
| `bool is_cmd_received()` | Return whether a command byte is waiting on the XBee link. |
| `char get_command()` | Read one command byte, ignoring line endings. |
| `bool send(const char* text)` | Send a null-terminated string without adding a newline. |
| `bool send(const uint8_t* data, size_t length)` | Send raw bytes. |
| `int receive(...)` | Receive bytes, optionally waiting for a timeout. |
| `int receive_until(...)` | Receive text up to a terminator. |
| `bool available()` | Return whether at least one byte can be read. |
| `bool enter_command_mode()` | Enter XBee command mode using the `+++` guard times. |
| `bool exit_command_mode()` | Send `ATCN` and leave command mode. |
| `bool send_at_command(...)` | Send an AT command and read its response. |
| `bool set_transparent_mode(bool save=false)` | Set `AP=0`. |
| `bool set_api_mode(bool save=false)` | Set `AP=1`; API frames are not implemented. |
| `const char* last_error()` | Return the most recent error message. |

The legacy `get_char()`, `send_char()`, `get_text()`, and `send_text()` methods
remain available for existing sketches.

`print`, `println`, `printf`, `write`, `is_cmd_received`, and `get_command` mirror
the CDH API so sketches can switch from `cdh.*` to `com.*` for wireless telemetry
and command handling while keeping `cdh` for SD card access.

## Example

```cpp
#include <HeptaSat.h>

HeptaCom com;

void setup() {
  Serial.begin(115200);
  com.begin();
  com.send("HEPTA-Sat online\n");
}

void loop() {
  char buffer[64];
  int received = com.receive_until('\n', buffer, sizeof(buffer), 1000);

  if (received > 0) {
    Serial.println(buffer);
  }
}
```

`set_transparent_mode()` and `set_api_mode()` do not write the setting to
nonvolatile memory unless `save=true` is passed. API frame transmission and
reception are not implemented in v0.1.
