#include "axp2101.hpp"

namespace nix
{

Axp2101::Axp2101(Logger& logger) : I2cDevice(logger)
{
}

Axp2101::~Axp2101()
{
}

esp_err_t Axp2101::Configure()
{
    esp_err_t ret;
    uint8_t data = 0x00;
    
    ret = ReadRegister(0x90, data);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to read register 0x90: %s", esp_err_to_name(ret));
        return ret;
    }
    
    data |= 0b10110100;
    
    struct RegCmd {
        uint8_t reg;
        uint8_t value;
    };
    
    RegCmd cmds[] = {
        {0x90, data},
        {0x99, (uint8_t)(0b11110 - 5)},
        {0x97, (uint8_t)(0b11110 - 2)},
        {0x69, 0b00110101},
        {0x30, 0b111111},
        {0x90, 0xBF},
        {0x94, 33 - 5},
        {0x95, 33 - 5},
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

esp_err_t Axp2101::ReadRegister(uint8_t reg, uint8_t& data)
{
    std::vector<uint8_t> read_data;
    esp_err_t ret = ReadReg(reg, read_data, 1);
    if (ret == ESP_OK && !read_data.empty()) {
        data = read_data[0];
    }
    return ret;
}

esp_err_t Axp2101::WriteRegister(uint8_t reg, uint8_t data)
{
    return WriteReg(reg, {data});
}

}
