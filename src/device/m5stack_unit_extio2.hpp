#pragma once

#include "wrapper/logger.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/display.hpp"

class UnitExtio2 : public wrapper::I2cDevice
{



  enum Reg : uint8_t
  {
    MODE_BASE = 0x00,
    MODE_IO0 = 0x00,
    MODE_IO1,
    MODE_IO2,
    MODE_IO3,
    MODE_IO4,
    MODE_IO5,
    MODE_IO6,
    MODE_IO7,

    OUTPUT_CTL_BASE = 0x10,
    OUTPUT_CTL_IO0 = 0x10,
    OUTPUT_CTL_IO1,
    OUTPUT_CTL_IO2,
    OUTPUT_CTL_IO3,
    OUTPUT_CTL_IO4,
    OUTPUT_CTL_IO5,
    OUTPUT_CTL_IO6,
    OUTPUT_CTL_IO7,
    OUTPUTS_CTL = 0x18,

    DIGITAL_INPUT_BASE = 0x20,
    DIGITAL_INPUT_IO0 = 0x20,
    DIGITAL_INPUT_IO1,
    DIGITAL_INPUT_IO2,
    DIGITAL_INPUT_IO3,
    DIGITAL_INPUT_IO4,
    DIGITAL_INPUT_IO5,
    DIGITAL_INPUT_IO6,
    DIGITAL_INPUT_IO7,
    DIGITAL_INPUTS = 0x28,

    ANALOG_INPUT_8B = 0x30,
    ANALOG_INPUT_12B = 0x40,

    SERVO_ANGLE_8B = 0x50,
    SERVO_PULSE_16B = 0x60,

    RGB_24B = 0x70,

    FW_VERSION = 0xFE,
    ADDRESS = 0xFF
  };

  constexpr static inline uint8_t REG_MODE_IO(uint8_t pin)
  {
    return static_cast<uint8_t>(Reg::MODE_BASE) + pin;
  }

  constexpr static inline uint8_t REG_OUTPUT_CTL_IO(uint8_t pin)
  {
    return static_cast<uint8_t>(Reg::OUTPUT_CTL_BASE) + pin;
  }

  constexpr static inline uint8_t REG_DIGITAL_INPUT_IO(uint8_t pin)
  {
    return static_cast<uint8_t>(Reg::DIGITAL_INPUT_BASE) + pin;
  }

  constexpr static inline uint8_t REG_ANALOG_INPUT_8B(uint8_t pin)
  {
    return static_cast<uint8_t>(Reg::ANALOG_INPUT_8B) + pin;
  }

  constexpr static inline uint8_t REG_ANALOG_INPUT_12B(uint8_t pin)
  {
    return static_cast<uint8_t>(Reg::ANALOG_INPUT_12B) + pin;
  }

public:

  static constexpr uint8_t I2C_ADDR_DEFAULT = 0x45;
  static constexpr uint32_t I2C_SPEED_HZ = 400000;

  enum Mode : uint8_t
  {
    DIGITAL_INPUT = 0,
    DIGITAL_OUTPUT,
    ADC_INPUT,
    SERVO_CTL,
    RGB_LED
  };

  enum AnalogReadMode : uint8_t
  {
    BITS8 = 0,
    BITS12 = 1
  };

  UnitExtio2(wrapper::Logger &logger) : I2cDevice(logger)
  {
  }

  ~UnitExtio2() = default;

  bool Init(const wrapper::I2cBus &bus)
  {
    wrapper::I2cDeviceConfig cfg = wrapper::I2cDeviceConfig(I2C_ADDR_DEFAULT, I2C_SPEED_HZ);
    return I2cDevice::Init(bus, cfg) == ESP_OK;
  }

  void SetMode(int pin, Mode mode)
  {
    GetLogger().Info("SetMode Pin %d: %d", pin, static_cast<uint8_t>(mode));
    WriteReg8(REG_MODE_IO(pin), static_cast<uint8_t>(mode), 1000);
  }

  Mode GetMode(int pin)
  {
    uint8_t mode = 0xFF;
    if (ReadReg8(REG_MODE_IO(pin), mode, 1000) == ESP_OK)
    {
      return static_cast<Mode>(mode);
    }
    return static_cast<Mode>(0xFF);
  }

  void SetModeAll(Mode mode)
  {
    GetLogger().Info("SetMode All: %d", static_cast<uint8_t>(mode));
    for (int pin = 0; pin < 8; pin++)
    {
      WriteReg8(REG_MODE_IO(pin), static_cast<uint8_t>(mode), 1000);
    }
  }

  bool SetDigitalOutput(int pin, bool state)
  {
    GetLogger().Debug("SetDigitalOutput: %d", state ? 1 : 0);
    return WriteReg8(REG_OUTPUT_CTL_IO(pin), state ? 1 : 0, 1000) == ESP_OK;
  }

  bool SetDigitalOutputs(uint8_t states)
  {
    GetLogger().Debug("SetDigitalOutputs: 0x%02X", states);
    return WriteReg8(Reg::OUTPUTS_CTL, states, 1000) == ESP_OK;
  }

  bool GetDigitalInput(int pin)
  {
    uint8_t state = 0;
    if (ReadReg8(REG_DIGITAL_INPUT_IO(pin), state, 1000) == ESP_OK)
    {
      GetLogger().Debug("GetDigitalInput Pin %d: %d", pin, state ? 1 : 0);
      return state != 0;
    }
    return false;
  }
};