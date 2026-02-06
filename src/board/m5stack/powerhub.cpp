#include "board/m5stack/powerhub.hpp"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <map>

namespace wrapper {

static const char* TAG = "PowerHub";

struct LedRegisterInfo {
    uint8_t colorStartReg;
    uint8_t brightnessReg;
};

struct VARegisterInfo {
    uint8_t voltageReg;
    uint8_t currentReg;
};

static const std::map<LedControl, LedRegisterInfo> ledRegisterMap = {
    {LedControl::USB_C, {0x60, 0x80}}, {LedControl::USB_A, {0x64, 0x81}},      {LedControl::UART, {0x68, 0x82}},    {LedControl::BUS, {0x6C, 0x83}},
    {LedControl::I2C, {0x70, 0x84}},   {LedControl::BAT_CHARGE, {0x74, 0x85}}, {LedControl::POWER_L, {0x78, 0x86}}, {LedControl::POWER_R, {0x7C, 0x87}}
};

static const std::map<VAMonitor, VARegisterInfo> VARegisterMap = {
    {VAMonitor::BAT, {0x30, 0x32}},       {VAMonitor::CAN_BUS, {0x34, 0x36}},
    {VAMonitor::RS485_BUS, {0x38, 0x3A}}, {VAMonitor::USB, {0x3C, 0x3E}},
    {VAMonitor::I2C, {0x40, 0x42}},       {VAMonitor::UART, {0x44, 0x46}}
};

PowerHub::PowerHub(Logger& logger) : I2cDevice(logger) {
}

esp_err_t PowerHub::Init(const I2cBus& bus, uint8_t addr) {
    I2cDeviceConfig config(addr, DEFAULT_SPEED);
    esp_err_t err = I2cDevice::Init(bus, config);
    if (err != ESP_OK) return err;

    // Configure Button GPIO (Select Key)
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << (int)ButtonKey::SELECT);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    return ESP_OK;
}

esp_err_t PowerHub::SetPowerState(PowerControl device, bool state) {
    return WriteReg8(REG_POWER_CTR + (uint8_t)device, state ? 1 : 0, -1);
}

esp_err_t PowerHub::GetPowerState(PowerControl device, bool& state) {
    uint8_t val;
    esp_err_t err = ReadReg8(REG_POWER_CTR + (uint8_t)device, val, -1);
    if (err == ESP_OK) {
        state = (val != 0);
    }
    return err;
}

esp_err_t PowerHub::SetUSBMode(USBMode mode) {
    return WriteReg8(REG_USB_MODE, (uint8_t)mode, -1);
}

esp_err_t PowerHub::GetUSBMode(USBMode& mode) {
    uint8_t val;
    esp_err_t err = ReadReg8(REG_USB_MODE, val, -1);
    if (err == ESP_OK) {
        mode = (USBMode)val;
    }
    return err;
}

esp_err_t PowerHub::SetBusConfig(const BusConfig& config) {
    std::vector<uint8_t> data(5);
    data[0] = config.voltage & 0xFF;
    data[1] = (config.voltage >> 8) & 0xFF;
    data[2] = config.currentLimit;
    data[3] = config.enable;
    data[4] = config.direction;
    return WriteRegBytes(REG_BUS_CFG, data, -1);
}

esp_err_t PowerHub::GetBusConfig(BusConfig& config) {
    std::vector<uint8_t> data(5);
    esp_err_t err = ReadRegBytes(REG_BUS_CFG, data, 5, -1);
    if (err == ESP_OK) {
        config.voltage = (data[1] << 8) | data[0];
        config.currentLimit = data[2];
        config.enable = data[3];
        config.direction = data[4];
    }
    return err;
}

esp_err_t PowerHub::GetDeviceVoltage(VAMonitor device, uint16_t& voltage) {
    if (VARegisterMap.find(device) == VARegisterMap.end()) return ESP_ERR_INVALID_ARG;
    uint8_t reg = VARegisterMap.at(device).voltageReg;
    std::vector<uint8_t> data(2);
    esp_err_t err = ReadRegBytes(reg, data, 2, -1);
    if (err == ESP_OK) {
        voltage = (uint16_t)((data[1] << 8) | data[0]);
    }
    return err;
}

esp_err_t PowerHub::GetDeviceCurrent(VAMonitor device, int16_t& current) {
    if (VARegisterMap.find(device) == VARegisterMap.end()) return ESP_ERR_INVALID_ARG;
    uint8_t reg = VARegisterMap.at(device).currentReg;
    std::vector<uint8_t> data(2);
    esp_err_t err = ReadRegBytes(reg, data, 2, -1);
    if (err == ESP_OK) {
        current = (int16_t)((data[1] << 8) | data[0]);
    }
    return err;
}

