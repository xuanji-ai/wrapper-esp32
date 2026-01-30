#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "esp_io_expander.h"
#include "sdmmc_cmd.h"
#include "esp_codec_dev.h"
#include "m5stack_tab5_cfg_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the M5Stack Tab5 Board
 * 
 * @return ESP_OK on success
 */
esp_err_t m5stack_tab5_init(void);

/**
 * @brief Deinitialize the M5Stack Tab5 Board
 * 
 * @return ESP_OK on success
 */
esp_err_t m5stack_tab5_deinit(void);

// Individual init functions (can be used if fine-grained control is needed)
esp_err_t m5stack_tab5_i2c_init(void);
esp_err_t m5stack_tab5_io_expander_init(void);
esp_err_t m5stack_tab5_display_init(void);
esp_err_t m5stack_tab5_touch_init(void);
esp_err_t m5stack_tab5_audio_init(void);
esp_err_t m5stack_tab5_sdcard_init(void);

// Control functions
void m5stack_tab5_display_backlight_set(int brightness_percent);
void m5stack_tab5_display_on(void);
void m5stack_tab5_display_off(void);

// LVGL Port (if needed)
#if __has_include("esp_lvgl_port.h")
#include "esp_lvgl_port.h"
esp_err_t m5stack_tab5_lvgl_init(void);
bool m5stack_tab5_lvgl_lock(uint32_t timeout_ms);
void m5stack_tab5_lvgl_unlock(void);
#endif

#ifdef __cplusplus
}
#endif
