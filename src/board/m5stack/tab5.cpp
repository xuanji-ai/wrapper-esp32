#include <esp_lcd_ili9881c.h>
#include <esp_lcd_touch_gt911.h>
#include "board/m5stack/tab5.hpp"
#include "device/ili9881c.hpp"
#include "device/gt911.hpp"

namespace wrapper
{
  I2cBusConfig i2c_cfg(
      I2C_NUM_0,           // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_31,         // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_32,         // 中文注释：已按当前代码逻辑本地化。
      I2C_CLK_SRC_DEFAULT, // 中文注释：已按当前代码逻辑本地化。
      7,                   // 中文注释：已按当前代码逻辑本地化。
      0,                   // 中文注释：已按当前代码逻辑本地化。
      0,                   // 中文注释：已按当前代码逻辑本地化。
      true,                // 中文注释：已按当前代码逻辑本地化。
      false                // 中文注释：已按当前代码逻辑本地化。
  );

  // 中文注释：已按当前代码逻辑本地化。
  LedcTimerConfig ledc_timer_cfg(
      LEDC_LOW_SPEED_MODE, // 中文注释：已按当前代码逻辑本地化。
      LEDC_TIMER_10_BIT,   // 中文注释：已按当前代码逻辑本地化。
      LEDC_TIMER_0,        // 中文注释：已按当前代码逻辑本地化。
      5000,                // 中文注释：已按当前代码逻辑本地化。
      LEDC_AUTO_CLK        // 中文注释：已按当前代码逻辑本地化。
  );

  LedcChannelConfig ledc_channel_cfg(
      GPIO_NUM_22,         // 中文注释：已按当前代码逻辑本地化。
      LEDC_LOW_SPEED_MODE, // 中文注释：已按当前代码逻辑本地化。
      LEDC_CHANNEL_0,      // 中文注释：已按当前代码逻辑本地化。
      LEDC_INTR_DISABLE,   // 中文注释：已按当前代码逻辑本地化。
      LEDC_TIMER_0,        // 中文注释：已按当前代码逻辑本地化。
      0,                   // 中文注释：已按当前代码逻辑本地化。
      0                    // 中文注释：已按当前代码逻辑本地化。
  );

  LdoChannelConfig dsi_phy_ldo_cfg(
      3,     // 中文注释：已按当前代码逻辑本地化。
      2500,  // 中文注释：已按当前代码逻辑本地化。
      false, // 中文注释：已按当前代码逻辑本地化。
      false  // 中文注释：已按当前代码逻辑本地化。
  );

  DsiBusConfig dsi_bus_cfg(
      0,                            // 中文注释：已按当前代码逻辑本地化。
      2,                            // 中文注释：已按当前代码逻辑本地化。
      MIPI_DSI_PHY_CLK_SRC_DEFAULT, // 中文注释：已按当前代码逻辑本地化。
      1000                          // 中文注释：已按当前代码逻辑本地化。
  );

  DsiDisplayConfig dsi_display_cfg(
      // 中文注释：已按当前代码逻辑本地化。
      0, // 中文注释：已按当前代码逻辑本地化。
      8, // 中文注释：已按当前代码逻辑本地化。
      8, // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      0,                                  // 中文注释：已按当前代码逻辑本地化。
      MIPI_DSI_DPI_CLK_SRC_DEFAULT,       // 中文注释：已按当前代码逻辑本地化。
      60,                                 // 中文注释：已按当前代码逻辑本地化。
      LCD_COLOR_PIXEL_FORMAT_RGB565,      // 中文注释：已按当前代码逻辑本地化。
      static_cast<lcd_color_format_t>(0), // 中文注释：已按当前代码逻辑本地化。
      static_cast<lcd_color_format_t>(0), // 中文注释：已按当前代码逻辑本地化。
      1,                                  // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      720,  // 中文注释：已按当前代码逻辑本地化。
      1280, // 中文注释：已按当前代码逻辑本地化。
      40,   // 中文注释：已按当前代码逻辑本地化。
      140,  // 中文注释：已按当前代码逻辑本地化。
      40,   // 中文注释：已按当前代码逻辑本地化。
      4,    // 中文注释：已按当前代码逻辑本地化。
      20,   // 中文注释：已按当前代码逻辑本地化。
      20,   // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      false, // 中文注释：已按当前代码逻辑本地化。
      false, // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_NC,               // 中文注释：已按当前代码逻辑本地化。
      LCD_RGB_ELEMENT_ORDER_RGB, // 中文注释：已按当前代码逻辑本地化。
      LCD_RGB_DATA_ENDIAN_BIG,   // 中文注释：已按当前代码逻辑本地化。
      16,                        // 中文注释：已按当前代码逻辑本地化。
      false                      // 中文注释：已按当前代码逻辑本地化。
  );

