# hepta_v4_library
Library files for next generation HEPTA-Sat

## CDH SD card access

`HeptaLiteCdh` initializes the SD card in `begin()` and exposes basic file access
through wrapper functions instead of inheriting from `SDClass`.

```cpp
#include <HeptaSatLite.h>

HeptaLiteCdh cdh;

void setup() {
  cdh.begin();

  File file = cdh.create_file("test.txt");
  if (file) {
    cdh.write_file(file, "Hello, HEPTA-Sat\n");
    file.close();
  }

  file = cdh.open_file("test.txt");
  while (file && file.available()) {
    Serial.write(cdh.read_file(file));
  }
  file.close();
}
```
