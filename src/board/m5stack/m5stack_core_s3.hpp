#pragma once

#include "wrapper/logger.hpp"
#include "wrapper/soc.hpp"
#include "wrapper/i2c.hpp"

class M5StackCoreS3 
{
private:
  bool initialized_ = false;

  M5StackCoreS3() = default;
  ~M5StackCoreS3() = default;
  M5StackCoreS3(const M5StackCoreS3&) = delete;
  M5StackCoreS3& operator=(const M5StackCoreS3&) = delete;
  M5StackCoreS3(M5StackCoreS3&&) = delete;
  M5StackCoreS3& operator=(M5StackCoreS3&&) = delete;

public:
  static M5StackCoreS3& GetInstance()
  {
    static M5StackCoreS3 instance;
    return instance;
  }
  bool InitSocAbility(bool nvs);
  bool InitBus(bool i2c, bool spi, bool i2s);
  bool InitDevice(bool power, bool audio, bool display, bool touch);
  bool InitMiddleware(bool lvgl);
  bool Init();

  wrapper::I2cBus& GetI2cBus1();
};