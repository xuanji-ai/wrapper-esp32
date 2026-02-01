#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the M5Stack Core S3 Board
 * 
 * Initializes I2C, Power (PMIC, Expand IO), Audio, Display, and Camera.
 * 
 * @return 0 on success, -1 on failure
 */
int m5_core_s3_init(void);

/**
 * @brief Deinitialize the M5Stack Core S3 Board
 * 
 * Deinitializes all peripherals in reverse order.
 * 
 * @return 0 on success, -1 on failure
 */
int m5_core_s3_deinit(void);

/**
 * @brief Perform a self-check of the board peripherals
 * 
 * Logs the status of each peripheral.
 * 
 * @return 0 on success
 */
int m5_core_s3_self_check(void);

/**
 * @brief Enable the display
 */
void m5_core_s3_display_enable(void);

/**
 * @brief Disable the display
 */
void m5_core_s3_display_disable(void);

/**
 * @brief Set the display brightness
 * 
 * @param level Brightness level (0-100)
 */
void m5_core_s3_display_set_brightness(int level);

/**
 * @brief Initialize the LVGL port
 * 
 * Initializes LVGL, adds the display and touch input.
 * 
 * @return 0 on success, -1 on failure
 */
int m5_core_s3_lvgl_port_init(void);

/**
 * @brief Deinitialize the LVGL port
 */
void m5_core_s3_lvgl_port_deinit(void);

/**
 * @brief Lock the LVGL port
 * 
 * Used for thread-safe LVGL operations.
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return true if locked, false if timeout
 */
bool m5_core_s3_lvgl_lock(uint32_t timeout_ms);

/**
 * @brief Unlock the LVGL port
 */
void m5_core_s3_lvgl_unlock(void);

#ifdef __cplusplus
}
#endif
