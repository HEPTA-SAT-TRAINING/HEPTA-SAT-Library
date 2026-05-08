/**
 * @file component_test.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-06-05
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#ifndef COMPONENT_TEST_H
#define COMPONENT_TEST_H

#include <Arduino.h>
#include "src/cdh/hepta_cdh.h"

extern void test_sd(HeptaCdh &cdh);

extern void test_mcp3208(void);
extern void test_bno055(void);
extern void test_gps(void);
extern void test_camera(void);
extern void test_motor(void);

extern void test_xbee(void);

#endif /* COMPONENT_TEST_H */
