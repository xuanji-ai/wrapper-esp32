#pragma once
#include "wrapper/i2c.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace wrapper {

enum class PowerControl { LED, USB, I2C, UART, BUS, VAMeter, Charge };
enum class LedControl { NONE = -1, USB_C, USB_A, UART, BUS, I2C, BAT_CHARGE, POWER_L, POWER_R };
enum class VAMonitor { BAT, CAN_BUS, RS485_BUS, USB, I2C, UART };
enum class ButtonKey { OK, KEY2, SELECT = 11 };
enum class WakeUpSource { RTC_ALARM, VIN, BUTTON };
enum class USBMode { SLAVE_MODE, HOST_MODE_TPYE_A, HOST_MODE_TPYE_C };

struct BusConfig {
    uint16_t voltage;
    uint8_t currentLimit;
    uint8_t enable;
    uint8_t direction;
};

struct SC8721Config {
    uint8_t csoValue;
    uint8_t slopeCompensation;
    uint16_t outputVoltageSet;
    uint8_t control;
    uint8_t systemSetting;
    uint8_t frequency;
    uint16_t statusFlags;
};

struct RtcTime {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t mon;
    uint8_t year;
    uint8_t wday;
};

struct AlarmTime {
    uint8_t min;
    uint8_t hour;
    uint8_t day;
};

class PowerHub : public I2cDevice {
public:
    static constexpr uint8_t DEFAULT_ADDR = 0x50;
    static constexpr uint32_t DEFAULT_SPEED = 100000;

    PowerHub(Logger& logger);
    virtual ~PowerHub() = default;

    esp_err_t Init(const I2cBus& bus, uint8_t addr = DEFAULT_ADDR);

    // Power Control
    esp_err_t SetPowerState(PowerControl device, bool state);
    esp_err_t GetPowerState(PowerControl device, bool& state);

    // USB Mode
    esp_err_t SetUSBMode(USBMode mode);
    esp_err_t GetUSBMode(USBMode& mode);

    // Bus Config
    esp_err_t SetBusConfig(const BusConfig& config);
    esp_err_t GetBusConfig(BusConfig& config);

    // VA Monitor
    esp_err_t GetDeviceVoltage(VAMonitor device, uint16_t& voltage);
    esp_err_t GetDeviceCurrent(VAMonitor device, int16_t& current);
    
    // Status
    esp_err_t GetChargeStatus(uint8_t& status);
    esp_err_t GetPowerSupplyStatus(uint8_t& status);

    // LED
    esp_err_t SetLEDColor(LedControl device, uint32_t color);
    esp_err_t UpdateLedColors(const std::vector<uint32_t>& colors);
    esp_err_t GetLEDColor(LedControl device, uint32_t& color);
    esp_err_t SetLEDBrightness(LedControl device, uint8_t brightness);
    esp_err_t GetLEDBrightness(LedControl device, uint8_t& brightness);

    // SC8721
    esp_err_t GetSC8721Config(SC8721Config& config);

    // Button
    bool GetButtonState(ButtonKey key);

    // Wakeup
    esp_err_t SetWakeUpSource(WakeUpSource source, bool state);
    esp_err_t CheckWakeUpSource(WakeUpSource source, bool& state);

    // RTC
    esp_err_t SetRTCTime(const RtcTime& time);
    esp_err_t GetRTCTime(RtcTime& time);
    esp_err_t SetAlarmTime(const AlarmTime& time);
    esp_err_t GetAlarmTime(AlarmTime& time);
    esp_err_t SetAlarmState(bool state);
    esp_err_t GetAlarmState(bool& state);

    // System
    esp_err_t PowerOff();
    esp_err_t GetBootloaderVersion(uint8_t& version);
    esp_err_t GetFirmwareVersion(uint8_t& version);
    esp_err_t SetI2CAddress(uint8_t newAddr);
    esp_err_t GetI2CAddress(uint8_t& addr);

    // Storage
    esp_err_t SaveConfig();
    esp_err_t LoadConfig();

private:
    // Registers
    static constexpr uint8_t REG_POWER_CTR = 0x00;
    static constexpr uint8_t REG_USB_MODE = 0x10;
    static constexpr uint8_t REG_BUS_CFG = 0x20;
    static constexpr uint8_t REG_CHG_STA = 0x50;
    static constexpr uint8_t REG_PWR_SUP = 0x51;
    static constexpr uint8_t REG_SC8721_CFG = 0x90;
    static constexpr uint8_t REG_BUTTON = 0xA0;
    static constexpr uint8_t REG_WAKEUP = 0xB0;
    static constexpr uint8_t REG_RTC_TIME = 0xC0;
    static constexpr uint8_t REG_RTC_ALARM = 0xD0;
    static constexpr uint8_t REG_RTC_ALARM_CTR = 0xD3;
    static constexpr uint8_t REG_POWER_OFF = 0xE0;
    static constexpr uint8_t REG_BL_VERSION = 0xFC;
    static constexpr uint8_t REG_FW_VERSION = 0xFE;
    static constexpr uint8_t REG_I2C_ADDR_CFG = 0xFF;

    struct PersistentConfig {
        BusConfig busConfig;
        USBMode usbMode;
        // Add more persistent fields if needed
    };
};

} // namespace wrapper
