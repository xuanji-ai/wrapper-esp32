#include <tuple>

#include "esp_lcd_ili9341.h"
#include "esp_lcd_touch_ft5x06.h"

#include "wrapper/logger.hpp"
#include "wrapper/soc.hpp"
#include "wrapper/nvs.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"
#include "wrapper/i2s.hpp"
#include "wrapper/display.hpp"
#include "wrapper/touch.hpp"
#include "wrapper/lvgl.hpp"
#include "wrapper/audio.hpp"

#include "device/axp2101.hpp"
#include "device/aw9523.hpp"

#include "m5stack_core_s3.hpp"

using namespace wrapper;

I2sBusConfig bus_config(I2S_NUM_0, I2S_ROLE_MASTER, 6, 240, true, false, 0);

I2sChanStdConfig tx_config(
    16000,
    I2S_CLK_SRC_DEFAULT,
    0,
    I2S_MCLK_MULTIPLE_256,
    I2S_DATA_BIT_WIDTH_16BIT,
    I2S_SLOT_BIT_WIDTH_AUTO,
    I2S_SLOT_MODE_STEREO,
    I2S_STD_SLOT_BOTH,
    I2S_DATA_BIT_WIDTH_16BIT,
    false,
    true,
    true,
    false,
    false,
    GPIO_NUM_0,
    GPIO_NUM_34,
    GPIO_NUM_33,
    GPIO_NUM_13,
    GPIO_NUM_NC,
    false,
    false,
    false);

I2SChanTdmConfig rx_config(
    16000,
    I2S_CLK_SRC_DEFAULT,
    0,
    I2S_MCLK_MULTIPLE_256,
    8,
    I2S_DATA_BIT_WIDTH_16BIT,
    I2S_SLOT_BIT_WIDTH_AUTO,
    I2S_SLOT_MODE_STEREO,
    (i2s_tdm_slot_mask_t)(I2S_TDM_SLOT0 | I2S_TDM_SLOT1 | I2S_TDM_SLOT2 | I2S_TDM_SLOT3),
    I2S_TDM_AUTO_WS_WIDTH,
    false,
    true,
    false,
    false,
    false,
    false,
    I2S_TDM_AUTO_SLOT_NUM,
    GPIO_NUM_0,
    GPIO_NUM_34,
    GPIO_NUM_33,
    GPIO_NUM_NC,
    GPIO_NUM_NC,
    false,
    false,
    false);

I2cBusConfig i2c_bus_config(
    I2C_NUM_1,
    GPIO_NUM_12,
    GPIO_NUM_11,
    I2C_CLK_SRC_DEFAULT,
    7,
    0,
    0,
    true,
    false);

SpiBusConfig spi_bus_config(
    SPI3_HOST,
    GPIO_NUM_37,
    GPIO_NUM_NC,
    GPIO_NUM_36,
    320 * 240 * sizeof(uint16_t),
    SPI_DMA_CH_AUTO,
    SPICOMMON_BUSFLAG_MASTER);

I2cTouchConfig ft5x06_config(
    0x38,        // dev_addr
    400000,      // scl_speed_hz (400kHz)
    320,         // x_max
    240,         // y_max
    GPIO_NUM_NC, // rst_gpio
    GPIO_NUM_NC  // int_gpio
);

SpiLcdConfig spi_lcd_config(
    GPIO_NUM_3,                // cs_gpio
    GPIO_NUM_35,               // dc_gpio
    40 * 1000 * 1000,          // clock_speed_hz (40MHz)
    2,                         // spi_mode
    8,                         // lcd_cmd_bits
    8,                         // lcd_param_bits
    GPIO_NUM_NC,               // reset_gpio
    LCD_RGB_ELEMENT_ORDER_BGR, // rgb_order
    LCD_RGB_DATA_ENDIAN_BIG,   // data_endian
    16,                        // bits_per_pixel
    false,                     // reset_active_high
    NULL                       // vendor_conf
);

I2cDeviceConfig axp2101_config(Axp2101::DEFAULT_ADDR, Axp2101::DEFAULT_SPEED);
I2cDeviceConfig aw9523_config(Aw9523::DEFAULT_ADDR, Aw9523::DEFAULT_SPEED);

LvglPortConfig lvgl_port_config(
    5,                                   // task_priority
    8192,                                // task_stack
    APP_CPU_NUM,                         // task_affinity
    500,                                 // task_max_sleep_ms
    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, // task_stack_caps
    20                                   // timer_period_ms
);

