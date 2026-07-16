#ifndef DAC8562_H
#define DAC8562_H

#include <SPI.h>

#define MISO_PIN -1
#define MOSI_PIN 4
#define SCK_PIN 5
#define CS_PIN 6

// DAC8562 命令字节定义
#define DAC_CMD_RESET 0x28
#define DAC_CMD_REF_GAIN_CONFIG 0x38
#define DAC_CMD_WRITE_CH_A 0x00
#define DAC_CMD_WRITE_CH_B 0x01
#define DAC_CMD_WRITE_CH_B_AND_UPDATE_ALL 0x11

class DAC8562
{
public:
    DAC8562(uint32_t frequency = 50000000);
    void begin();
    void write(uint8_t cmd_byte, uint16_t data_val);
    void sync_outputs(uint16_t ch_a_val, uint16_t ch_b_val);

private:
    uint32_t _freq;
    SPISettings _settings;
    void _transfer(uint8_t cmd, uint16_t data);
};

#endif