/**
 * @file hepta_lite_cdh.h
 * @brief HEPTA-SAT Lite CDH: SD pins and Serial wired for the Lite board.
 */

#ifndef HEPTA_LITE_CDH_H
#define HEPTA_LITE_CDH_H

#include "../common/hepta_cdh_base.h"


class HeptaLiteCdh : public HeptaCdhBase {
  public:
    HeptaLiteCdh() : HeptaCdhBase(/*cs=*/3, /*tx=*/19, /*rx=*/16, /*sck=*/18) {}
};


#endif /* HEPTA_LITE_CDH_H */
