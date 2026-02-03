#pragma once
#include "wrapper/i2c.hpp"

class Ip5306 : public wrapper::I2cDevice
{
public:
    static constexpr uint8_t I2C_ADDR_DEFAULT = 0x75;
    static constexpr uint32_t I2C_SPEED_HZ = 100000;

    static constexpr uint8_t REG_SYS_CTL0 = 0x00;
    static constexpr uint8_t REG_SYS_CTL1 = 0x01;
    static constexpr uint8_t REG_SYS_CTL2 = 0x02;

    static constexpr uint8_t REG_CHARGER_CTL0 = 0x20;
    static constexpr uint8_t REG_CHARGER_CTL1 = 0x21;
    static constexpr uint8_t REG_CHARGER_CTL2 = 0x22;
    static constexpr uint8_t REG_CHARGER_CTL3 = 0x23;

    static constexpr uint8_t REG_CHG_DIG_CTL0 = 0x24;

    static constexpr uint8_t REG_READ0 = 0x70;
    static constexpr uint8_t REG_READ0_BIT_CHARGE_EN = 3;  // 充电使能标志
    
    static constexpr uint8_t REG_READ1 = 0x71;
    static constexpr uint8_t REG_READ1_BIT_CHARGE_STATUS = 3;

    static constexpr uint8_t REG_READ2 = 0x72;
    static constexpr uint8_t REG_READ3 = 0x77;

    // Charger voltage cutoff settings (REG_CHARGER_CTL0 bits 1:0)
    enum class ChargerVoltage : uint8_t
    {
        V_4_14_4_26_4_305_4_35 = 0b00,  // 建议使用
        V_4_17_4_275_4_32_4_365 = 0b01, // 建议使用
        V_4_185_4_29_4_335_4_38 = 0b10, // 默认值
        V_4_2_4_305_4_35_4_395 = 0b11
    };

    Ip5306(wrapper::Logger &logger) : I2cDevice(logger)
    {
    }

    ~Ip5306() = default;

    bool Init(const wrapper::I2cBus &bus)
    {
        wrapper::I2cDeviceConfig cfg = wrapper::I2cDeviceConfig(I2C_ADDR_DEFAULT, I2C_SPEED_HZ);
        return I2cDevice::Init(bus, cfg) == ESP_OK;
    }

    bool GetChargingStatus()
    {
        uint8_t reg_value = 0;
        esp_err_t err = ReadReg8(REG_READ0, reg_value, -1);
        if (err != ESP_OK)
        {
            GetLogger().Error("Failed to read REG_READ0: %s", esp_err_to_name(err));
            return false;
        }
        return (reg_value & (1 << REG_READ0_BIT_CHARGE_EN)) != 0;
    }

    bool SetChargerVoltage(ChargerVoltage voltage)
    {
        uint8_t reg_value = 0;
        esp_err_t err = ReadReg8(REG_CHARGER_CTL0, reg_value, 1000);
        if (err != ESP_OK)
        {
            GetLogger().Error("Failed to read REG_CHARGER_CTL0: %s", esp_err_to_name(err));
            return false;
        }

        // 清除 bits 1:0，然后设置新值
        reg_value = (reg_value & 0xFC) | static_cast<uint8_t>(voltage);
        
        err = WriteReg8(REG_CHARGER_CTL0, reg_value, 1000);
        if (err != ESP_OK)
        {
            GetLogger().Error("Failed to write REG_CHARGER_CTL0: %s", esp_err_to_name(err));
            return false;
        }

        GetLogger().Info("Charger voltage set to: 0x%02X", static_cast<uint8_t>(voltage));
        return true;
    }

    ChargerVoltage GetChargerVoltage()
    {
        uint8_t reg_value = 0;
        esp_err_t err = ReadReg8(REG_CHARGER_CTL0, reg_value, 1000);
        if (err != ESP_OK)
        {
            GetLogger().Error("Failed to read REG_CHARGER_CTL0: %s", esp_err_to_name(err));
            return ChargerVoltage::V_4_185_4_29_4_335_4_38; // 返回默认值
        }

        return static_cast<ChargerVoltage>(reg_value & 0x03);
    }

};