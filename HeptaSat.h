/**
 * @file HeptaSat.h
 * @brief Entry header for the HEPTA-SAT board.
 *
 * Sketches for the HEPTA-SAT board include only this header.
 * It exposes the HEPTA-SAT classes (HeptaCdh / HeptaEps /
 * HeptaSensor) and nothing else. (COM is still in development:
 * a HeptaCom class is planned but currently ships on the
 * HEPTA-SAT Lite board only.)
 *
 * The HEPTA-SAT Lite board has a separate entry header (HeptaSatLite.h)
 * and its classes live in hepta_sat_lite/.
 */

#ifndef HEPTA_SAT_H
#define HEPTA_SAT_H

#include "hepta_sat/hepta_cdh.h"
#include "hepta_sat/hepta_eps.h"
#include "hepta_sat/hepta_sensor.h"

#endif /* HEPTA_SAT_H */
