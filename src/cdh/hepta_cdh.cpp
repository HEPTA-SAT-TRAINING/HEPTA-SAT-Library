/**
 * @file hepta_cdh.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "hepta_cdh.h"

cmd_t HeptaCdh::get_command(void) {
  cmd_t cmd = 0;

  char *e;
  uint8_t base = 10;
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // 1行分読み込む
    cmd = (cmd_t)strtol(input.c_str(), &e, base);
  }


  if(cmd!= 0) {
    Serial.print("Received command: ");
    Serial.println(cmd, HEX);
  }

  return cmd; // Return the received command
}
