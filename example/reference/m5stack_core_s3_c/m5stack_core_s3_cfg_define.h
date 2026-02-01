#pragma once

#include <stdbool.h>
#include <stddef.h>

//! Config Includes
#include "sdkconfig.h"
//! Bus Includes
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <driver/i2s_common.h>
//! Misc Includes

//! Audio Includes
#include <driver/i2s_std.h>
#include <driver/i2s_tdm.h>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
//! Display Includes
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9341.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_lvgl_port.h>
//! Camera Includes
#include <esp_camera.h>

//! Bus Default Configurations
#define I2C_MASTER_BUS_CONFIG_M5CS3_I2C1() {  \
  .i2c_port   = I2C_NUM_1,           \
  .sda_io_num = GPIO_NUM_12,         \
  .scl_io_num = GPIO_NUM_11,         \
  .clk_source = I2C_CLK_SRC_DEFAULT, \
}

#define SPI_BUS_CONFIG_M5CS3_SPI3() {      \
  .mosi_io_num     = GPIO_NUM_37,                  \
  .miso_io_num     = GPIO_NUM_NC,                  \
  .sclk_io_num     = GPIO_NUM_36,                  \
  .quadwp_io_num   = GPIO_NUM_NC,                  \
  .quadhd_io_num   = GPIO_NUM_NC,                  \
  .max_transfer_sz = 320 * 240 * sizeof(uint16_t), \
}

#define I2S_CHAN_CONFIG_M5CS3_I2S0() { \
  .id                   = I2S_NUM_0,         \
  .role                 = I2S_ROLE_MASTER,   \
  .dma_desc_num         = 6,                 \
  .dma_frame_num        = 240,               \
  .auto_clear_after_cb  = true,              \
  .auto_clear_before_cb = false,             \
  .intr_priority        = 0,                 \
}

//! PowerManager Device Default Configurations
#define I2C_DEVICE_CONFIG_AXP2101() { \
  .dev_addr_length = I2C_ADDR_BIT_LEN_7, \
  .device_address  = 0x34,               \
  .scl_speed_hz    = 400000,             \
}

#define I2C_DEVICE_CONFIG_AW9523() { \
  .dev_addr_length = I2C_ADDR_BIT_LEN_7, \
  .device_address  = 0x58,               \
  .scl_speed_hz    = 400000,             \
}

//! Audio Default Configurations
#define I2S_STD_CONFIG_M5CS3_I2S0_TX() {                \
  .clk_cfg = {                                       \
    .sample_rate_hz = 16000,                         \
    .clk_src        = I2S_CLK_SRC_DEFAULT,           \
    .ext_clk_freq_hz = 0,                            \
    .mclk_multiple  = I2S_MCLK_MULTIPLE_256          \
  },                                                 \
  .slot_cfg = {                                      \
    .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,      \
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,       \
    .slot_mode      = I2S_SLOT_MODE_STEREO,          \
    .slot_mask      = I2S_STD_SLOT_BOTH,             \
    .ws_width       = I2S_DATA_BIT_WIDTH_16BIT,      \
    .ws_pol         = false,                         \
    .bit_shift      = true,                          \
    .left_align     = true,                          \
    .big_endian     = false,                         \
    .bit_order_lsb  = false                          \
  },                                                 \
  .gpio_cfg = {                                      \
    .mclk = GPIO_NUM_0,                              \
    .bclk = GPIO_NUM_34,                             \
    .ws   = GPIO_NUM_33,                             \
    .dout = GPIO_NUM_13,                             \
    .din  = GPIO_NUM_NC,                             \
    .invert_flags = {                                \
      .mclk_inv = false,                             \
      .bclk_inv = false,                             \
      .ws_inv   = false                              \
    }                                                \
  }                                                  \
}

