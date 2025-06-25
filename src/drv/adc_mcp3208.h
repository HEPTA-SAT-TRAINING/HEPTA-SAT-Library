/**
 * @file adc_mcp3208.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-06-25
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#ifndef ADC_MCP3208_H
#define ADC_MCP3208_H

#include <Arduino.h>

class AdcMcp3208 {
  public:
    void begin(uint8_t cs_pin, float ref_vol = 3.3);
    uint16_t get_raw_data(uint8_t channel);
    float get_voltage(uint8_t channel);

  private:
    uint8_t _cs_pin;
    float _ref_voltage;
    void spi_block_transaction(uint16_t* send_data, uint16_t* ret_data, uint8_t Byte_size);
};

#endif /* ADC_MCP_3208_H */
