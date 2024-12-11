/**
 * @file hepta_rtc.h
 * @author Masaki Naito
 * @date 2024-10-24
 */


#ifndef RTC_MCP79410_H
#define RTC_MCP79410_H

#include <Arduino.h>

class RealTimeClock {
  public:
    int8_t begin();
    uint64_t get_time_unix_ms();
    void set_time_unix_ms(uint64_t unix_time_ms);

  private:
    typedef enum {
      RTCSEC = 0x00,
      RTCMIN,
      RTCHOUR,
      RTCWKDAY,
      RTCDATE,
      RTCMTH,
      RTCYEAR,
      CONTROL,
      OSCTRIM,
      EEUNLOCK,
      ALM0SEC,
      ALM0MIN,
      ALM0HOUR,
      ALM0WKDAY,
      ALM0DATE,
      ALM0MTH
    } RTCC_REG;

    typedef enum {
      ONES_DIGIT,
      TENS_DIGIT
    } DIGIT;

    void enable_crystal_operation(void);
    void set_24_hours_mode(void);
    uint8_t get_second(void);
    void set_second(uint8_t sec);
    uint8_t get_minute(void);
    void set_minute(uint8_t min);
    uint8_t get_hour(void);
    void set_hour(uint8_t hour);
    uint8_t get_date(void);
    void set_date(uint8_t date);
    uint8_t get_month(void);
    void set_month(uint8_t month);
    uint16_t get_year(void);
    void set_year(uint16_t year);
    void mcp79410_write_reg(RTCC_REG reg, uint8_t value);
    uint8_t mcp79410_read_reg(RTCC_REG reg);
    uint16_t get_digit(uint16_t value, DIGIT digit);
    uint8_t get_weekday();
};

#endif /* RTC_MCP79410_H */
