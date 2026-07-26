/**
 * @file hepta_cdh.h
 * @brief HEPTA-SAT CDH: SD pins and Serial wired for the full board (V4.1.1).
 */

#ifndef HEPTA_CDH_H
#define HEPTA_CDH_H

#include "../common/hepta_cdh_base.h"


class HeptaCdh : public HeptaCdhBase {
  public:
    HeptaCdh() : HeptaCdhBase(/*cs=*/3, /*tx=*/19, /*rx=*/16, /*sck=*/18) {}

    /**
     * @brief Hold CAMERA_CS inactive, then start Serial + SD (shared SPI bus).
     *
     * On V4.1.1 the Arducam shares SPI with the SD card and MCP3208. GP0 must
     * be driven high before any SPI traffic so a connected camera stays deselected
     * until a future camera driver owns the bus.
     */
    void begin(void) {
      pinMode(_camera_cs_pin, OUTPUT);
      digitalWrite(_camera_cs_pin, HIGH);
      HeptaCdhBase::begin();
    }

  private:
    const uint8_t _camera_cs_pin = 0;
};


#endif /* HEPTA_CDH_H */
