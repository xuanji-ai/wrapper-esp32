#include <string>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "service.hpp"
#include "freertps.hpp"

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_CORE_S3
#include "board/m5stack/core-s3.hpp"
using namespace wrapper;
static void board_init(void *arg)
{
  M5StackCoreS3& board = M5StackCoreS3::GetInstance();

  board.Init();
  board.GetLvglPort().Test();
  vTaskDelete(nullptr);
}
#endif

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_TAB5
#include "board/m5stack/tab5.hpp"
using namespace wrapper;
static void board_init(void *arg)
{
  M5StackTab5& board = M5StackTab5::GetInstance();

  board.Init();
  board.SetDisplayBacklight(true);
  board.SetDisplayBrightness(80);

  board.GetLvglPort().Lock(0);
  lv_obj_t* scr = lv_scr_act();
  lv_obj_t* label = lv_label_create(scr);
  lv_label_set_text(label, "Hello, M5Stack Tab5!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  board.GetLvglPort().Unlock();
  vTaskDelete(nullptr);
}
#endif

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_CARDPUTER

#include "wrapper/logger.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"
#include "wrapper/display.hpp"
#include "wrapper/i2s.hpp"

using namespace wrapper;

// SPI Bus Config
SpiBusConfig spi_bus_config(
  SPI2_HOST,
  GPIO_NUM_35, // mosi
  GPIO_NUM_NC, // miso
  GPIO_NUM_36, // sclk
  -1, -1, -1, -1, -1, -1, // quad/data pins
  true, // data_default_level
  4096, // max_transfer_sz
  SPICOMMON_BUSFLAG_MASTER,
  ESP_INTR_CPU_AFFINITY_AUTO,
  0,
  SPI_DMA_CH_AUTO
);

// SPI Display Config
SpiDisplayConfig spi_display_config(
  GPIO_NUM_37, // cs
  GPIO_NUM_34, // dc
  0,           // spi_mode
  40000000,    // clock_speed_hz
  8,           // cmd_bits
  8,           // param_bits
  10,          // queue_depth
  nullptr,     // on_color_trans_done
  nullptr,     // user_ctx
  0, 0,        // cs_ena pre/post
  0, 0, 0, 0, 0, 0, 0, 0, // flags
  GPIO_NUM_33  // reset_gpio
);

// I2C Config (Grove)
I2cBusConfig i2c_bus_config(
  I2C_NUM_0,
  GPIO_NUM_2, // sda
  GPIO_NUM_1, // scl
  I2C_CLK_SRC_DEFAULT,
  7,          // glitch_ignore
  0,          // intr_prio
  10,         // queue_depth
  true,       // enable_pullup
  false       // enable_pd
);

// I2S Config (Speaker - NS4168)
// Note: Speaker uses BCLK=41, SDATA=42, LRCLK=43
I2sBusConfig i2s_speaker_bus_cfg(
    I2S_NUM_0,
    I2S_ROLE_MASTER,
    6,    // dma_desc_num
    1024, // dma_frame_num
    true, // auto_clear_after_cb
    false, // auto_clear_before_cb
    0     // intr_priority
);

I2sChanStdConfig i2s_speaker_chan_cfg(
    48000,                  // sample_rate_hz
    I2S_CLK_SRC_DEFAULT,    // clk_src
    0,                      // ext_clk_freq_hz
    I2S_MCLK_MULTIPLE_256,  // mclk_multiple
    8,                      // bclk_div
    I2S_DATA_BIT_WIDTH_16BIT, // data_bit_width
    I2S_SLOT_BIT_WIDTH_AUTO,  // slot_bit_width
    I2S_SLOT_MODE_MONO,       // slot_mode (Speaker usually mono or stereo, NS4168 is mono amp but input usually stereo I2S?)
    I2S_STD_SLOT_BOTH,        // slot_mask
    16,                       // ws_width
    false,                    // ws_pol
    false,                    // bit_shift
    false,                    // left_align
    false,                    // big_endian
    false,                    // bit_order_lsb
    GPIO_NUM_NC,              // mclk
    GPIO_NUM_41,              // bclk
    GPIO_NUM_43,              // ws (LRCLK)
    GPIO_NUM_42,              // dout (SDATA)
    GPIO_NUM_NC               // din
);

// I2S Config (Mic - SPM1423)
// Note: Mic uses DAT=46, CLK=43. CLK(43) is shared with Speaker LRCLK?
// This implies they might share the same I2S bus if PDM and Standard I2S can mix or if configured carefully.
// However, SPM1423 is PDM. NS4168 is I2S.
// If they share GPIO 43 (LRCLK for Speaker, CLK for Mic), it implies the Mic uses the WS signal as its PDM clock?
// Or they use different I2S peripherals?
// I will create a separate config for Mic on I2S_NUM_1 for now, but note the pin conflict if 43 is used.
// Wait, G43 is LRCLK for Speaker and CLK for Mic.
// In PDM mode, WS is often the CLK.
// So maybe they share the bus?
// I will leave Mic config commented out or separate for now to avoid conflict in init without logic.

/*
// Keyboard & Battery (No wrapper found for GPIO/ADC yet)
// Battery: ADC G10
// Keyboard: Matrix (Rows: G13,15,3,4,5,6,7; Cols: G8,9,11)
*/

/*
// SD Card (No wrapper found yet)
// Pins: CS=12, MOSI=14, CLK=40, MISO=39
*/

void board_init(void *arg)
{

  vTaskDelete(nullptr);
}

#endif


extern "C" void app_main()
{
  // nvs init
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  xTaskCreate(board_init, "board_init", 8192, nullptr, 5, nullptr);
}
