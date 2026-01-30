#include "ni2c.hpp"
#include <iomanip>
#include <sstream>

// --- I2cBus ---
using namespace nix;

I2cBus::I2cBus(Logger& logger) : m_logger(logger), m_bus_handle(NULL) {
}

I2cBus::~I2cBus() {
    Deinit();
}

Logger& I2cBus::GetLogger() {
    return m_logger;
}

i2c_master_bus_handle_t I2cBus::GetHandle() const {
    return m_bus_handle;
}

esp_err_t I2cBus::Init(const I2cBusConfig& config) {
    if (m_bus_handle != NULL) {
        m_logger.Warning("Already initialized. Deinitializing first.");
        Deinit();
    }

    esp_err_t ret = i2c_new_master_bus(&config, &m_bus_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Initialized (Port: %d, SDA: %d, SCL: %d)", 
                     config.i2c_port, config.sda_io_num, config.scl_io_num);
    } else {
        m_logger.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2cBus::Deinit() {
    if (m_bus_handle != NULL) {
        esp_err_t ret = i2c_del_master_bus(m_bus_handle);
        if (ret == ESP_OK) {
            m_logger.Info("Deinitialized");
            m_bus_handle = NULL;
        } else {
            m_logger.Error("Failed to deinitialize: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t I2cBus::Reset() {
    if (m_bus_handle == NULL) {
        m_logger.Error("Cannot reset: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = i2c_master_bus_reset(m_bus_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Reset successfully");
    } else {
        m_logger.Error("Failed to reset: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2cBus::Probe(int addr) {
    return ProbeInternal(addr);
}

esp_err_t I2cBus::ProbeInternal(int addr) {
    if (m_bus_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Default timeout 50ms for probing
    return i2c_master_probe(m_bus_handle, static_cast<uint16_t>(addr), 50);
}

esp_err_t I2cBus::Scan(const std::vector<uint8_t>& addrs) {
    esp_err_t last_err = ESP_OK;
    bool found_any = false;
    for (uint8_t addr : addrs) {
        esp_err_t ret = ProbeInternal(addr);
        if (ret == ESP_OK) {
            found_any = true;
            m_logger.Info("Found device at address 0x%02X", addr);
        } else if (ret != ESP_ERR_NOT_FOUND) {
            last_err = ret;
            m_logger.Error("Failed to probe address 0x%02X: %s", addr, esp_err_to_name(ret));
        }
    }
    return (last_err != ESP_OK) ? last_err : (found_any ? ESP_OK : ESP_ERR_NOT_FOUND);
}

esp_err_t I2cBus::Scan(int start_addr, int end_addr) {
    if (start_addr > end_addr) {
        m_logger.Error("Invalid scan range: 0x%02X - 0x%02X", start_addr, end_addr);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (m_bus_handle == NULL) {
         m_logger.Error("Cannot scan: Not initialized");
         return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Scanning from 0x%02X to 0x%02X...", start_addr, end_addr);
    
    int found_count = 0;
    
    // 打印表头
    m_logger.Info("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    
    for (int i = 0; i < 128; i += 16) {
        // 检查当前行是否在扫描范围内
        if (i + 15 < start_addr || i > end_addr) {
             continue;
        }

        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << i << ":";
        
        for (int j = 0; j < 16; j++) {
            int addr = i + j;
            if (addr < start_addr || addr > end_addr) {
                ss << "   "; // 超出范围
            } else {
                esp_err_t ret = ProbeInternal(addr);
                if (ret == ESP_OK) {
                    ss << " " << std::hex << std::setw(2) << std::setfill('0') << addr;
                    found_count++;
                } else if (ret == ESP_ERR_TIMEOUT) {
                    ss << " UU";
                } else {
                    ss << " --";
                }
            }
        }
        m_logger.Info("%s", ss.str().c_str());
    }
    
    m_logger.Info("Scan complete. Found %d devices.", found_count);
    return (found_count > 0) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t I2cBus::Scan() {
    return Scan(0x00, 0x7F);
}

// --- I2cDevice ---

I2cDevice::I2cDevice(Logger& logger) : m_logger(logger), m_dev_handle(NULL) {
}

I2cDevice::~I2cDevice() {
    Deinit();
}

Logger& I2cDevice::GetLogger() {
    return m_logger;
}

esp_err_t I2cDevice::Init(const I2cBus& bus, const I2cDeviceConfig& config) {
    if (m_dev_handle != NULL) {
        m_logger.Warning("Device already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus.GetHandle() == NULL) {
        m_logger.Error("Bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2c_master_bus_add_device(bus.GetHandle(), &config, &m_dev_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Device initialized (Addr: 0x%02X)", config.device_address);
    } else {
        m_logger.Error("Failed to add device: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2cDevice::Deinit() {
    if (m_dev_handle != NULL) {
        esp_err_t ret = i2c_master_bus_rm_device(m_dev_handle);
        if (ret == ESP_OK) {
            m_logger.Info("Device deinitialized");
            m_dev_handle = NULL;
        } else {
            m_logger.Error("Failed to remove device: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t I2cDevice::Write(const std::vector<uint8_t>& data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2c_master_transmit(m_dev_handle, data.data(), data.size(), -1);
}

esp_err_t I2cDevice::Read(std::vector<uint8_t>& data, size_t len) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    data.resize(len);
    return i2c_master_receive(m_dev_handle, data.data(), len, -1);
}

esp_err_t I2cDevice::WriteRead(const std::vector<uint8_t>& write_data, std::vector<uint8_t>& read_data, size_t read_len) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    read_data.resize(read_len);
    return i2c_master_transmit_receive(m_dev_handle, write_data.data(), write_data.size(), read_data.data(), read_len, -1);
}

esp_err_t I2cDevice::WriteReg(uint8_t reg_addr, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.reserve(1 + data.size());
    buffer.push_back(reg_addr);
    buffer.insert(buffer.end(), data.begin(), data.end());
    return Write(buffer);
}

esp_err_t I2cDevice::ReadReg(uint8_t reg_addr, std::vector<uint8_t>& data, size_t len) {
    std::vector<uint8_t> reg = {reg_addr};
    return WriteRead(reg, data, len);
}

esp_err_t I2cDevice::WriteReg16(uint8_t reg_addr, const std::vector<uint16_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.reserve(1 + data.size() * 2);
    buffer.push_back(reg_addr);
    for (uint16_t val : data) {
        // Big Endian (Network Byte Order) is common for I2C registers
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back(val & 0xFF);
    }
    return Write(buffer);
}

esp_err_t I2cDevice::ReadReg16(uint8_t reg_addr, std::vector<uint16_t>& data, size_t len) {
    std::vector<uint8_t> read_buffer;
    esp_err_t ret = ReadReg(reg_addr, read_buffer, len * 2);
    if (ret != ESP_OK) {
        return ret;
    }
    
    data.clear();
    data.reserve(len);
    for (size_t i = 0; i < read_buffer.size(); i += 2) {
        uint16_t val = (static_cast<uint16_t>(read_buffer[i]) << 8) | read_buffer[i + 1];
        data.push_back(val);
    }
    return ESP_OK;
}

