/**
 * @file hepta_cdh.h
 * @brief HEPTA-SAT CDH: SD pins and Serial wired for the full board.
 */

#ifndef HEPTA_CDH_H
#define HEPTA_CDH_H

#include "../common/hepta_cdh_base.h"


class HeptaCdh : public HeptaCdhBase {
  public:
    HeptaCdh() : HeptaCdhBase(/*cs=*/3, /*tx=*/19, /*rx=*/16, /*sck=*/18) {}
};


#endif /* HEPTA_CDH_H */
