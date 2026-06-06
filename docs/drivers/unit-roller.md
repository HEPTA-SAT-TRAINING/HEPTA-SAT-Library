# UnitRollerI2C &mdash; M5 Unit Roller485

Source: [drv/unit_rolleri2c.hpp](../../drv/unit_rolleri2c.hpp) ·
[drv/unit_rolleri2c.cpp](../../drv/unit_rolleri2c.cpp) ·
[drv/unit_roller_common.hpp](../../drv/unit_roller_common.hpp) ·
[drv/unit_roller_common.cpp](../../drv/unit_roller_common.cpp)

Class: `UnitRollerI2C`.

This driver is **vendored from M5Stack** and slightly modified:

- Upstream repo: <https://github.com/m5stack/M5Unit-Roller>
- Protocol documentation:
  <https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/776/Unit-Roller485-I2C-Protocol-EN.pdf>

It is not part of the core HEPTA-Sat subsystem stack; it is included for projects
that drive an M5 Unit Roller485.

## Connection (I2C)

| Signal | Pin |
|--------|-----|
| SDA | pin 4 |
| SCL | pin 5 |
| GND | GND |
| Vcc | 5V |

## Usage

Refer to the upstream M5Stack documentation linked above for the full API; the
local copy follows the same I2C protocol.
