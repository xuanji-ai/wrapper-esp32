#include "aw9523.hpp"

namespace nix
{

Aw9523::Aw9523(Logger& logger) : I2cDevice(logger)
{
}

Aw9523::~Aw9523()
{
}

esp_err_t Aw9523::Configure()
{
    esp_err_t ret;
    
    struct RegCmd {
        uint8_t reg;
        uint8_t value;
    };
    
    RegCmd cmds[] = {
        {0x02, 0b00000111},
        {0x03, 0b10001111},
        {0x04, 0b00011000},
        {0x05, 0b00001100},
        {0x11, 0b00010000},
        {0x12, 0b11111111},
        {0x13, 0b11111111},
    };
    
    for (const auto& cmd : cmds) {
        ret = WriteRegister(cmd.reg, cmd.value);
        if (ret != ESP_OK) {
            m_logger.Error("Failed to write register 0x%02X: %s", cmd.reg, esp_err_to_name(ret));
            return ret;
        }
    }
    
    m_logger.Info("Device configured successfully");
    return ESP_OK;
}

esp_err_t Aw9523::ReadRegister(uint8_t reg, uint8_t& data)
{
    std::vector<uint8_t> read_data;
    esp_err_t ret = ReadReg(reg, read_data, 1);
    if (ret == ESP_OK && !read_data.empty()) {
        data = read_data[0];
    }
    return ret;
}

esp_err_t Aw9523::WriteRegister(uint8_t reg, uint8_t data)
{
    return WriteReg(reg, {data});
}

esp_err_t Aw9523::SetPortDirection(uint8_t port, uint8_t direction)
{
    uint8_t reg = (port == 0) ? 0x04 : 0x05;
    return WriteRegister(reg, direction);
}

esp_err_t Aw9523::SetPortOutput(uint8_t port, uint8_t value)
{
    uint8_t reg = (port == 0) ? 0x02 : 0x03;
    return WriteRegister(reg, value);
}

esp_err_t Aw9523::GetPortInput(uint8_t port, uint8_t& value)
{
    uint8_t reg = (port == 0) ? 0x00 : 0x01;
    return ReadRegister(reg, value);
}

}