#define I2S_TDM_CONFIG_M5CS3_I2S0_RX() {                       \
  .clk_cfg = {                                              \
    .sample_rate_hz = 16000,                                \
    .clk_src        = I2S_CLK_SRC_DEFAULT,                  \
    .ext_clk_freq_hz = 0,                                   \
    .mclk_multiple  = I2S_MCLK_MULTIPLE_256,                \
    .bclk_div       = 8,                                    \
  },                                                        \
  .slot_cfg = {                                             \
    .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,             \
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,              \
    .slot_mode      = I2S_SLOT_MODE_STEREO,                 \
    .slot_mask      = (i2s_tdm_slot_mask_t)(I2S_TDM_SLOT0 | I2S_TDM_SLOT1 | I2S_TDM_SLOT2 | I2S_TDM_SLOT3), \
    .ws_width       = I2S_TDM_AUTO_WS_WIDTH,                \
    .ws_pol         = false,                                \
    .bit_shift      = true,                                 \
    .left_align     = false,                                \
    .big_endian     = false,                                \
    .bit_order_lsb  = false,                                \
    .skip_mask      = false,                                \
    .total_slot     = I2S_TDM_AUTO_SLOT_NUM                 \
  },                                                        \
  .gpio_cfg = {                                             \
    .mclk = GPIO_NUM_0,                                     \
    .bclk = GPIO_NUM_34,                                    \
    .ws   = GPIO_NUM_33,                                    \
    .dout = GPIO_NUM_NC,                                    \
    .din  = GPIO_NUM_NC,                                    \
    .invert_flags = {                                       \
      .mclk_inv = false,                                    \
      .bclk_inv = false,                                    \
      .ws_inv   = false                                     \
    }                                                       \
  }                                                         \
}

//speaker codec aw88298
#define AUDIO_CODEC_I2C_CFG_AW88298() { \
  .port       = I2C_NUM_1,                 \
  .addr       = AW88298_CODEC_DEFAULT_ADDR,\
  .bus_handle = NULL                       \
}

#define AW88298_CODEC_CFG() { \
  .ctrl_if   = NULL,     \
  .gpio_if   = NULL,     \
  .reset_pin = GPIO_NUM_NC, \
  .hw_gain   = {               \
    .pa_voltage        = 5.0f, \
    .codec_dac_voltage = 3.3f, \
    .pa_gain           = 1.f\
  },                         \
}

//microphone codec es8388
#define AUDIO_CODEC_I2C_CFG_ES7210() { \
  .port       = I2C_NUM_1,                 \
  .addr       = ES7210_CODEC_DEFAULT_ADDR,\
  .bus_handle = NULL,                  \
}

#define ES7210_CODEC_CFG() {           \
  .ctrl_if     = NULL,                 \
  .master_mode = false,                \
  .mic_selected = 0,                   \
  .mclk_src    = ES7210_MCLK_FROM_PAD, \
  .mclk_div    = 0,                    \
}

//! Display Default Configurations
#define ESP_LCD_PANEL_IO_SPI_CONFIG_M5CS3_DISP() { \
  .cs_gpio_num      = GPIO_NUM_3,          \
  .dc_gpio_num      = GPIO_NUM_35,         \
  .spi_mode         = 2,                   \
  .pclk_hz          = 40 * 1000 * 1000,    \
  .trans_queue_depth = 10,                 \
  .lcd_cmd_bits     = 8,                   \
  .lcd_param_bits   = 8,                   \
  .flags = {                               \
    .dc_high_on_cmd = 0,                   \
    .dc_low_on_data = 0,                   \
    .dc_low_on_param = 0,                  \
    .octal_mode     = 0,                   \
    .quad_mode      = 0,                   \
    .sio_mode       = 0,                   \
    .lsb_first      = 0,                   \
    .cs_high_active = 0,                   \
  }                                        \
}

#define ESP_LCD_PANEL_DEV_CONFIG_M5CS3_DISP() { \
  .reset_gpio_num = GPIO_NUM_NC,              \
  .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR,\
  .data_endian    = LCD_RGB_DATA_ENDIAN_BIG,  \
  .bits_per_pixel = 16,                       \
  .flags = {                                  \
    .reset_active_high = 0                    \
  },                                          \
  .vendor_config = NULL                       \
}

#define ESP_LCD_PANEL_IO_I2C_CONFIG_M5CS3_TOUCH() { \
  .dev_addr            = 0x38,          \
  .on_color_trans_done = NULL,          \
  .user_ctx            = NULL,          \
  .control_phase_bytes = 1,             \
  .dc_bit_offset       = 0,             \
  .lcd_cmd_bits        = 8,             \
  .lcd_param_bits      = 0,             \
  .flags = {                           \
    .dc_low_on_data       = 0,          \
    .disable_control_phase = 1,         \
  },                                   \
  .scl_speed_hz        = 400000         \
}

#define ESP_LCD_TOUCH_CONFIG_M5CS3_TOUCH() { \
  .x_max = 320,                  \
  .y_max = 240,                  \
  .rst_gpio_num = GPIO_NUM_NC,   \
  .int_gpio_num = GPIO_NUM_NC,   \
  .levels = {                    \
    .reset     = 0,              \
    .interrupt = 0,              \
  },                             \
  .flags = {                     \
    .swap_xy  = 0,               \
    .mirror_x = 0,               \
    .mirror_y = 0,               \
  },                             \
}

