#pragma once
#include <vector>
#include "driver/i2c_master.h"
#include "wrapper/logger.hpp"

namespace wrapper
{

  struct I2cBusConfig : public i2c_master_bus_config_t
  {
  public:
    I2cBusConfig(i2c_port_t port,
                 gpio_num_t sda,
                 gpio_num_t scl,
                 i2c_clock_source_t clk_src,
                 uint8_t glitch_ignore,
                 int intr_prio,
                 size_t queue_depth,
                 bool enable_pullup,
                 bool enable_pd) : i2c_master_bus_config_t{}
    {
      i2c_port = (i2c_port_num_t)port;
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
    Logger &logger_;
    i2c_port_t port_;
    i2c_master_bus_handle_t bus_handle_;

    // 私有方法：实际执行Probe的逻辑
    esp_err_t ProbeInternal(int addr);

  public:
    I2cBus(Logger &logger);
    ~I2cBus();
    
    esp_err_t Init(const I2cBusConfig &config);
    esp_err_t Deinit();
    esp_err_t Reset();

    Logger &GetLogger();
    i2c_master_bus_handle_t GetHandle() const;
    i2c_port_t GetPort() const;

    esp_err_t Probe(int addr);

    template <typename... Args>
    esp_err_t Scan(Args... args)
    {
      std::vector<uint8_t> addrs = {static_cast<uint8_t>(args)...};
      return Scan(addrs);
    }

    esp_err_t Scan(const std::vector<uint8_t> &addrs);
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
    Logger &logger_;
    i2c_master_dev_handle_t dev_handle_;

  public:
    I2cDevice(Logger &logger);
    ~I2cDevice();
    Logger &GetLogger();
    esp_err_t Init(const I2cBus &bus, const I2cDeviceConfig &config);
    esp_err_t Deinit();

    inline esp_err_t WriteBytes(const uint8_t *data, size_t length, int timeout_ms)
    {
      return i2c_master_transmit(dev_handle_, data, length, timeout_ms);
    }

    inline esp_err_t ReadBytes(uint8_t *data, size_t length, int timeout_ms)
    {
      return i2c_master_receive(dev_handle_, data, length, timeout_ms);
    }

    inline esp_err_t WriteReadBytes(
        const uint8_t *write_data, size_t write_length, uint8_t *read_data, size_t read_length, int timeout_ms)
    {
      return i2c_master_transmit_receive(dev_handle_, write_data, write_length, read_data, read_length, timeout_ms);
    }

    esp_err_t WriteByte(uint8_t data, int timeout_ms);
    esp_err_t ReadByte(uint8_t &data, int timeout_ms);

    esp_err_t WriteBytes(const std::vector<uint8_t> &data, int timeout_ms);
    esp_err_t ReadBytes(std::vector<uint8_t> &data, size_t len, int timeout_ms);
    esp_err_t WriteReadBytes(const std::vector<uint8_t> &write_data, std::vector<uint8_t> &read_data, size_t read_len, int timeout_ms);

    esp_err_t WriteRegBytes(uint8_t reg_addr, const std::vector<uint8_t> &data, int timeout_ms);
    esp_err_t ReadRegBytes(uint8_t reg_addr, std::vector<uint8_t> &data, size_t len, int timeout_ms);

    esp_err_t WriteReg8(uint8_t reg_addr, uint8_t data, int timeout_ms);
    esp_err_t ReadReg8(uint8_t reg_addr, uint8_t &data, int timeout_ms);
    esp_err_t WriteReg16(uint8_t reg_addr, uint16_t data, int timeout_ms);
    esp_err_t ReadReg16(uint8_t reg_addr, uint16_t &data, int timeout_ms);
    esp_err_t WriteReg32(uint8_t reg_addr, uint32_t data, int timeout_ms);
    esp_err_t ReadReg32(uint8_t reg_addr, uint32_t &data, int timeout_ms);

    esp_err_t WriteRegBit(uint8_t reg_addr, uint8_t bit, bool value, int timeout_ms);
    esp_err_t ReadRegBit(uint8_t reg_addr, uint8_t bit, bool &value, int timeout_ms);
    esp_err_t WriteRegBits(uint8_t reg_addr, uint8_t mask, uint8_t value, int timeout_ms);
    esp_err_t ReadRegBits(uint8_t reg_addr, uint8_t mask, uint8_t &value, int timeout_ms);
  };

}