LvglDisplayConfig lvgl_display_config(
    320 * 240,              // buffer_size
    true,                   // double_buffer
    0,                      // trans_size
    320,                    // hres
    240,                    // vres
    false,                  // monochrome
    LV_COLOR_FORMAT_RGB565, // color_format
    false,                  // swap_xy
    false,                  // mirror_x
    false,                  // mirror_y
    true,                   // buff_dma
    true,                   // buff_spiram
    false,                  // sw_rotate
    true,                   // swap_bytes
    false,                  // full_refresh
    false                   // direct_mode
);

LvglTouchConfig lvgl_touch_config(0.0f, 0.0f);

Logger logger_nvs_ability("M5StackCoreS3", "nvs");
Logger logger_i2c_bus("M5StackCoreS3", "i2c", "bus");
Logger logger_spi_bus("M5StackCoreS3", "spi", "bus");
Logger logger_i2s_bus("M5StackCoreS3", "i2s", "bus");

Logger logger_axp2101("M5StackCoreS3", "i2c", "axp2101");
Logger logger_aw9523("M5StackCoreS3", "i2c", "aw9523");
Logger logger_ft5x06("M5StackCoreS3", "i2c", "ft5x06");
Logger logger_ili9341("M5StackCoreS3", "spi", "ili9341");
Logger logger_lvgl("M5StackCoreS3", "lvgl", "port");
Logger logger_audio_codec("M5StackCoreS3", "audio", "codec");

Nvs nvs_ability(logger_nvs_ability);
I2cBus i2c_bus(logger_i2c_bus);
SpiBus spi_bus(logger_spi_bus);
I2sBus i2s_bus(logger_i2s_bus);

Axp2101 axp2101(logger_axp2101);
Aw9523 aw9523(logger_aw9523);
I2cTouch ft5x06(logger_ft5x06);
SpiLcd ili9341(logger_ili9341);
LvglPort lvgl_port(logger_lvgl);

AudioCodec audio_codec(logger_audio_codec);
std::function<esp_err_t()> spk_codec_new_func = []() -> esp_err_t
{
  aw88298_codec_cfg_t aw88298_cfg = {
      .ctrl_if = audio_codec.GetSpeakerCtrlInterface(),
      .gpio_if = audio_codec.GetGpioInterface(),
      .reset_pin = GPIO_NUM_NC,
      .hw_gain = {
          .pa_voltage = 5.0f,
          .codec_dac_voltage = 3.3f,
          .pa_gain = 1.f},
  };
  const audio_codec_if_t *codec_if = aw88298_codec_new(&aw88298_cfg);
  if (codec_if == NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }
  audio_codec.SetSpeakerCodecInterface(codec_if);

  esp_codec_dev_handle_t codec_dev_handle = nullptr;
  esp_codec_dev_cfg_t codec_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = codec_if,
      .data_if = audio_codec.GetDataInterface(),
  };
  codec_dev_handle = esp_codec_dev_new(&codec_dev_cfg);
  if (codec_dev_handle == nullptr)
  {
    return ESP_ERR_INVALID_STATE;
  }
  audio_codec.SetSpeakerCodecDeviceHandle(codec_dev_handle);
  return ESP_OK;
};
std::function<esp_err_t()> mic_codec_new_func = []() -> esp_err_t
{
  es7210_codec_cfg_t codec_cfg = {
      .ctrl_if = audio_codec.GetMicrophoneCtrlInterface(),
      .master_mode = false,
      .mic_selected = 0,
      .mclk_src = ES7210_MCLK_FROM_PAD,
      .mclk_div = 0,
  };
  const audio_codec_if_t *codec_if = es7210_codec_new(&codec_cfg);
  if (codec_if == NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }
  audio_codec.SetMicrophoneCodecInterface(codec_if);
  esp_codec_dev_handle_t codec_dev_handle = nullptr;
  esp_codec_dev_cfg_t codec_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = codec_if,
      .data_if = audio_codec.GetDataInterface(),
  };
  codec_dev_handle = esp_codec_dev_new(&codec_dev_cfg);
  if (codec_dev_handle == nullptr)
  {
    return ESP_ERR_INVALID_STATE;
  }
  audio_codec.SetMicrophoneCodecDeviceHandle(codec_dev_handle);
  return ESP_OK;
};

bool M5StackCoreS3::InitSocAbility(bool nvs)
{
    esp_err_t err = ESP_OK;
    if (nvs) {
      err = nvs_ability.Init();
      if (err != ESP_OK) return false;
    }
    return true;
}

