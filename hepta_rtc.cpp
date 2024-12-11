/**
 * @file hepta_rtc.c
 * @author Masaki Naito
 * @date 2024-10-24
 */

#include "hepta_rtc.h"
#include "Wire.h"

// 7bit address
#define MCP79410_RTCC_ADDR   0x6F

#define SECONDS_ONES_DIGIT_POS  0
#define SECONDS_TENS_DIGIT_POS  4
#define START_OSC_POS           7
#define SECONDS_TENS_DIGIT_MSK  0x07 << SECONDS_TENS_DIGIT_POS
#define SECONDS_ONES_DIGIT_MSK  0x0F << SECONDS_ONES_DIGIT_POS

#define MINUTES_ONES_DIGIT_POS  0
#define MINUTES_TENS_DIGIT_POS  4
#define MINUTES_TENS_DIGIT_MSK  0x07 << MINUTES_TENS_DIGIT_POS
#define MINUTES_ONES_DIGIT_MSK  0x0F << MINUTES_ONES_DIGIT_POS

// 24時間形式での使用しか想定していない
#define HOURS_ONES_DIGIT_POS    0
#define HOURS_TENS_DIGIT_POS    4
#define HOURS_12_OR_24_POS      6
#define HOURS_TENS_DIGIT_MSK    0x03 << HOURS_TENS_DIGIT_POS
#define HOURS_ONES_DIGIT_MSK    0x0F << HOURS_ONES_DIGIT_POS
#define HOURS_12_OR_24_MSK      0x01 << HOURS_12_OR_24_POS

#define DATE_ONES_DIGIT_POS     0
#define DATE_TENS_DIGIT_POS     4
#define DATE_ONES_DIGIT_MSK     0x0F << DATE_ONES_DIGIT_POS
#define DATE_TENS_DIGIT_MSK     0x03 << DATE_TENS_DIGIT_POS

#define MONTH_ONES_DIGIT_POS    0
#define MONTH_TENS_DIGIT_POS    4
#define MONTH_ONES_DIGIT_MSK    0x0F << MONTH_ONES_DIGIT_POS
#define MONTH_TENS_DIGIT_MSK    1 << MONTH_TENS_DIGIT_POS

#define YEAR_ONES_DIGIT_POS     0
#define YEAR_TENS_DIGIT_POS     4
#define YEAR_ONES_DIGIT_MSK     0x0F << YEAR_ONES_DIGIT_POS
#define YEAR_TENS_DIGIT_MSK     0x0F << YEAR_TENS_DIGIT_POS

#define ENABLE_EXT_OSC_POS      3


int8_t RealTimeClock::begin() {
  Wire.begin();
  enable_crystal_operation();
  set_24_hours_mode();
  // get_weekday();
}

uint64_t RealTimeClock::get_time_unix_ms() {
  struct tm calender_time;
  time_t unix_time;

  calender_time.tm_sec = get_second();
  calender_time.tm_min = get_minute();
  calender_time.tm_hour = get_hour();
  calender_time.tm_mday = get_date();

  // tm構造体の月は1月からの経過月数
  calender_time.tm_mon = get_month() - 1;

  // tm構造体の年は1900年からの経過年数
  calender_time.tm_year = get_year() - 1900;

  Serial.print(calender_time.tm_year + 1900);
  Serial.print(" ");
  Serial.print(calender_time.tm_mon + 1);
  Serial.print(" ");
  Serial.print(calender_time.tm_mday);
  Serial.print(" ");
  Serial.print(calender_time.tm_hour);
  Serial.print(" ");
  Serial.print(calender_time.tm_min);
  Serial.print(" ");
  Serial.println(calender_time.tm_sec);

  unix_time = mktime(&calender_time);

  // ミリ秒にするために1000倍して返す
  return (unix_time * 1000);
}

void RealTimeClock::set_time_unix_ms(uint64_t unix_time_ms) {
  time_t unix_time = unix_time_ms / 1000;
  struct tm *calender_time = gmtime(&unix_time);

  set_year(calender_time->tm_year + 1900);
  set_month(calender_time->tm_mon + 1);
  set_date(calender_time->tm_mday);
  set_hour(calender_time->tm_hour);
  set_minute(calender_time->tm_min);
  set_second(calender_time->tm_sec);
}

/*------------------------------------------
  Private Functions
-------------------------------------------*/
void RealTimeClock::enable_crystal_operation(void) {
  mcp79410_write_reg(CONTROL, 0 << ENABLE_EXT_OSC_POS);
  mcp79410_write_reg(RTCSEC, 1 << START_OSC_POS);
}

void RealTimeClock::set_24_hours_mode(void) {
  // ビットを1にすると12hになってしまうのでビット反転．
  // mcp79410_write_reg(RTCHOUR, ~HOURS_12_OR_24_MSK);
}

