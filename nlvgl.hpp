#pragma once

#include "esp_lvgl_port.h"
#include "ndisplay.hpp"
#include "ntouch.hpp"
#include "nlogger.hpp"

namespace nix
{

/**
 * @brief Configuration for LVGL port task.
 */
struct LvglPortConfig : public lvgl_port_cfg_t
{
    LvglPortConfig(int task_prio,
                   int stack_sz,
                   int affinity,
                   int max_sleep_ms,
                   uint32_t stack_caps,
                   uint32_t timer_ms)
    {
        task_priority = task_prio;
        task_stack = stack_sz;
        task_affinity = affinity;
        task_max_sleep_ms = max_sleep_ms;
        task_stack_caps = stack_caps;
        timer_period_ms = timer_ms;
    }
};

/**
 * @brief Configuration for LVGL display.
 */
struct LvglDisplayConfig : public lvgl_port_display_cfg_t
{
    LvglDisplayConfig(uint32_t buf_sz,
                      bool double_buf,
                      uint32_t trans_sz,
                      uint32_t hor_res,
                      uint32_t ver_res,
                      bool mono,
                      lv_color_format_t format,
                      bool swap_xy,
                      bool mirror_x,
                      bool mirror_y,
                      bool buff_dma,
                      bool buff_spiram,
                      bool sw_rotate,
                      bool swap_bytes,
                      bool full_refresh,
                      bool direct_mode)
    {
        io_handle = NULL;
        panel_handle = NULL;
        control_handle = NULL;
        buffer_size = buf_sz;
        double_buffer = double_buf;
        trans_size = trans_sz;
        hres = hor_res;
        vres = ver_res;
        monochrome = mono;
        color_format = format;
        rotation.swap_xy = swap_xy;
        rotation.mirror_x = mirror_x;
        rotation.mirror_y = mirror_y;
        flags.buff_dma = buff_dma;
        flags.buff_spiram = buff_spiram;
        flags.sw_rotate = sw_rotate;
        flags.swap_bytes = swap_bytes;
        flags.full_refresh = full_refresh;
        flags.direct_mode = direct_mode;
    }
};

/**
 * @brief Configuration for LVGL touch.
 */
struct LvglTouchConfig : public lvgl_port_touch_cfg_t
{
    LvglTouchConfig(float scale_x = 0.0f, float scale_y = 0.0f)
    {
        disp = NULL;
        handle = NULL;
        scale.x = scale_x;
        scale.y = scale_y;
    }
};

class LvglPort
{
    Logger& m_logger;
    lv_display_t* m_lvgl_display;
    lv_indev_t* m_lvgl_touch;
    bool m_initialized;

public:
    LvglPort(Logger& logger);
    ~LvglPort();

    /**
     * @brief Initialize the LVGL port.
     * 
     * @param config The port configuration.
     * @return esp_err_t ESP_OK on success.
     */
    esp_err_t Init(const LvglPortConfig& config);

    /**
     * @brief Deinitialize the LVGL port and remove devices.
     * 
     * @return esp_err_t ESP_OK on success.
     */
    esp_err_t Deinit();

    /**
     * @brief Add an I2C LCD to LVGL.
     * 
     * @param lcd The I2C LCD instance.
     * @param config The display configuration.
     * @return esp_err_t ESP_OK on success.
     */
    esp_err_t AddDisplay(const I2cLcd& lcd, const LvglDisplayConfig& config);

    /**
     * @brief Add an SPI LCD to LVGL.
     * 
     * @param lcd The SPI LCD instance.
     * @param config The display configuration.
     * @return esp_err_t ESP_OK on success.
     */
    esp_err_t AddDisplay(const SpiLcd& lcd, const LvglDisplayConfig& config);

    /**
     * @brief Add a touch controller to LVGL.
     * 
     * @param touch The I2C touch controller instance.
     * @param config The touch configuration.
     * @return esp_err_t ESP_OK on success.
     */
    esp_err_t AddTouch(const I2cTouch& touch, const LvglTouchConfig& config);

    /**
     * @brief Lock the LVGL port for thread-safe access.
     * 
     * @param timeout_ms The timeout in milliseconds.
     * @return true if locked successfully.
     */
    bool Lock(uint32_t timeout_ms);

    /**
     * @brief Unlock the LVGL port.
     */
    void Unlock();

    /**
     * @brief Check if initialized.
     * 
     * @return true if initialized.
     */
    bool IsInitialized() const { return m_initialized; }

    /**
     * @brief Get the display handle.
     * 
     * @return lv_display_t* 
     */
    lv_display_t* GetDisplay() const { return m_lvgl_display; }

    /**
     * @brief Get the touch input device handle.
     * 
     * @return lv_indev_t* 
     */
    lv_indev_t* GetTouch() const { return m_lvgl_touch; }
};

} // namespace nix