esp_err_t PowerHub::GetChargeStatus(uint8_t& status) {
    return ReadReg8(REG_CHG_STA, status, -1);
}

esp_err_t PowerHub::GetPowerSupplyStatus(uint8_t& status) {
    return ReadReg8(REG_PWR_SUP, status, -1);
}

esp_err_t PowerHub::SetLEDColor(LedControl device, uint32_t color) {
    if (ledRegisterMap.find(device) == ledRegisterMap.end()) return ESP_ERR_INVALID_ARG;
    uint8_t reg = ledRegisterMap.at(device).colorStartReg;
    std::vector<uint8_t> data(3);
    data[0] = color & 0xFF;
    data[1] = (color >> 8) & 0xFF;
    data[2] = (color >> 16) & 0xFF;
    return WriteRegBytes(reg, data, -1);
}

esp_err_t PowerHub::UpdateLedColors(const std::vector<uint32_t>& colors) {
    if (colors.size() > 8) return ESP_ERR_INVALID_ARG;
    std::vector<uint8_t> data(32, 0);
    for (size_t i = 0; i < colors.size(); ++i) {
        data[i * 4 + 0] = colors[i] & 0xFF;
        data[i * 4 + 1] = (colors[i] >> 8) & 0xFF;
        data[i * 4 + 2] = (colors[i] >> 16) & 0xFF;
        data[i * 4 + 3] = 0x00;
    }
    // Assume start from first LED (USB_C)
    return WriteRegBytes(ledRegisterMap.at(LedControl::USB_C).colorStartReg, data, -1);
}

esp_err_t PowerHub::GetLEDColor(LedControl device, uint32_t& color) {
    if (ledRegisterMap.find(device) == ledRegisterMap.end()) return ESP_ERR_INVALID_ARG;
    uint8_t reg = ledRegisterMap.at(device).colorStartReg;
    std::vector<uint8_t> data(3);
    esp_err_t err = ReadRegBytes(reg, data, 3, -1);
    if (err == ESP_OK) {
        color = (data[2] << 16) | (data[1] << 8) | data[0];
    }
    return err;
}

esp_err_t PowerHub::SetLEDBrightness(LedControl device, uint8_t brightness) {
    if (ledRegisterMap.find(device) == ledRegisterMap.end()) return ESP_ERR_INVALID_ARG;
    return WriteReg8(ledRegisterMap.at(device).brightnessReg, brightness, -1);
}

esp_err_t PowerHub::GetLEDBrightness(LedControl device, uint8_t& brightness) {
    if (ledRegisterMap.find(device) == ledRegisterMap.end()) return ESP_ERR_INVALID_ARG;
    return ReadReg8(ledRegisterMap.at(device).brightnessReg, brightness, -1);
}