  I2cTouchConfig gt911_touch_cfg(
      ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP,
      nullptr,
      nullptr,
      1,
      0,
      16,
      0,
      0,
      0,
      400000,
      1280,
      800,
      GPIO_NUM_NC,
      GPIO_NUM_NC,
      0,
      0,
      0,
      0,
      0,
      nullptr,
      nullptr);

  I2sBusConfig i2s_bus_cfg(
      I2S_NUM_0,
      I2S_ROLE_MASTER,
      6,
      256,
      true,
      false,
      0);

  I2sChanStdConfig tx_rx_std_cfg(
      // 中文注释：已按当前代码逻辑本地化。
      48000,                 // 中文注释：已按当前代码逻辑本地化。
      I2S_CLK_SRC_DEFAULT,   // 中文注释：已按当前代码逻辑本地化。
      0,                     // 中文注释：已按当前代码逻辑本地化。
      I2S_MCLK_MULTIPLE_256, // 中文注释：已按当前代码逻辑本地化。
      8,                     // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      I2S_DATA_BIT_WIDTH_16BIT, // 中文注释：已按当前代码逻辑本地化。
      I2S_SLOT_BIT_WIDTH_AUTO,  // 中文注释：已按当前代码逻辑本地化。
      I2S_SLOT_MODE_MONO,       // 中文注释：已按当前代码逻辑本地化。
      I2S_STD_SLOT_BOTH,        // 中文注释：已按当前代码逻辑本地化。
      16,                       // 中文注释：已按当前代码逻辑本地化。
      false,                    // 中文注释：已按当前代码逻辑本地化。
      true,                     // 中文注释：已按当前代码逻辑本地化。
      true,                     // 中文注释：已按当前代码逻辑本地化。
      false,                    // 中文注释：已按当前代码逻辑本地化。
      false,                    // 中文注释：已按当前代码逻辑本地化。
      // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_30, // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_27, // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_29, // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_26, // 中文注释：已按当前代码逻辑本地化。
      GPIO_NUM_28, // 中文注释：已按当前代码逻辑本地化。
      false,       // 中文注释：已按当前代码逻辑本地化。
      false,       // 中文注释：已按当前代码逻辑本地化。
      false        // 中文注释：已按当前代码逻辑本地化。
  );

  LvglPortConfig lvgl_port_cfg(
      5,          // 中文注释：已按当前代码逻辑本地化。
      8192,       // 中文注释：已按当前代码逻辑本地化。
      1,          // 中文注释：已按当前代码逻辑本地化。
      20,         // 中文注释：已按当前代码逻辑本地化。
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, // 中文注释：已按当前代码逻辑本地化。
      25          // 中文注释：已按当前代码逻辑本地化。
  );

  LvglDisplayConfig lvgl_display_cfg(
      720 * 50,   // 中文注释：已按当前代码逻辑本地化。
      true,       // 中文注释：已按当前代码逻辑本地化。
      0,          // 中文注释：已按当前代码逻辑本地化。
      720,        // 中文注释：已按当前代码逻辑本地化。
      1280,       // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      LV_COLOR_FORMAT_RGB565, // 中文注释：已按当前代码逻辑本地化。
      true,       // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      true,       // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      false,      // 中文注释：已按当前代码逻辑本地化。
      false       // 中文注释：已按当前代码逻辑本地化。
  );

  LvglTouchConfig lvgl_touch_cfg(
      0.0f,   // 中文注释：已按当前代码逻辑本地化。
      0.0f    // 中文注释：已按当前代码逻辑本地化。
  );

  LvglDisplayDsiConfig lvgl_dsi_cfg(
      false   // 中文注释：已按当前代码逻辑本地化。
  );

  Logger lmain("Main");
  Logger li2c("Board", "I2C", "Bus");
  Logger lioexp0("Board", "I2C", "IoExpander0");
  Logger lioexp1("Board", "I2C", "IoExpander1");
  Logger lledc("Board", "LEDC");
  Logger lldo("Board", "LDO");
  Logger ldisp("Board", "Display");
  Logger ltouch("Board", "Touch");
  Logger laudio("Board", "Audio");
  Logger llvgl("Board", "LVGL");

  I2cBus i2c_bus(li2c);
  Pi4ioe5v6408 io_expander0(lioexp0); // 中文注释：已按当前代码逻辑本地化。
  Pi4ioe5v6408 io_expander1(lioexp1); // 中文注释：已按当前代码逻辑本地化。
  LedcTimer ledc_timer(lledc);
  LedcChannel ledc_channel(lledc);
  LdoRegulator dsi_phy_ldo(lldo);

