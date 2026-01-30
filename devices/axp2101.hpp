#pragma once

#include "ni2c.hpp"

namespace nix
{

class Axp2101 : public I2cDevice
{
public:
    static constexpr uint8_t DEFAULT_ADDR = 0x34;
    static constexpr uint32_t DEFAULT_SPEED = 400000;

    Axp2101(Logger& logger);
    ~Axp2101();

    esp_err_t Configure();
    esp_err_t ReadRegister(uint8_t reg, uint8_t& data);
    esp_err_t WriteRegister(uint8_t reg, uint8_t data);
};

}
