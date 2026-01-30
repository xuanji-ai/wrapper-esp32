#pragma once

#include "driver/i2c_master.h"

#include "nlogger.hpp"
#include <vector>
#include <functional>

namespace nix
{

struct I2cBusConfig : public i2c_master_bus_config_t
{
public:
    I2cBusConfig(i2c_port_num_t port, 
                        gpio_num_t sda, 
                        gpio_num_t scl,
                        i2c_clock_source_t clk_src,
                        uint8_t glitch_ignore,
                        int intr_prio,
                        size_t queue_depth,
                        bool enable_pullup,
                        bool enable_pd) : i2c_master_bus_config_t{}
    {
        i2c_port = port;
        sda_io_num = sda;
        scl_io_num = scl;
        clk_source = clk_src;
        glitch_ignore_cnt = glitch_ignore;
        intr_priority = intr_prio;
        trans_queue_depth = queue_depth;
        flags.enable_internal_pullup = enable_pullup;
        flags.allow_pd = enable_pd;
    }
};

class I2cBus
{
    Logger& m_logger;
    i2c_master_bus_handle_t m_bus_handle;
    
    // 私有方法：实际执行Probe的逻辑
    esp_err_t ProbeInternal(int addr);

public:
    I2cBus(Logger& logger);
    ~I2cBus();
    Logger& GetLogger();
    esp_err_t Init(const I2cBusConfig& config);
    esp_err_t Deinit();
    esp_err_t Reset();
    i2c_master_bus_handle_t GetHandle() const;
    
    esp_err_t Probe(int addr);
    
    template<typename... Args>
    esp_err_t Scan(Args... args) {
        std::vector<uint8_t> addrs = {static_cast<uint8_t>(args)...};
        return Scan(addrs);
    }
    
    esp_err_t Scan(const std::vector<uint8_t>& addrs);
    esp_err_t Scan(int start_addr, int end_addr);
    esp_err_t Scan();
};

struct I2cDeviceConfig : public i2c_device_config_t 
{
    I2cDeviceConfig(uint8_t addr, uint32_t speed_hz) : i2c_device_config_t{}
    {
        dev_addr_length = I2C_ADDR_BIT_LEN_7;
        device_address = addr;
        scl_speed_hz = speed_hz;
        scl_wait_us = 0;
        flags.disable_ack_check = 0;
    }
};

class I2cDevice
{
protected:
    Logger& m_logger;
    i2c_master_dev_handle_t m_dev_handle;
    
public:
    I2cDevice(Logger& logger);
    ~I2cDevice();
    Logger& GetLogger();
    esp_err_t Init(const I2cBus& bus, const I2cDeviceConfig& config);
    esp_err_t Deinit();

    esp_err_t Write(const std::vector<uint8_t>& data);
    esp_err_t Read(std::vector<uint8_t>& data, size_t len);
    esp_err_t WriteRead(const std::vector<uint8_t>& write_data, std::vector<uint8_t>& read_data, size_t read_len);
    
    // Register operations
    esp_err_t WriteReg(uint8_t reg_addr, const std::vector<uint8_t>& data);
    esp_err_t ReadReg(uint8_t reg_addr, std::vector<uint8_t>& data, size_t len);
    esp_err_t WriteReg16(uint8_t reg_addr, const std::vector<uint16_t>& data);
    esp_err_t ReadReg16(uint8_t reg_addr, std::vector<uint16_t>& data, size_t len);
};



}