uint8_t RealTimeClock::get_second(void) {
  uint8_t tmp = mcp79410_read_reg(RTCSEC);

  uint8_t tens_digit = (tmp & SECONDS_TENS_DIGIT_MSK) >> SECONDS_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & SECONDS_ONES_DIGIT_MSK) >> SECONDS_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit);
}

void RealTimeClock::set_second(uint8_t sec) {
  uint8_t tens_digit = get_digit(sec, TENS_DIGIT);
  uint8_t ones_digit = get_digit(sec, ONES_DIGIT);

  uint8_t write_val = tens_digit << SECONDS_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCSEC, write_val | 1 << START_OSC_POS);
}

uint8_t RealTimeClock::get_minute(void) {
  uint8_t tmp = mcp79410_read_reg(RTCMIN);

  uint8_t tens_digit = (tmp & MINUTES_TENS_DIGIT_MSK) >> MINUTES_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & MINUTES_ONES_DIGIT_MSK) >> MINUTES_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit);
}

void RealTimeClock::set_minute(uint8_t min) {
  uint8_t tens_digit = get_digit(min, TENS_DIGIT);
  uint8_t ones_digit = get_digit(min, ONES_DIGIT);

  uint8_t write_val = tens_digit << MINUTES_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCMIN, write_val);
}

uint8_t RealTimeClock::get_hour(void) {
  uint8_t tmp = mcp79410_read_reg(RTCHOUR);

  uint8_t tens_digit = (tmp & HOURS_TENS_DIGIT_MSK) >> HOURS_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & HOURS_ONES_DIGIT_MSK) >> HOURS_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit);
}

void RealTimeClock::set_hour(uint8_t hour) {
  uint8_t tens_digit = get_digit(hour, TENS_DIGIT);
  uint8_t ones_digit = get_digit(hour, ONES_DIGIT);

  uint8_t write_val = tens_digit << HOURS_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCHOUR, write_val);
}

uint8_t RealTimeClock::get_weekday(void) {
  Serial.println("get weekday");
  // uint8_t tmp = mcp79410_read_reg(RTCWKDAY);

  return (0);
}

uint8_t RealTimeClock::get_date(void) {
  uint8_t tmp = mcp79410_read_reg(RTCDATE);

  uint8_t tens_digit = (tmp & DATE_TENS_DIGIT_MSK) >> DATE_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & DATE_ONES_DIGIT_MSK) >> DATE_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit);
}

void RealTimeClock::set_date(uint8_t date) {
  uint8_t tens_digit = get_digit(date, TENS_DIGIT);
  uint8_t ones_digit = get_digit(date, ONES_DIGIT);

  uint8_t write_val = tens_digit << DATE_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCDATE, write_val);
}

uint8_t RealTimeClock::get_month(void) {
  uint8_t tmp = mcp79410_read_reg(RTCMTH);

  uint8_t tens_digit = (tmp & MONTH_TENS_DIGIT_MSK) >> MONTH_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & MONTH_ONES_DIGIT_MSK) >> MONTH_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit);
}

void RealTimeClock::set_month(uint8_t month) {
  uint8_t tens_digit = get_digit(month, TENS_DIGIT);
  uint8_t ones_digit = get_digit(month, ONES_DIGIT);

  uint8_t write_val = tens_digit << MONTH_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCMTH, write_val);
}

uint16_t RealTimeClock::get_year(void) {
  uint8_t tmp = mcp79410_read_reg(RTCYEAR);

  uint8_t tens_digit = (tmp & YEAR_TENS_DIGIT_MSK) >> YEAR_TENS_DIGIT_POS;
  uint8_t ones_digit = (tmp & YEAR_ONES_DIGIT_MSK) >> YEAR_ONES_DIGIT_POS;

  return (10 * tens_digit + ones_digit + 2000);
}

void RealTimeClock::set_year(uint16_t year) {
  uint8_t tens_digit = get_digit(year, TENS_DIGIT);
  uint8_t ones_digit = get_digit(year, ONES_DIGIT);

  uint8_t write_val = tens_digit << YEAR_TENS_DIGIT_POS | ones_digit;

  mcp79410_write_reg(RTCYEAR, write_val);
}

void RealTimeClock::mcp79410_write_reg(RTCC_REG reg, uint8_t value) {
  Wire.beginTransmission(MCP79410_RTCC_ADDR);
  Wire.write((uint8_t)reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t RealTimeClock::mcp79410_read_reg(RTCC_REG reg) {
  uint8_t ret;

  Wire.beginTransmission(MCP79410_RTCC_ADDR);
  Wire.write((uint8_t)reg);

  Wire.requestFrom(MCP79410_RTCC_ADDR, 1);
  if (Wire.available() == 1) {
    ret = Wire.read();
  }

  return ret;
}

uint16_t RealTimeClock::get_digit(uint16_t value, DIGIT digit) {
  uint16_t mod_value = value % (uint16_t)pow(10, (uint8_t)digit + 1);
  return (uint16_t)(mod_value / pow(10, (uint8_t)digit));
}
