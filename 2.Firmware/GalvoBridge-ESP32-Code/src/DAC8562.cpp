#include <Arduino.h>
#include "DAC8562.h"

DAC8562::DAC8562(uint32_t frequency)
    : _freq(frequency),
      _settings(frequency, MSBFIRST, SPI_MODE1) {} // DAC8562需Mode 1

void DAC8562::begin()
{
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    // 初始化设备
    write(DAC_CMD_RESET, 0x0001);
    write(DAC_CMD_REF_GAIN_CONFIG, 0x0001);
}

void DAC8562::_transfer(uint8_t cmd, uint16_t data)
{
    SPI.beginTransaction(_settings);
    digitalWrite(CS_PIN, LOW);

    SPI.transfer(cmd);
    SPI.transfer((data >> 8) & 0xFF);
    SPI.transfer(data & 0xFF);

    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
}

void DAC8562::write(uint8_t cmd_byte, uint16_t data_val)
{
    _transfer(cmd_byte, data_val);
}

void DAC8562::sync_outputs(uint16_t ch_a_val, uint16_t ch_b_val)
{
    // 按照原逻辑，先写A但不更新，后写B并同时更新A和B
    _transfer(DAC_CMD_WRITE_CH_A, ch_a_val);
    _transfer(DAC_CMD_WRITE_CH_B_AND_UPDATE_ALL, ch_b_val);
}