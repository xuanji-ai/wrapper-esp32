#include "nlvgl.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace nix;

LvglPort::LvglPort(Logger& logger) 
    : m_logger(logger), 
      m_lvgl_display(NULL), 
      m_lvgl_touch(NULL), 
      m_initialized(false)
{
}

LvglPort::~LvglPort()
{
    Deinit();
}

esp_err_t LvglPort::Init(const LvglPortConfig& config)
{
    if (m_initialized)
    {
        m_logger.Warning("LVGL port already initialized");
        return ESP_OK;
    }

    esp_err_t ret = lvgl_port_init(&config);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to initialize LVGL port: %s", esp_err_to_name(ret));
        return ret;
    }

    m_initialized = true;
    m_logger.Info("LVGL port initialized");
    return ESP_OK;
}

esp_err_t LvglPort::Deinit()
{
    if (m_lvgl_touch != NULL)
    {
        lvgl_port_remove_touch(m_lvgl_touch);
        m_lvgl_touch = NULL;
    }

    if (m_lvgl_display != NULL)
    {
        lvgl_port_remove_disp(m_lvgl_display);
        m_lvgl_display = NULL;
    }

    if (m_initialized)
    {
        esp_err_t ret = lvgl_port_deinit();
        if (ret != ESP_OK)
        {
            m_logger.Error("Failed to deinitialize LVGL port: %s", esp_err_to_name(ret));
            return ret;
        }
        m_initialized = false;
        m_logger.Info("LVGL port deinitialized");
        
        // Give LVGL worker task a moment to exit cleanly
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

esp_err_t LvglPort::AddDisplay(const I2cLcd& lcd, const LvglDisplayConfig& config)
{
    if (!m_initialized)
    {
        m_logger.Error("LVGL port not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_lvgl_display != NULL)
    {
        m_logger.Warning("Display already added. Removing existing display first.");
        lvgl_port_remove_disp(m_lvgl_display);
        m_lvgl_display = NULL;
    }

    LvglDisplayConfig final_config = config;
    final_config.io_handle = lcd.GetIoHandle();
    final_config.panel_handle = lcd.GetPanelHandle();

    m_lvgl_display = lvgl_port_add_disp(&final_config);
    if (m_lvgl_display == NULL)
    {
        m_logger.Error("Failed to add LVGL display");
        return ESP_FAIL;
    }

    m_logger.Info("LVGL display added");
    return ESP_OK;
}

esp_err_t LvglPort::AddDisplay(const SpiLcd& lcd, const LvglDisplayConfig& config)
{
    if (!m_initialized)
    {
        m_logger.Error("LVGL port not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_lvgl_display != NULL)
    {
        m_logger.Warning("Display already added. Removing existing display first.");
        lvgl_port_remove_disp(m_lvgl_display);
        m_lvgl_display = NULL;
    }

    LvglDisplayConfig final_config = config;
    final_config.io_handle = lcd.GetIoHandle();
    final_config.panel_handle = lcd.GetPanelHandle();

    m_lvgl_display = lvgl_port_add_disp(&final_config);
    if (m_lvgl_display == NULL)
    {
        m_logger.Error("Failed to add LVGL display");
        return ESP_FAIL;
    }

    m_logger.Info("LVGL display added");
    return ESP_OK;
}

esp_err_t LvglPort::AddTouch(const I2cTouch& touch, const LvglTouchConfig& config)
{
    if (!m_initialized)
    {
        m_logger.Error("LVGL port not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_lvgl_display == NULL)
    {
        m_logger.Error("Display must be added before touch");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_lvgl_touch != NULL)
    {
        m_logger.Warning("Touch already added. Removing existing touch first.");
        lvgl_port_remove_touch(m_lvgl_touch);
        m_lvgl_touch = NULL;
    }

    LvglTouchConfig final_config = config;
    final_config.disp = m_lvgl_display;
    final_config.handle = touch.GetHandle();

    m_lvgl_touch = lvgl_port_add_touch(&final_config);
    if (m_lvgl_touch == NULL)
    {
        m_logger.Error("Failed to add LVGL touch");
        return ESP_FAIL;
    }

    m_logger.Info("LVGL touch added");
    return ESP_OK;
}

bool LvglPort::Lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void LvglPort::Unlock()
{
    lvgl_port_unlock();
}
