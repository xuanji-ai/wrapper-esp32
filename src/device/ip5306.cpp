#include "ip5306.hpp"

namespace wrapper
{

  bool Ip5306::Init(const I2cBus &bus)
  {
    I2cDeviceConfig cfg = I2cDeviceConfig(I2C_ADDR_DEFAULT, I2C_SPEED_HZ);
    return I2cDevice::Init(bus, cfg);
  }

  bool Ip5306::GetChargingStatus()
  {
    bool charging = false;
    if (ReadRegBit(REG_READ1, REG_READ1_BIT_CHARGE_STATUS, charging, 1000))
    {
      return charging;
    }
    return false;
  }

  bool Ip5306::SetChargerVoltage(ChargerVoltage voltage)
  {
    uint8_t value = static_cast<uint8_t>(voltage) & 0x03;
    return WriteRegBits(REG_CHARGER_CTL0, 0x03, value, 1000);
  }

  Ip5306::ChargerVoltage Ip5306::GetChargerVoltage()
  {
    uint8_t value = 0;
    if (ReadRegBits(REG_CHARGER_CTL0, 0x03, value, 1000))
    {
      return static_cast<ChargerVoltage>(value & 0x03);
    }
    return ChargerVoltage::V_4_185_4_29_4_335_4_38;
  }

} // namespace wrapper
