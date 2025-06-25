#ifndef ADC_MCP3208_H
#define ADC_MCP3208_H

#include <Arduino.h>

class AdcMcp3208 {
  public:
    void begin(uint8_t CS_pin);
    uint16_t get_data(uint8_t channel);

  private:
    uint8_t _cs_pin;
    void spi_block_transaction(uint16_t* send_data, uint16_t* ret_data, uint8_t Byte_size);
};

#endif /* MCP_3208_H */
