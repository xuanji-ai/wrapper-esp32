#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_codec_dev.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "esp_lcd_ili9881c.h"
#include "es8388.h"

/*
 * Pin Definitions
 */
#define M5TAB5_I2C_SCL           (GPIO_NUM_32)
#define M5TAB5_I2C_SDA           (GPIO_NUM_31)

#define M5TAB5_I2S_MCLK          (GPIO_NUM_30)
#define M5TAB5_I2S_SCLK          (GPIO_NUM_27)
#define M5TAB5_I2S_LCLK          (GPIO_NUM_29)
#define M5TAB5_I2S_DOUT          (GPIO_NUM_26)
#define M5TAB5_I2S_DSIN          (GPIO_NUM_28)

#define M5TAB5_LCD_BACKLIGHT     (GPIO_NUM_22)
// IO Expander pins
#define M5TAB5_LCD_EN_PIN        (IO_EXPANDER_PIN_NUM_4)
#define M5TAB5_TOUCH_EN_PIN      (IO_EXPANDER_PIN_NUM_5)
#define M5TAB5_SPEAKER_EN_PIN    (IO_EXPANDER_PIN_NUM_1)
#define M5TAB5_CAMERA_EN_PIN     (IO_EXPANDER_PIN_NUM_6)
#define M5TAB5_USB_EN_PIN        (IO_EXPANDER_PIN_NUM_3)
#define M5TAB5_WIFI_EN_PIN       (IO_EXPANDER_PIN_NUM_0)

#define M5TAB5_USB_POS           (GPIO_NUM_20)
#define M5TAB5_USB_NEG           (GPIO_NUM_19)

#define M5TAB5_SD_D0             (GPIO_NUM_39)
#define M5TAB5_SD_D1             (GPIO_NUM_40)
#define M5TAB5_SD_D2             (GPIO_NUM_41)
#define M5TAB5_SD_D3             (GPIO_NUM_42)
#define M5TAB5_SD_CMD            (GPIO_NUM_44)
#define M5TAB5_SD_CLK            (GPIO_NUM_43)

// Display Resolution
#define M5TAB5_LCD_H_RES         (1280)
#define M5TAB5_LCD_V_RES         (800)
#define M5TAB5_LCD_MIPI_DSI_LANE_NUM (2)

/*
 * Configuration Macros
 */

// I2C Config
#define I2C_MASTER_BUS_CONFIG_M5TAB5() {       \
    .i2c_port = I2C_NUM_0,                     \
    .sda_io_num = M5TAB5_I2C_SDA,              \
    .scl_io_num = M5TAB5_I2C_SCL,              \
    .clk_source = I2C_CLK_SRC_DEFAULT,         \
    .glitch_ignore_cnt = 7,                    \
    .intr_priority = 0,                        \
    .flags.enable_internal_pullup = true,      \
}

// IO Expander Config (Address only as new_i2c function is used)
#define IO_EXPANDER_0_ADDR_M5TAB5 (ESP_IO_EXPANDER_I2C_PI4IOE5V6408_ADDRESS_LOW)
#define IO_EXPANDER_1_ADDR_M5TAB5 (ESP_IO_EXPANDER_I2C_PI4IOE5V6408_ADDRESS_HIGH)

// LEDC Backlight Config
#define LEDC_TIMER_CONFIG_M5TAB5() { \
    .speed_mode = LEDC_LOW_SPEED_MODE, \
    .duty_resolution = LEDC_TIMER_10_BIT, \
    .timer_num = LEDC_TIMER_0, \
    .freq_hz = 5000, \
    .clk_cfg = LEDC_AUTO_CLK, \
}

#define LEDC_CHANNEL_CONFIG_M5TAB5() { \
    .gpio_num = M5TAB5_LCD_BACKLIGHT, \
    .speed_mode = LEDC_LOW_SPEED_MODE, \
    .channel = LEDC_CHANNEL_0, \
    .intr_type = LEDC_INTR_DISABLE, \
    .timer_sel = LEDC_TIMER_0, \
    .duty = 0, \
    .hpoint = 0, \
}

