#pragma once

#include "ni2c.hpp"

namespace nix
{

class Aw9523 : public I2cDevice
{
public:
    static constexpr uint8_t DEFAULT_ADDR = 0x58;
    static constexpr uint32_t DEFAULT_SPEED = 400000;

    Aw9523(Logger& logger);
    ~Aw9523();

    esp_err_t Configure();
    esp_err_t ReadRegister(uint8_t reg, uint8_t& data);
    esp_err_t WriteRegister(uint8_t reg, uint8_t data);
    
    esp_err_t SetPortDirection(uint8_t port, uint8_t direction);
    esp_err_t SetPortOutput(uint8_t port, uint8_t value);
    esp_err_t GetPortInput(uint8_t port, uint8_t& value);
};

}
