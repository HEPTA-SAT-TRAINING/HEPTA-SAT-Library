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
| `bool set_api_mode(bool save=false)` | Set `AP=1` for a sketch that handles raw frames itself. |
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

`set_transparent_mode()` and `set_api_mode()` do not write the setting to
nonvolatile memory unless `save=true` is passed.

## S2C Zigbee joint labs

Use the shipped 38400-baud pair with saved AP=0, AO=0 and a unicast DH/DL.
The managed overload pulses the existing reset line and enables AP=1 in RAM.
Check the return from begin before using the radio. Full and Lite share the
same interface. Power-cycle the radio when returning to an older sketch.

- `begin(XBEE_MODE_API)` offers byte-stream reads from the paired sender.
- `begin(XBEE_MODE_API, XbeeReceive::Packets)` offers complete packets from all
  senders via `receive_packet(packet)`. Each carries source, options, length
  and binary data. Four packets can wait. A full queue drops a whole packet
  and increments `zigbee()->dropped()`. Poll regularly, including during other work.
- `send`, `printf` and `write` send to the DH/DL pair, splitting at the radio's
  NP limit. Success means local submission. The last fragment's radio status
  is available from `zigbee()->paired_result()`. Subsequent fragments wait for
  the previous radio result. A returned byte count is not an application ACK.
- `send_to(peer, bytes, size)` submits one unicast and returns its nonzero
  frame ID. Zero means the request was rejected (including busy or oversize).
  Poll `tx_result()` for Pending, Delivered, Failed or TimedOut and match its
  frame_id. Collect it before submitting the next addressed packet. Default
  output uses a separate slot and retains its destination.
- `zigbee()->query("DB", value)` reads a numeric register. The joint lab USB
  service uses local queued AT requests for temporary network changes.
  `zigbee()->ping(peer)` verifies a matching Zigbee loopback response.

Typical packet reply:

```cpp
XbeePacket packet;
if (com.tx_result().state != XbeeTxResult::Pending && com.receive_packet(packet)) {
  const uint8_t reply[] = {'O', 'K'};
  com.send_to(packet.source, reply, sizeof(reply));
}
```

Run the host regression from the library root with a C++17 compiler:

```sh
c++ -std=c++17 -DHEPTA_ZIGBEE_TEST -Itests/zigbee/stubs tests/zigbee/test_zigbee.cpp drv/zigbee.cpp -o zigbee_test
./zigbee_test
```
