#include <vector>
#include "esp_log.h"

#include "esp_lcd_ili9341.h"
#include "esp_lcd_touch_ft5x06.h"

#include "nlogger.hpp"
#include "ni2c.hpp"
#include "nspi.hpp"
#include "ni2s.hpp"
#include "ndisplay.hpp"
#include "ntouch.hpp"
#include "nlvgl.hpp"

#include "devices/axp2101.hpp"
#include "devices/aw9523.hpp"

using namespace nix;

Logger logger_main("app_main");

Logger logger_i2c_bus("i2c","bus");
Logger logger_spi_bus("spi","bus");
Logger logger_i2s_bus("i2s","bus");

Logger logger_axp2101("i2c","device","axp2101");
Logger logger_aw9523("i2c","device","aw9523");
Logger logger_touch("i2c","touch");
Logger logger_ili9341("spi","lcd","ili9341");
Logger logger_lvgl("lvgl","port");

I2sBusConfig bus_config(I2S_NUM_0,I2S_ROLE_MASTER,6,240,true,false,0);

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
    false
);

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
    false
);

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
    0x38,                           // dev_addr
    400000,                         // scl_speed_hz (400kHz)
    320,                            // x_max
    240,                            // y_max
    GPIO_NUM_NC,                    // rst_gpio
    GPIO_NUM_NC                     // int_gpio
);

SpiLcdConfig spi_lcd_config(
    GPIO_NUM_3,                      // cs_gpio
    GPIO_NUM_35,                     // dc_gpio
    40 * 1000 * 1000,               // clock_speed_hz (40MHz)
    2,                              // spi_mode
    8,                              // lcd_cmd_bits
    8,                              // lcd_param_bits
    GPIO_NUM_NC,                    // reset_gpio
    LCD_RGB_ELEMENT_ORDER_BGR,      // rgb_order
    LCD_RGB_DATA_ENDIAN_BIG,        // data_endian
    16,                             // bits_per_pixel
    false,                          // reset_active_high
    NULL                            // vendor_conf
);

I2cDeviceConfig axp2101_config(Axp2101::DEFAULT_ADDR, 400000);
I2cDeviceConfig aw9523_config(Aw9523::DEFAULT_ADDR, 400000);

LvglPortConfig lvgl_port_config(
    5,                                  // task_priority
    8192,                               // task_stack
    APP_CPU_NUM,                        // task_affinity
    500,                                // task_max_sleep_ms
    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT,// task_stack_caps
    20                                  // timer_period_ms
);

LvglDisplayConfig lvgl_display_config(
    320 * 240,                          // buffer_size
    true,                               // double_buffer
    0,                                  // trans_size
    320,                                // hres
    240,                                // vres
    false,                              // monochrome
    LV_COLOR_FORMAT_RGB565,             // color_format
    false,                              // swap_xy
    false,                              // mirror_x
    false,                              // mirror_y
    true,                               // buff_dma
    true,                               // buff_spiram
    false,                              // sw_rotate
    true,                               // swap_bytes
    false,                              // full_refresh
    false                               // direct_mode
);

LvglTouchConfig lvgl_touch_config(0.0f, 0.0f);

I2cBus i2c_bus(logger_i2c_bus);
SpiBus spi_bus(logger_spi_bus);
I2sBus i2s_bus(logger_i2s_bus);

Axp2101 axp2101(logger_axp2101);
Aw9523 aw9523(logger_aw9523);
I2cTouch ft5x06(logger_touch);
SpiLcd ili9341(logger_ili9341);
LvglPort lvgl_port(logger_lvgl);

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;
  //! bus--------------------------------------------------------------------------
  err = i2c_bus.Init(i2c_bus_config);
  if (err != ESP_OK) {
      return;
  }
  err = spi_bus.Init(spi_bus_config);
  if (err != ESP_OK) {
      return;
  }
  err = i2s_bus.Init(bus_config);
  if (err != ESP_OK) {
      return;
  }
  err = i2s_bus.ConfigureTxChannel(tx_config);
  if (err != ESP_OK) {
      return;
  }
  err = i2s_bus.ConfigureRxChannel(rx_config);
  if (err != ESP_OK) {
      return;
  }
  //! device--------------------------------------------------------------------------
  err = axp2101.Init(i2c_bus, axp2101_config);
  if (err != ESP_OK) {
      return;
  }
  err = axp2101.Configure();
  if (err != ESP_OK) {
      return;
  }
  err = aw9523.Init(i2c_bus, aw9523_config);
  if (err != ESP_OK) {
      return;
  }
  err = aw9523.Configure();
  if (err != ESP_OK) {
      return;
  }
  err = ft5x06.Init(i2c_bus, ft5x06_config, esp_lcd_touch_new_i2c_ft5x06);
  if (err != ESP_OK) {
      return;
  }
  err = ili9341.Init(spi_bus, spi_lcd_config, esp_lcd_new_panel_ili9341);
  if (err != ESP_OK) {
      return;
  }
  //! middleware--------------------------------------------------------------------------
  err = lvgl_port.Init(lvgl_port_config);
  if (err != ESP_OK) {
      return;
  }
  err = lvgl_port.AddDisplay(ili9341, lvgl_display_config);
  if (err != ESP_OK) {
      return;
  }
  err = lvgl_port.AddTouch(ft5x06, lvgl_touch_config);
  if (err != ESP_OK) {
      return;
  }
  //! app--------------------------------------------------------------------------
  lvgl_port.Lock(1000);
  lv_obj_t* scr = lv_scr_act();
  lv_obj_t* label = lv_label_create(scr);
  lv_label_set_text(label, "Hello, LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  lvgl_port.Unlock();
}