// Display - MIPI DSI Bus
#define DSI_BUS_CONFIG_M5TAB5() { \
    .bus_id = 0, \
    .num_data_lanes = M5TAB5_LCD_MIPI_DSI_LANE_NUM, \
    .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT, \
    .lane_bit_rate_mbps = 1000, \
}

// Display - DBI IO
#define DBI_IO_CONFIG_M5TAB5() { \
    .virtual_channel = 0, \
    .lcd_cmd_bits = 8, \
    .lcd_param_bits = 8, \
}

// Display - DPI Panel
#define DPI_PANEL_CONFIG_M5TAB5() { \
    .virtual_channel = 0, \
    .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT, \
    .dpi_clock_freq_mhz = 60, \
    .in_color_format = LCD_COLOR_FMT_RGB565, \
    .video_timing = { \
        .h_size = M5TAB5_LCD_H_RES, \
        .v_size = M5TAB5_LCD_V_RES, \
        .hsync_back_porch = 140, \
        .hsync_pulse_width = 40, \
        .hsync_front_porch = 40, \
        .vsync_back_porch = 20, \
        .vsync_pulse_width = 4, \
        .vsync_front_porch = 20, \
    }, \
    .num_fbs = 1, \
    .flags.use_dma2d = true, \
}

// Display - Vendor Config
// Note: m5tab5_disp_init_data must be visible or declared extern if used here.
// Since this is a header, we assume m5tab5_disp_init_data is available where this macro is used or we declare it.
// To avoid header dependency loop, we can leave init_cmds NULL here and set it in C file if it's a large array.
// BUT user wants "ALL config" in header.
// I will include the disp_init_data header in the C file, and here I will assume the symbol exists or include it?
// Including `m5stack_tab5_disp_init_data.h` here might be messy if it defines variables (not just declares).
// My previous `m5stack_tab5_disp_init_data.h` defined `static const ...`. This is bad for inclusion in multiple files, but fine if included once.
// Ideally `m5stack_tab5_cfg_define.h` shouldn't depend on large data arrays.
// I will declare extern here? No, `static` in header means every inclusion gets a copy.
// I'll refer to it by name, and the user of the macro must have it visible.
#define ILI9881C_VENDOR_CONFIG_M5TAB5() { \
    .init_cmds = m5tab5_disp_init_data, \
    .init_cmds_size = sizeof(m5tab5_disp_init_data) / sizeof(m5tab5_disp_init_data[0]), \
    .mipi_config = { \
        .dsi_bus = NULL, /* Set at runtime */ \
        .dpi_config = NULL, /* Set at runtime */ \
        .lane_num = M5TAB5_LCD_MIPI_DSI_LANE_NUM, \
    }, \
}

// Display - Panel Device
#define LCD_PANEL_DEV_CONFIG_M5TAB5() { \
    .reset_gpio_num = GPIO_NUM_NC, \
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, \
    .bits_per_pixel = 16, \
    .vendor_config = NULL, /* Set at runtime */ \
}

// Touch Config
#define TOUCH_PANEL_IO_I2C_CONFIG_M5TAB5() {   \
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP, \
    .on_color_trans_done = NULL,               \
    .user_ctx = NULL,                          \
    .control_phase_bytes = 1,                  \
    .dc_bit_offset = 0,                        \
    .lcd_cmd_bits = 16,                        \
    .lcd_param_bits = 0,                       \
    .flags = {                                 \
        .dc_low_on_data = 0,                   \
        .disable_control_phase = 0,            \
    },                                         \
    .scl_speed_hz = 400 * 1000,                \
}

#define TOUCH_PANEL_CONFIG_M5TAB5() {          \
    .x_max = M5TAB5_LCD_H_RES,                 \
    .y_max = M5TAB5_LCD_V_RES,                 \
    .rst_gpio_num = GPIO_NUM_NC,               \
    .int_gpio_num = GPIO_NUM_NC,               \
    .levels = {                                \
        .reset = 0,                            \
        .interrupt = 0,                        \
    },                                         \
    .flags = {                                 \
        .swap_xy = 0,                          \
        .mirror_x = 0,                         \
        .mirror_y = 0,                         \
    },                                         \
    .process_coordinates = NULL,               \
    .interrupt_callback = NULL,                \
}

