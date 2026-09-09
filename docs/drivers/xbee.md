# XBee driver

Files: [drv/xbee.h](../../drv/xbee.h),
[drv/xbee.cpp](../../drv/xbee.cpp)

`Xbee` owns the SoftwareSerial connection and implements device-specific XBee
behavior:

- raw byte transmission and reception
- command-mode guard times and the `+++` sequence
- AT command transmission and response handling
- `AP=0` / `AP=1` mode changes
- optional `ATWR` nonvolatile storage

The driver does not depend on HEPTA-SAT board classes. RX/TX pins are supplied
to its constructor by `HeptaComBase`. The UART baud rate defaults to 38400
(`BD=5`). `begin(baud)` can reopen the port (Lab99 tries 9600 if 38400 AT fails).

The initial version supports AT / Transparent mode communication. It can change
the radio to API mode, but API frame construction and parsing are not yet
implemented.