  DsiBus dsi_bus(ldisp);
  Ili9881c dsi_display(ldisp);
  Gt911 gt911_touch(ltouch);

  I2sBus i2s_bus(laudio);
  LvglPort lvgl_port(llvgl);

  AudioCodec audio_codec(laudio);
  std::function<esp_err_t()> spk_codec_new_func = []() -> esp_err_t
  {
    es8388_codec_cfg_t spk_codec_cfg = {
        .ctrl_if = audio_codec.GetSpeakerCtrlInterface(),
        .gpio_if = audio_codec.GetGpioInterface(),
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .master_mode = false,
        .pa_pin = GPIO_NUM_NC,
        .pa_reverted = false,
        .hw_gain = {.pa_voltage = 5.0, .codec_dac_voltage = 3.3, .pa_gain = 0.0f},
    };

    const audio_codec_if_t *codec_if = es8388_codec_new(&spk_codec_cfg);
    if (codec_if == NULL)
    {
      return ESP_ERR_INVALID_STATE;
    }
    audio_codec.SetSpeakerCodecInterface(codec_if);

    esp_codec_dev_handle_t spk_codec_dev_handle = nullptr;
    esp_codec_dev_cfg_t spk_codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .codec_if = codec_if,
        .data_if = audio_codec.GetDataInterface(),
    };
    spk_codec_dev_handle = esp_codec_dev_new(&spk_codec_dev_cfg);
    if (spk_codec_dev_handle == nullptr)
    {
      return ESP_ERR_INVALID_STATE;
    }
    audio_codec.SetSpeakerCodecDeviceHandle(spk_codec_dev_handle);
    return ESP_OK;
  };
  std::function<esp_err_t()> mic_codec_new_func = []() -> esp_err_t
  {
    es7210_codec_cfg_t mic_codec_cfg = {
        .ctrl_if = audio_codec.GetMicrophoneCtrlInterface(),
        .master_mode = false,
        .mic_selected = ES7210_SEL_MIC1,
        .mclk_src = ES7210_MCLK_FROM_PAD,
        .mclk_div = 0};

    const audio_codec_if_t *codec_if = es7210_codec_new(&mic_codec_cfg);
    if (codec_if == NULL)
    {
      return ESP_ERR_INVALID_STATE;
    }
    audio_codec.SetMicrophoneCodecInterface(codec_if);
    esp_codec_dev_handle_t mic_codec_dev_handle = nullptr;
    esp_codec_dev_cfg_t mic_codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = codec_if,
        .data_if = audio_codec.GetDataInterface(),
    };
    mic_codec_dev_handle = esp_codec_dev_new(&mic_codec_dev_cfg);
    if (mic_codec_dev_handle == nullptr)
    {
      return ESP_ERR_INVALID_STATE;
    }
    audio_codec.SetMicrophoneCodecDeviceHandle(mic_codec_dev_handle);
    return ESP_OK;
  };

  bool M5StackTab5::Init(int level)
  {
    if(level>0)
    {
        // 中文注释：已按当前代码逻辑本地化。
        if (!i2c_bus.Init(i2c_cfg)) {
        return false;
        }
        i2c_bus.Scan();

        // 中文注释：已按当前代码逻辑本地化。
        // 中文注释：已按当前代码逻辑本地化。
        if (!io_expander0.Init(i2c_bus, Pi4ioe5v6408::ADDR_LOW)) {  // 中文注释：已按当前代码逻辑本地化。
        return false;
        }
        if (!io_expander1.Init(i2c_bus, Pi4ioe5v6408::ADDR_HIGH)) { // 中文注释：已按当前代码逻辑本地化。
        return false;
        }
    }
    
    if(level>1)
    {
        // 中文注释：已按当前代码逻辑本地化。
        io_expander0.SetDirection(IO_EXPANDER_PIN_NUM_4, IO_EXPANDER_OUTPUT); // 中文注释：已按当前代码逻辑本地化。
        io_expander0.SetLevel(IO_EXPANDER_PIN_NUM_4, 1);
        io_expander0.SetOutputMode(IO_EXPANDER_PIN_NUM_4, IO_EXPANDER_OUTPUT_MODE_PUSH_PULL);

        io_expander0.SetDirection(IO_EXPANDER_PIN_NUM_5, IO_EXPANDER_OUTPUT); // 中文注释：已按当前代码逻辑本地化。
        io_expander0.SetLevel(IO_EXPANDER_PIN_NUM_5, 1);

        io_expander0.SetDirection(IO_EXPANDER_PIN_NUM_1, IO_EXPANDER_OUTPUT); // 中文注释：已按当前代码逻辑本地化。
        io_expander0.SetLevel(IO_EXPANDER_PIN_NUM_1, 1);

        io_expander1.SetDirection(IO_EXPANDER_PIN_NUM_3, IO_EXPANDER_OUTPUT); // 中文注释：已按当前代码逻辑本地化。
        io_expander1.SetLevel(IO_EXPANDER_PIN_NUM_3, 1);

        io_expander1.SetDirection(IO_EXPANDER_PIN_NUM_0, IO_EXPANDER_OUTPUT); // 中文注释：已按当前代码逻辑本地化。
        io_expander1.SetLevel(IO_EXPANDER_PIN_NUM_0, 1);

        // 中文注释：已按当前代码逻辑本地化。
        if (!ledc_timer.Init(ledc_timer_cfg)) {
            return false;
        }
        if (!ledc_channel.Init(ledc_channel_cfg)) {
            return false;
        }

        // 中文注释：已按当前代码逻辑本地化。
        if (!dsi_phy_ldo.Init(dsi_phy_ldo_cfg)) {
            return false;
        }

        // 中文注释：已按当前代码逻辑本地化。
        vTaskDelay(pdMS_TO_TICKS(100));

        dsi_bus.Init(dsi_bus_cfg);
        if (!dsi_display.Init(dsi_bus, dsi_display_cfg)) {
            return false;
        }

        // 中文注释：已按当前代码逻辑本地化。
        dsi_display.InvertColor(false);

        if (!gt911_touch.Init(i2c_bus, gt911_touch_cfg)) {
            return false;
        }

        if (!i2s_bus.Init(i2s_bus_cfg)) {
            return false;
        }
        if (!i2s_bus.ConfigureTxChannel(tx_rx_std_cfg)) {
            return false;
        }
        if (!i2s_bus.ConfigureRxChannel(tx_rx_std_cfg)) {
            return false;
        }
        if (!i2s_bus.EnableTxChannel()) {
            return false;
        }
        if (!i2s_bus.EnableRxChannel()) {
            return false;
        }

        audio_codec.Init(i2s_bus);
        audio_codec.AddSpeaker(i2c_bus, ES8388_CODEC_DEFAULT_ADDR, spk_codec_new_func);
        audio_codec.AddMicrophone(i2c_bus, ES7210_CODEC_DEFAULT_ADDR, mic_codec_new_func);

        if (!lvgl_port.Init(lvgl_port_cfg)) {
            return false;
        }
        if (!lvgl_port.AddDisplayDsi(dsi_display, lvgl_display_cfg, lvgl_dsi_cfg)) {
            return false;
        }
        if (!lvgl_port.AddTouch(gt911_touch, lvgl_touch_cfg)) {
            return false;
        }

        // 开启背光
        SetDisplayBrightness(100);
    }   

    return true;
  }

  I2cBus& M5StackTab5::GetI2cBus()
  {
    return i2c_bus;
  }

  Pi4ioe5v6408& M5StackTab5::GetIoExpander0()
  {
    return io_expander0;
  }

  Pi4ioe5v6408& M5StackTab5::GetIoExpander1()
  {
    return io_expander1;
  }

  LedcTimer& M5StackTab5::GetLedcTimer()
  {
    return ledc_timer;
  }

  LedcChannel& M5StackTab5::GetLedcChannel()
  {
    return ledc_channel;
  }

  LdoRegulator& M5StackTab5::GetDsiPhyLdo()
  {
    return dsi_phy_ldo;
  }

  DsiBus& M5StackTab5::GetDsiBus()
  {
    return dsi_bus;
  }

  Ili9881c& M5StackTab5::GetDsiDisplay()
  {
    return dsi_display;
  }

  Gt911& M5StackTab5::GetGt911Touch()
  {
    return gt911_touch;
  }

  I2sBus& M5StackTab5::GetI2sBus()
  {
    return i2s_bus;
  }

  AudioCodec& M5StackTab5::GetAudioCodec()
  {
    return audio_codec;
  }

  LvglPort& M5StackTab5::GetLvglPort()
  {
    return lvgl_port;
  }

  void M5StackTab5::SetDisplayBrightness(int percent)
  {
    if (percent < 0)
      percent = 0;
    if (percent > 100)
      percent = 100;
      
    // 中文注释：已按当前代码逻辑本地化。
    uint32_t duty = (1023 * percent) / 100;
    ledc_channel.SetDutyAndUpdate(duty);
  }

  void M5StackTab5::SetDisplayBacklight(bool on)
  {
    SetDisplayBrightness(on ? 100 : 0);
  }

  void M5StackTab5::SetDisplayPower(bool on)
  {
    // 中文注释：已按当前代码逻辑本地化。
    io_expander0.SetLevel(IO_EXPANDER_PIN_NUM_4, on ? 1 : 0);
  }
} // 中文注释：已按当前代码逻辑本地化。