// Audio - I2S Channel
#define I2S_CHAN_CONFIG_M5TAB5() { \
    .id = I2S_NUM_0, \
    .role = I2S_ROLE_MASTER, \
    .dma_desc_num = 6, \
    .dma_frame_num = 240, \
    .auto_clear = true, \
}

// Audio - I2S STD
#define I2S_STD_CONFIG_M5TAB5() { \
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000), \
    .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), \
    .gpio_cfg = { \
        .mclk = M5TAB5_I2S_MCLK, \
        .bclk = M5TAB5_I2S_SCLK, \
        .ws = M5TAB5_I2S_LCLK, \
        .dout = M5TAB5_I2S_DOUT, \
        .din = M5TAB5_I2S_DSIN, \
    }, \
}

// Audio - Codec I2S Data
#define AUDIO_CODEC_I2S_CONFIG_M5TAB5() { \
    .port = I2S_NUM_0, \
    .rx_handle = NULL, /* Set at runtime */ \
    .tx_handle = NULL, /* Set at runtime */ \
}

// Audio - Codec I2C Ctrl
#define AUDIO_CODEC_I2C_CONFIG_M5TAB5() { \
    .port = I2C_NUM_0, \
    .addr = ES8388_CODEC_DEFAULT_ADDR, \
    .bus_handle = NULL, /* Set at runtime */ \
}

// Audio - ES8388
#define ES8388_CODEC_CONFIG_M5TAB5() { \
    .ctrl_if = NULL, /* Set at runtime */ \
    .gpio_if = NULL, /* Set at runtime */ \
    .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC, \
    .pa_pin = GPIO_NUM_NC, \
    .hw_gain = {.pa_voltage = 5.0, .codec_dac_voltage = 3.3}, \
}

// Audio - Codec Dev
#define CODEC_DEV_CONFIG_M5TAB5() { \
    .dev_type = ESP_CODEC_DEV_TYPE_OUT, \
    .codec_if = NULL, /* Set at runtime */ \
    .data_if = NULL, /* Set at runtime */ \
}

// SD Card - Host
#define SDMMC_HOST_CONFIG_M5TAB5() { \
    .flags = SDMMC_HOST_FLAG_8BIT | SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_1BIT | SDMMC_HOST_FLAG_DDR, \
    .slot = SDMMC_HOST_SLOT_0, \
    .max_freq_khz = SDMMC_FREQ_HIGHSPEED, \
    .io_voltage = 3.3f, \
    .init_wait_ms = 0, \
    .command_timeout_ms = 0, \
    .pwr_ctrl_handle = NULL, /* Set at runtime */ \
}

// SD Card - Slot
#define SDMMC_SLOT_CONFIG_M5TAB5() { \
    .clk = M5TAB5_SD_CLK, \
    .cmd = M5TAB5_SD_CMD, \
    .d0 = M5TAB5_SD_D0, \
    .d1 = M5TAB5_SD_D1, \
    .d2 = M5TAB5_SD_D2, \
    .d3 = M5TAB5_SD_D3, \
    .cd = SDMMC_SLOT_NO_CD, \
    .wp = SDMMC_SLOT_NO_WP, \
    .width = 4, \
    .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP, \
}

// SD Card - Mount
#define SD_MOUNT_CONFIG_M5TAB5() { \
    .format_if_mount_failed = false, \
    .max_files = 5, \
    .allocation_unit_size = 16 * 1024, \
}

// LDO Config for DSI PHY
#define LDO_DSI_PHY_CONFIG_M5TAB5() { \
    .chan_id = 3, \
    .voltage_mv = 2500, \
}

// LDO Config for SD Power
#define LDO_SD_PWR_CONFIG_M5TAB5() { \
    .ldo_chan_id = 4, \
}
