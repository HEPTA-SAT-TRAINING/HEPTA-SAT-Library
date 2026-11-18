# COM - Communications (XBee)

Classes:

- `HeptaCom` for HEPTA-SAT
- `HeptaLiteCom` for HEPTA-SAT Lite

Both derive from `HeptaComBase` and use SoftwareSerial RX pin 15 / TX pin 14.
On **both** boards, GP1 is **XBEE_RESET** (active low). The board classes
deassert RESET in `begin()` and expose `xbee_reset()`.

`HeptaComBase` provides the education-facing API and delegates device-specific
communication and AT commands to the [`Xbee` driver](../drivers/xbee.md).
The initial implementation targets XBee AT / Transparent mode.

## Radio setup

Configure the radio for Transparent-mode communication.

- `AP=0`
- a matching PAN ID
- `DH` / `DL` for the peer XBee
- `BD=5` (38400 baud)

The library opens the XBee UART at 38400 baud by default. Lab99 can reopen
it at 9600 via `set_uart_baud()` if factory-default AT fails.

## API

| Method | Description |
|--------|-------------|
| `bool begin()` | Deassert XBEE_RESET (GP1), then start the XBee SoftwareSerial port at 38400 baud. |
| `void xbee_reset()` | Hardware-reset the XBee (active-low GP1). UART is ready ~1 s after RESET. |
| `bool set_uart_baud(uint32_t baud)` | Reopen the XBee UART at `baud` (`end()` then `begin()`). |
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

`begin()` and `xbee_reset()` live on `HeptaCom` / `HeptaLiteCom`. The remaining
methods live on `HeptaComBase`.

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

## Switching to Transparent mode

`set_transparent_mode()` and `set_api_mode()` enter XBee command mode and
change `AP` from the microcontroller. Entering command mode requires guard
times before and after `+++`, so a mode change takes several seconds.

Both functions default to `save=false` and therefore do not send `ATWR`:

```cpp
com.set_transparent_mode();      // Temporary AP=0
com.set_transparent_mode(true);  // AP=0 and write to nonvolatile memory
```

Specify `save=true` only when the setting must survive a power cycle. `ATWR`
writes nonvolatile memory and should not be called frequently. The standard
state for a reused kit is AT / Transparent mode.

`set_api_mode()` only changes the XBee to `AP=1`. HeptaCom v0.1 does not
implement API frame transmission or reception. After switching to API mode,
calling `send("hello\n")` does not work as Transparent-mode communication.