#define LVGL_PORT_CONFIG_M5CS3() {        \
  .task_priority    = 5,               \
  .task_stack       = 8192,            \
  .task_affinity    = APP_CPU_NUM,     \
  .task_max_sleep_ms = 500,            \
  .task_stack_caps  = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, \
  .timer_period_ms  = 20,              \
}

#define LVGL_PORT_DISPLAY_CONFIG_M5CS3() { \
  .io_handle      = NULL,             \
  .panel_handle   = NULL,             \
  .control_handle = NULL,             \
  .buffer_size    = 320 * 240,        \
  .double_buffer  = true,             \
  .trans_size     = 0,                \
  .hres           = 320,              \
  .vres           = 240,              \
  .monochrome     = false,            \
  .rotation = {                       \
    .swap_xy  = false,                \
    .mirror_x = false,                \
    .mirror_y = false,                \
  },                                  \
  .color_format = LV_COLOR_FORMAT_RGB565, \
  .flags = {                          \
    .buff_dma     = 1,                \
    .buff_spiram  = 1,                \
    .sw_rotate    = 0,                \
    .swap_bytes   = 1,                \
    .full_refresh = 0,                \
    .direct_mode  = 0,                \
  }                                   \
}

#define LVGL_PORT_TOUCH_CONFIG_M5CS3() { \
  .disp   = NULL,                     \
  .handle = NULL,                     \
  .scale  = {                         \
    .x = 0.0f,                        \
    .y = 0.0f,                        \
  },                                  \
}

/**************************************************************************************************
 *
 * Camera interface
 *
 * M5Stack-Core-S3 is shipped with GC0308 camera module.
 * As a camera driver, esp32-camera component is used.
 *
 * Example configuration:
 * \code{.c}
 * const camera_config_t camera_config = BSP_CAMERA_DEFAULT_CONFIG;
 * esp_err_t err = esp_camera_init(&camera_config);
 * \endcode
 **************************************************************************************************/
/**
 * @brief Camera default configuration
 *
 * In this configuration we select RGB565 color format and 320x240 image size - matching the display.
 * We use double-buffering for the best performance.
 * Since we don't want to waste internal SRAM, we allocate the framebuffers in external PSRAM.
 * By setting XCLK to 16MHz, we configure the esp32-camera driver to use EDMA when accessing the PSRAM.
 *
 * @attention I2C must be enabled by bsp_i2c_init(), before camera is initialized
 */
#define CAMERA_CONFIG_M5CS3_GC0308() \
    {                                     \
        .pin_pwdn      = GPIO_NUM_NC,        \
        .pin_reset     = GPIO_NUM_NC,        \
        .pin_xclk      = GPIO_NUM_NC,        \
        .pin_sccb_sda  = GPIO_NUM_NC,        \
        .pin_sccb_scl  = GPIO_NUM_NC,        \
        .pin_d7        = GPIO_NUM_47,        \
        .pin_d6        = GPIO_NUM_48,        \
        .pin_d5        = GPIO_NUM_16,        \
        .pin_d4        = GPIO_NUM_15,        \
        .pin_d3        = GPIO_NUM_42,        \
        .pin_d2        = GPIO_NUM_41,        \
        .pin_d1        = GPIO_NUM_40,        \
        .pin_d0        = GPIO_NUM_39,        \
        .pin_vsync     = GPIO_NUM_46,        \
        .pin_href      = GPIO_NUM_38,        \
        .pin_pclk      = GPIO_NUM_45,        \
        .xclk_freq_hz  = 10000000,           \
        .ledc_timer    = LEDC_TIMER_0,       \
        .ledc_channel  = LEDC_CHANNEL_0,     \
        .pixel_format  = PIXFORMAT_RGB565,   \
        .frame_size    = FRAMESIZE_QVGA,     \
        .jpeg_quality  = 12,                 \
        .fb_count      = 2,                  \
        .fb_location   = CAMERA_FB_IN_PSRAM, \
        .sccb_i2c_port = I2C_NUM_1,          \
    }
#define CAMERA_CONFIG_M5CS3_GC0308_VFLIP        0
#define CAMERA_CONFIG_M5CS3_GC0308_HMIRROR      0
