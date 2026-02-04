#include "wrapper/lvgl.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace wrapper;

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

esp_err_t LvglPort::AddDisplay(const Display& lcd, const LvglDisplayConfig& config)
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

void LvglPort::Test()
{
    m_logger.Info("LVGL Functional Test Start");

    if (!Lock(0))
    {
        m_logger.Error("Failed to acquire LVGL lock");
        return;
    }

    // 1. Check if active screen exists
    lv_obj_t *scr = lv_scr_act();
    if (scr)
    {
        m_logger.Info("Active screen found: %p", scr);
    }
    else
    {
        m_logger.Error("No active screen!");
        Unlock();
        return;
    }

    // Clear screen first (Black background)
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);

    // 2. Create a test label
    lv_obj_t *label = lv_label_create(scr);
    if (label)
    {
        lv_label_set_text(label, "LVGL TEST\nRUNNING");
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0); // White text
        m_logger.Info("Test label created");
    }
    else
    {
        m_logger.Error("Failed to create label");
    }

    // 3. Create a simple animation object (Spinner)
    lv_obj_t *spinner = lv_spinner_create(scr);
    if (spinner)
    {
        lv_spinner_set_anim_params(spinner, 1000, 60);
        lv_obj_set_size(spinner, 40, 40);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 20);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
        m_logger.Info("Test spinner created");
    }

    Unlock();

    m_logger.Info("LVGL Functional Test Complete");
}

bool LvglPort::SetRotation(uint32_t rotation)
{
    if (!Lock(0))
    {
        m_logger.Error("Failed to acquire LVGL lock");
        return false;
    }
    
    if (m_lvgl_display == NULL)
    {
        m_logger.Error("Display must be added before setting rotation");
        Unlock();
        return false;
    }

    if (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270)
    {
        m_logger.Error("Invalid rotation value. Must be 0, 90, 180, or 270 degrees");
        Unlock();
        return false;
    }

    lvgl_port_set_rotation(m_lvgl_display, rotation);
    Unlock();
    m_logger.Info("Display rotation set to %d degrees", rotation);
    return true;
}

