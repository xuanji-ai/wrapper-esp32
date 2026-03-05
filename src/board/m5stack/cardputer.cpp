#include "sdkconfig.h"
#include "cardputer.hpp"

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_CARDPUTER

#include "esp_lcd_st7789.h"
#include <cmath>
#include "wrapper/logger.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"
#include "wrapper/display.hpp"
#include "wrapper/i2s.hpp"
#include "wrapper/audio.hpp"
#include "device/m5stack_cardputer_keyboard.hpp"

namespace wrapper
{

// 中文注释：已按当前代码逻辑本地化。
SpiBusConfig spi_bus_config(
  SPI2_HOST,
  GPIO_NUM_35, // 中文注释：已按当前代码逻辑本地化。
  GPIO_NUM_NC, // 中文注释：已按当前代码逻辑本地化。
  GPIO_NUM_36, // 中文注释：已按当前代码逻辑本地化。
  -1, -1, -1, -1, -1, -1, // 中文注释：已按当前代码逻辑本地化。
  true, // 中文注释：已按当前代码逻辑本地化。
  4096, // 中文注释：已按当前代码逻辑本地化。
  SPICOMMON_BUSFLAG_MASTER,
  ESP_INTR_CPU_AFFINITY_AUTO,
  0,
  SPI_DMA_CH_AUTO
);

// 中文注释：已按当前代码逻辑本地化。
SpiDisplayConfig spi_display_config(
  GPIO_NUM_37, // 中文注释：已按当前代码逻辑本地化。
  GPIO_NUM_34, // 中文注释：已按当前代码逻辑本地化。
  0,           // 中文注释：已按当前代码逻辑本地化。
  40000000,    // 中文注释：已按当前代码逻辑本地化。
  8,           // 中文注释：已按当前代码逻辑本地化。
  8,           // 中文注释：已按当前代码逻辑本地化。
  10,          // 中文注释：已按当前代码逻辑本地化。
  nullptr,     // 中文注释：已按当前代码逻辑本地化。
  nullptr,     // 中文注释：已按当前代码逻辑本地化。
  0, 0,        // 中文注释：已按当前代码逻辑本地化。
  0, 0, 0, 0, 0, 0, 0, 0, // 中文注释：已按当前代码逻辑本地化。
  GPIO_NUM_33  // 中文注释：已按当前代码逻辑本地化。
);

// 中文注释：已按当前代码逻辑本地化。
I2cBusConfig i2c_bus_config(
  I2C_NUM_1,
  GPIO_NUM_2, // 中文注释：已按当前代码逻辑本地化。
  GPIO_NUM_1, // 中文注释：已按当前代码逻辑本地化。
  I2C_CLK_SRC_DEFAULT,
  7,          // 中文注释：已按当前代码逻辑本地化。
  0,          // 中文注释：已按当前代码逻辑本地化。
  0,         // 中文注释：已按当前代码逻辑本地化。
  true,       // 中文注释：已按当前代码逻辑本地化。
  false       // 中文注释：已按当前代码逻辑本地化。
);

// 中文注释：已按当前代码逻辑本地化。
// 中文注释：已按当前代码逻辑本地化。
I2sBusConfig i2s_bus_cfg(
    I2S_NUM_0,
    I2S_ROLE_MASTER,
    6,    // 中文注释：已按当前代码逻辑本地化。
    1024, // 中文注释：已按当前代码逻辑本地化。
    true, // 中文注释：已按当前代码逻辑本地化。
    false, // 中文注释：已按当前代码逻辑本地化。
    0     // 中文注释：已按当前代码逻辑本地化。
);

I2sChanStdConfig i2s_speaker_chan_cfg(
    48000,                  // 中文注释：已按当前代码逻辑本地化。
    I2S_CLK_SRC_DEFAULT,    // 中文注释：已按当前代码逻辑本地化。
    0,                      // 中文注释：已按当前代码逻辑本地化。
    I2S_MCLK_MULTIPLE_256,  // 中文注释：已按当前代码逻辑本地化。
    8,                      // 中文注释：已按当前代码逻辑本地化。
    I2S_DATA_BIT_WIDTH_16BIT, // 中文注释：已按当前代码逻辑本地化。
    I2S_SLOT_BIT_WIDTH_AUTO,  // 中文注释：已按当前代码逻辑本地化。
    I2S_SLOT_MODE_MONO,       // 中文注释：已按当前代码逻辑本地化。
    I2S_STD_SLOT_BOTH,        // 中文注释：已按当前代码逻辑本地化。
    16,                       // 中文注释：已按当前代码逻辑本地化。
    false,                    // 中文注释：已按当前代码逻辑本地化。
    false,                    // 中文注释：已按当前代码逻辑本地化。
    false,                    // 中文注释：已按当前代码逻辑本地化。
    false,                    // 中文注释：已按当前代码逻辑本地化。
    false,                    // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_NC,              // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_41,              // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_43,              // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_42,              // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_NC               // 中文注释：已按当前代码逻辑本地化。
);

I2sChanPdmRxConfig i2s_mic_chan_cfg(
    16000,                  // 中文注释：已按当前代码逻辑本地化。
    I2S_CLK_SRC_DEFAULT,    // 中文注释：已按当前代码逻辑本地化。
    I2S_MCLK_MULTIPLE_256,  // 中文注释：已按当前代码逻辑本地化。
    I2S_PDM_DSR_8S,         // 中文注释：已按当前代码逻辑本地化。
    8,                      // 中文注释：已按当前代码逻辑本地化。
    I2S_DATA_BIT_WIDTH_16BIT, // 中文注释：已按当前代码逻辑本地化。
    I2S_SLOT_BIT_WIDTH_AUTO,  // 中文注释：已按当前代码逻辑本地化。
    I2S_SLOT_MODE_MONO,       // 中文注释：已按当前代码逻辑本地化。
    I2S_PDM_SLOT_BOTH,        // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_43,              // 中文注释：已按当前代码逻辑本地化。
    GPIO_NUM_46,              // 中文注释：已按当前代码逻辑本地化。
    false                     // 中文注释：已按当前代码逻辑本地化。
);

/* 中文注释：已按当前代码逻辑本地化。 */

/* 中文注释：已按当前代码逻辑本地化。 */
Keyboard keyboard;

/* 中文注释：已按当前代码逻辑本地化。 */

// 中文注释：已按当前代码逻辑本地化。
Logger logger_i2c("I2C");

Logger logger_display("Display");
Logger logger_spi("SPI");

Logger logger_i2s("I2S");
Logger logger_spk("Speaker");
Logger logger_mic("Mic");

I2cBus i2c_bus(logger_i2c);

SpiBus spi_bus(logger_spi);
SpiDisplay display(logger_display, spi_bus);

I2sBus i2s_bus(logger_i2s);
Speaker speaker(logger_spk);
Microphone mic(logger_mic);

bool M5StackCardputer::Init()
{
  i2c_bus.Init(i2c_bus_config);// 中文注释：已按当前代码逻辑本地化。
  spi_bus.Init(spi_bus_config);// 中文注释：已按当前代码逻辑本地化。
  display.Init(spi_display_config, esp_lcd_new_panel_st7789);// 中文注释：已按当前代码逻辑本地化。
  i2s_bus.Init(i2s_bus_cfg);// 中文注释：已按当前代码逻辑本地化。
  i2s_bus.ConfigureTxChannel(i2s_speaker_chan_cfg);// 中文注释：已按当前代码逻辑本地化。
  i2s_bus.ConfigureRxChannel(i2s_mic_chan_cfg);// 中文注释：已按当前代码逻辑本地化。

  // 中文注释：已按当前代码逻辑本地化。
  KeyboardConfig keyboard_config;
  keyboard_config.input_pins = {13, 15, 3, 4, 5, 6, 7};
  keyboard_config.output_pins = {8, 9, 11};
  keyboard.Init(keyboard_config);
  logger_display.Info("Keyboard Initialized");
  return true;
}

} // 中文注释：已按当前代码逻辑本地化。

#endif