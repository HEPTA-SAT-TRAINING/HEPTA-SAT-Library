/**
 * @file gps_async.h
 * @brief Non-blocking GPS NMEA parser with mutex-protected cache.
 */

#ifndef GPS_ASYNC_H
#define GPS_ASYNC_H

#include "gps_fix.h"

class Gps1818mk;

void gps_async_init(void);
void gps_async_begin(Gps1818mk *gps);
void gps_service(void);
bool gps_get_latest(GpsFix *out);

#endif /* GPS_ASYNC_H */
