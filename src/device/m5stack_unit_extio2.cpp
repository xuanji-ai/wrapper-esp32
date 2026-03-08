  #include "m5stack_unit_extio2.hpp"
  
namespace wrapper
{

  bool UnitExtio2::setPwmDutyCycle(uint8_t pin, uint8_t duty) {
    uint8_t reg = pin + EXTIO2_PWM_DUTY_CYCLE_REG;
    return WriteReg8(reg, duty, 1000);
  }

  bool UnitExtio2::setPwmFrequency(uint8_t pin, uint8_t freq) {
    uint8_t reg = pin + EXTIO2_PWM_FREQUENCY_REG;
    return WriteReg8(reg, freq, 1000);
  }

  bool UnitExtio2::Init(const I2cBus &bus)
  {
    I2cDeviceConfig cfg = I2cDeviceConfig(I2C_ADDR_DEFAULT, I2C_SPEED_HZ);
    return I2cDevice::Init(bus, cfg) == true;
  }

  UnitExtio2::Mode UnitExtio2::GetMode(int pin)
  {
    uint8_t mode = 0xFF;
    if (ReadReg8(REG_MODE_IO(pin), mode, 1000) == true)
    {
      return static_cast<Mode>(mode);
    }
    return static_cast<Mode>(0xFF);
  }

  bool UnitExtio2::SetMode(int pin, Mode mode)
  {

    bool result = WriteReg8(REG_MODE_IO(pin), static_cast<uint8_t>(mode), 1000);
    GetLogger().Info("SetMode Pin %d: %d. result: %s", pin, static_cast<uint8_t>(mode), result ? "Success" : "Failure");
    return result;
  }

  bool UnitExtio2::SetModeAll(Mode mode)
  {
    GetLogger().Info("SetMode All: %d", static_cast<uint8_t>(mode));
    bool success = true;
    for (int pin = 0; pin < 8; pin++)
    {
      success = WriteReg8(REG_MODE_IO(pin), static_cast<uint8_t>(mode), 1000) && success;
    }
    return success;
  }

  bool UnitExtio2::SetDigitalOutput(int pin, bool state)
  {
    GetLogger().Debug("SetDigitalOutput: %d", state ? 1 : 0);
    return WriteReg8(REG_OUTPUT_CTL_IO(pin), state ? 1 : 0, 1000) == true;
  }

  bool UnitExtio2::SetDigitalOutputs(uint8_t states)
  {
    GetLogger().Debug("SetDigitalOutputs: 0x%02X", states);
    return WriteReg8(Reg::OUTPUTS_CTL, states, 1000) == true;
  }

  int UnitExtio2::GetDigitalInput(int pin)
  {
    uint8_t state = 0;
    if (ReadReg8(REG_DIGITAL_INPUT_IO(pin), state, 1000) == true)
    {
      GetLogger().Debug("GetDigitalInput Pin %d: %d", pin, state ? 1 : 0);
      return state ? 1 : 0;
    }
    return -1;
  }  
}