bool M5StackCoreS3::InitBus(bool i2c, bool spi, bool i2s)
{
  esp_err_t err = ESP_OK;
  if (i2c) {
      err = i2c_bus.Init(i2c_bus_config);
      if (err != ESP_OK) return false;
  }
  
  if (spi) {
      err = spi_bus.Init(spi_bus_config);
      if (err != ESP_OK) return false;
  }
  
  if (i2s) {
      err = i2s_bus.Init(bus_config);
      if (err != ESP_OK) return false;
      err = i2s_bus.ConfigureTxChannel(tx_config);
      if (err != ESP_OK) return false;
      err = i2s_bus.ConfigureRxChannel(rx_config);
      if (err != ESP_OK) return false;
  }
  return true;
}

bool M5StackCoreS3::InitDevice(bool power, bool audio, bool display, bool touch)
{
  esp_err_t err = ESP_OK;
  
  if (power) {
      // AXP2101
      err = axp2101.Init(i2c_bus, axp2101_config);
      if (err != ESP_OK) return false;
      
      uint8_t data = 0x00;
      err = axp2101.ReadReg8(0x90, data, -1);
      if (err != ESP_OK) return false;
      data |= 0b10110100;
      std::tuple<uint8_t, uint8_t> axp_cmds[] = {
          {0x90, data},
          {0x99, (uint8_t)(0b11110 - 5)},
          {0x97, (uint8_t)(0b11110 - 2)},
          {0x69, 0b00110101},
          {0x30, 0b111111},
          {0x90, 0xBF},
          {0x94, 33 - 5},
          {0x95, 33 - 5},
      };
      for (const auto &[reg, value] : axp_cmds)
      {
        err = axp2101.WriteReg8(reg, value, -1);
        if (err != ESP_OK) return false;
      }
      axp2101.GetLogger().Info("Configured successfully");

      // AW9523
      err = aw9523.Init(i2c_bus, aw9523_config);
      if (err != ESP_OK) return false;
      
      std::tuple<uint8_t, uint8_t> aw_cmds[] = {
          {0x02, 0b00000111},
          {0x03, 0b10001111},
          {0x04, 0b00011000},
          {0x05, 0b00001100},
          {0x11, 0b00010000},
          {0x12, 0b11111111},
          {0x13, 0b11111111},
      };
      for (const auto &[reg, value] : aw_cmds)
      {
        err = aw9523.WriteReg8(reg, value, -1);
        if (err != ESP_OK)
        {
          aw9523.GetLogger().Error("Failed to write register 0x%02X: %s", reg, esp_err_to_name(err));
          return false;
        }
      }
      aw9523.GetLogger().Info("Configured successfully");
  }

  if (touch) {
      err = ft5x06.Init(i2c_bus, ft5x06_config, esp_lcd_touch_new_i2c_ft5x06);
      if (err != ESP_OK) return false;
  }

  if (display) {
      err = ili9341.Init(spi_bus, spi_lcd_config, esp_lcd_new_panel_ili9341);
      if (err != ESP_OK) return false;
  }

  if (audio) {
      audio_codec.Init(i2s_bus);
      // speaker
      err = audio_codec.AddSpeaker(i2c_bus, AW88298_CODEC_DEFAULT_ADDR, spk_codec_new_func);
      if (err != ESP_OK)
      {
        audio_codec.GetLogger().Error("Failed to add speaker: %s", esp_err_to_name(err));
        return false;
      }
      // microphone
      err = audio_codec.AddMicrophone(i2c_bus, ES7210_CODEC_DEFAULT_ADDR, mic_codec_new_func);
      if (err != ESP_OK)
      {
        audio_codec.GetLogger().Error("Failed to add microphone: %s", esp_err_to_name(err));
        return false;
      }
  }
  
  return true;
}

bool M5StackCoreS3::InitMiddleware(bool lvgl)
{
    esp_err_t err = ESP_OK;
    if (lvgl) {
      err = lvgl_port.Init(lvgl_port_config);
      if (err != ESP_OK) return false;
      
      err = lvgl_port.AddDisplay(ili9341, lvgl_display_config);
      if (err != ESP_OK) return false;
      
      err = lvgl_port.AddTouch(ft5x06, lvgl_touch_config);
      if (err != ESP_OK) return false;
    }
    return true;
}

bool M5StackCoreS3::Init()
{
    if (initialized_) return true;
    if (!InitSocAbility(true)) return false;
    if (!InitBus(true, true, true)) return false;
    if (!InitDevice(true, true, true, true)) return false;
    if (!InitMiddleware(true)) return false;
    
    initialized_ = true;
    return true;
}