esp_err_t PowerHub::GetSC8721Config(SC8721Config& config) {
    uint8_t writeData = 1;
    esp_err_t err = WriteReg8(REG_SC8721_CFG, writeData, -1);
    if (err != ESP_OK) return err;

    const unsigned long timeout = 1000;
    unsigned long startTime = (unsigned long)(esp_timer_get_time() / 1000ULL);

    while ((unsigned long)(esp_timer_get_time() / 1000ULL) - startTime <= timeout) {
        err = ReadReg8(REG_SC8721_CFG, writeData, -1);
        if (err != ESP_OK) return err;

        if (writeData == 0) {
            std::vector<uint8_t> data(10);
            err = ReadRegBytes(REG_SC8721_CFG + 1, data, 10, -1);
            if (err == ESP_OK) {
                config.csoValue = data[0];
                config.slopeCompensation = data[1];
                config.outputVoltageSet = (data[2] << 8) | data[3];
                config.control = data[4];
                config.systemSetting = data[5];
                config.frequency = data[6];
                config.statusFlags = (data[8] << 8) | data[7];
            }
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return ESP_ERR_TIMEOUT;
}

bool PowerHub::GetButtonState(ButtonKey key) {
    if (key == ButtonKey::SELECT) {
        return gpio_get_level((gpio_num_t)key);
    } else {
        uint8_t data;
        if (ReadReg8(REG_BUTTON, data, -1) == ESP_OK) {
            return (data >> (int)key) & 0x01;
        }
        return false;
    }
}

esp_err_t PowerHub::SetWakeUpSource(WakeUpSource source, bool state) {
    return WriteReg8(REG_WAKEUP + (uint8_t)source, state ? 1 : 0, -1);
}

esp_err_t PowerHub::CheckWakeUpSource(WakeUpSource source, bool& state) {
    uint8_t val;
    esp_err_t err = ReadReg8(REG_WAKEUP + (uint8_t)source, val, -1);
    if (err == ESP_OK) {
        state = (val != 0);
    }
    return err;
}

esp_err_t PowerHub::SetRTCTime(const RtcTime& time) {
    std::vector<uint8_t> data(7);
    data[0] = time.sec;
    data[1] = time.min;
    data[2] = time.hour;
    data[3] = time.day;
    data[4] = time.mon;
    data[5] = time.year;
    uint8_t wday_map[] = {2, 4, 8, 10, 20, 40, 1}; // Mon -> Sun
    data[6] = (time.wday >= 1 && time.wday <= 7) ? wday_map[time.wday - 1] : 0;
    return WriteRegBytes(REG_RTC_TIME, data, -1);
}

esp_err_t PowerHub::GetRTCTime(RtcTime& time) {
    std::vector<uint8_t> data(7);
    esp_err_t err = ReadRegBytes(REG_RTC_TIME, data, 7, -1);
    if (err == ESP_OK) {
        time.sec = data[0];
        time.min = data[1];
        time.hour = data[2];
        time.day = data[3];
        time.mon = data[4];
        time.year = data[5];
        
        uint8_t wday_map[] = {1, 2, 4, 8, 10, 20, 40}; // Sun -> Sat (based on logic)
        // Note: original code mapping was a bit confusing, assuming standard 1-7 Mon-Sun
        time.wday = 0; 
        for (int i = 0; i < 7; i++) {
            if (data[6] == wday_map[i]) {
                time.wday = i + 1;
                break;
            }
        }
    }
    return err;
}

esp_err_t PowerHub::SetAlarmTime(const AlarmTime& time) {
    std::vector<uint8_t> data(3);
    data[0] = time.min;
    data[1] = time.hour;
    data[2] = time.day;
    return WriteRegBytes(REG_RTC_ALARM, data, -1);
}

esp_err_t PowerHub::GetAlarmTime(AlarmTime& time) {
    std::vector<uint8_t> data(3);
    esp_err_t err = ReadRegBytes(REG_RTC_ALARM, data, 3, -1);
    if (err == ESP_OK) {
        time.min = data[0];
        time.hour = data[1];
        time.day = data[2];
    }
    return err;
}

esp_err_t PowerHub::SetAlarmState(bool state) {
    return WriteReg8(REG_RTC_ALARM_CTR, state ? 1 : 0, -1);
}

esp_err_t PowerHub::GetAlarmState(bool& state) {
    uint8_t val;
    esp_err_t err = ReadReg8(REG_RTC_ALARM_CTR, val, -1);
    if (err == ESP_OK) {
        state = (val != 0);
    }
    return err;
}

esp_err_t PowerHub::PowerOff() {
    return WriteReg8(REG_POWER_OFF, 1, -1);
}

esp_err_t PowerHub::GetBootloaderVersion(uint8_t& version) {
    return ReadReg8(REG_BL_VERSION, version, -1);
}

esp_err_t PowerHub::GetFirmwareVersion(uint8_t& version) {
    return ReadReg8(REG_FW_VERSION, version, -1);
}

esp_err_t PowerHub::SetI2CAddress(uint8_t newAddr) {
    return WriteReg8(REG_I2C_ADDR_CFG, newAddr, -1);
}

esp_err_t PowerHub::GetI2CAddress(uint8_t& addr) {
    return ReadReg8(REG_I2C_ADDR_CFG, addr, -1);
}

esp_err_t PowerHub::SaveConfig() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("powerhub", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    PersistentConfig config;
    // Get current state to save
    GetBusConfig(config.busConfig);
    GetUSBMode(config.usbMode);
    // Note: We are saving the *current* state of registers as the config.
    // In a real application, we might want to save *desired* config and apply it on boot.
    // For this implementation, we save what is currently set.

    err = nvs_set_blob(my_handle, "config", &config, sizeof(PersistentConfig));
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
    }
    nvs_close(my_handle);
    return err;
}

esp_err_t PowerHub::LoadConfig() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("powerhub", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    PersistentConfig config;
    size_t size = sizeof(PersistentConfig);
    err = nvs_get_blob(my_handle, "config", &config, &size);
    
    if (err == ESP_OK && size == sizeof(PersistentConfig)) {
        // Apply loaded config
        SetBusConfig(config.busConfig);
        SetUSBMode(config.usbMode);
    }
    nvs_close(my_handle);
    return err;
}

} // namespace wrapper
