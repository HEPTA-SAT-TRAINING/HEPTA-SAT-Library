# CDH &mdash; Command & Data Handling

Classes: `HeptaCdh` (full) / `HeptaLiteCdh` (Lite), both deriving from
`HeptaCdhBase` ([common/hepta_cdh_base.h](../../common/hepta_cdh_base.h)).

CDH wraps the USB `Serial` console, the SD card, and a small command interface.
The board classes only fix the SD pins; all methods below live on the base.

## Lifecycle

| Method | Description |
|--------|-------------|
| `void begin(void)` | Initialize Serial and the SD card. |
| `void wait_for_serial(void)` | Block until the Serial console is connected. |
| `void wait_for_sd(void)` | Block until the SD card is ready. |
| `bool sd_begin(void)` | (Re)initialize the SD card; returns success. |
| `bool sd_is_available(void) const` | True if the SD card initialized OK. |

File operations retry SD initialization automatically, so a card that was
unavailable at boot can be inserted and used later without resetting the board.

## Serial / logging output

These print to the USB `Serial` console.

| Method | Description |
|--------|-------------|
| `size_t print(const String&)` / `print(const char*)` | Print without newline. |
| `size_t println(const String&)` / `println(const char*)` | Print with newline. |
| `size_t printf(const char* fmt, ...)` | `printf`-style formatted print. |
| `size_t vprintf(const char* fmt, va_list)` | `va_list` variant. |
| `size_t write(uint8_t)` / `write(const uint8_t*, size_t)` | Raw byte output. |

## File access

| Method | Description |
|--------|-------------|
| `File open_file(const char* path, int mode = FILE_READ)` | Open an existing file. |
| `File create_file(const char* path)` | Create/truncate a file for writing. |
| `File append_file(const char* path)` | Open a file for appending. |
| `bool file_exists(const char* path)` | Test for existence. |
| `bool remove_file(const char* path)` | Delete a file. |
| `size_t write_file(File&, const char* text)` | Write a string to an open file. |
| `size_t write_file(File&, const uint8_t* buf, size_t size)` | Write raw bytes. |
| `int read_file(File&)` | Read one byte (`-1` at EOF). |
| `int read_file(File&, uint8_t* buf, size_t size)` | Read up to `size` bytes. |
| `size_t printf_file(File&, const char* fmt, ...)` | `printf` into a file. |
| `size_t vprintf_file(File&, const char* fmt, va_list)` | `va_list` variant. |

## Command interface

| Method | Description |
|--------|-------------|
| `bool is_cmd_received(void)` | True if a command byte is waiting on Serial. |
| `char get_command(void)` | Read the pending command byte. |
| `virtual bool command_execute(cmd_t cmd, cmd_arg_t arg = 0)` | Override to dispatch commands. |

`cmd_t` is `uint8_t`; `cmd_arg_t` is `uint64_t` (reserved for future use).
Subclass and override `command_execute()` to implement your own command set.

## Example

```cpp
#include <HeptaSatLite.h>
HeptaLiteCdh cdh;

void setup() {
  cdh.begin();

  File f = cdh.create_file("log.csv");
  cdh.printf_file(f, "t,vbat\n");
  f.close();
}

void loop() {
  if (cdh.is_cmd_received()) {
    char c = cdh.get_command();
    cdh.command_execute((cmd_t)c);
  }
}
```
