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

void LvglPort::Test(bool is_monochrome)
{
    m_logger.Info("LVGL Functional Test Start (%s mode)", is_monochrome ? "Monochrome" : "Color");

    if (!Lock(0))
    {
        m_logger.Error("Failed to acquire LVGL lock");
        return;
    }

    lv_obj_t *scr = lv_scr_act();
    if (!scr)
    {
        m_logger.Error("No active screen!");
        Unlock();
        return;
    }

    // Get display resolution
    int32_t hor_res = lv_display_get_horizontal_resolution(m_lvgl_display);
    int32_t ver_res = lv_display_get_vertical_resolution(m_lvgl_display);
    m_logger.Info("Display resolution: %dx%d", hor_res, ver_res);

    // 1. Clear screen and remove default padding/border
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);

    if (is_monochrome) {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x1F1F1F), LV_PART_MAIN); // Dark gray
    }

    // 2. Create Title Label
    lv_obj_t *label = lv_label_create(scr);
    if (label)
    {
        lv_label_set_text(label, is_monochrome ? "MONO TEST" : "COLOR TEST");
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        // Small screens (like 128x64) need smaller margins
        int32_t label_y = (ver_res < 100) ? 2 : 15;
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, label_y);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        
        // Scale font if possible/needed for very small screens
        if (ver_res < 64) {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        }
        m_logger.Info("Test label created");
    }

    if (!is_monochrome) {
        // 3. Color Specific: RGB Bars
        lv_obj_t *container = lv_obj_create(scr);
        int32_t cont_w = (hor_res * 150) / 320;
        int32_t cont_h = (ver_res * 50) / 240;
        if (cont_h < 30) cont_h = 30; // Minimum height

        lv_obj_set_size(container, cont_w, cont_h);
        lv_obj_align(container, LV_ALIGN_CENTER, 0, -ver_res / 20);
        lv_obj_set_style_bg_opa(container, 0, 0);
        lv_obj_set_style_border_opa(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);

        static lv_color_t colors[] = {lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_BLUE)};
        int32_t rect_size = (cont_h * 35) / 50;
        int32_t spacing = (cont_w - 3 * rect_size) / 4;
        if (spacing < 2) spacing = 2;

        for (int i = 0; i < 3; i++) {
            lv_obj_t *rect = lv_obj_create(container);
            lv_obj_set_size(rect, rect_size, rect_size);
            lv_obj_set_style_bg_color(rect, colors[i], 0);
            lv_obj_set_style_border_width(rect, 0, 0);
            lv_obj_set_style_radius(rect, rect_size / 8, 0);
            lv_obj_align(rect, LV_ALIGN_LEFT_MID, i * (rect_size + spacing) + spacing, 0);
        }
    } else {
        // 3. Mono Specific: White Frame
        lv_obj_t *frame = lv_obj_create(scr);
        int32_t frame_w = (hor_res * 80) / 320;
        int32_t frame_h = (ver_res * 50) / 240;
        if (ver_res <= 64) {
            frame_w = 60;
            frame_h = 30;
        }
        lv_obj_set_size(frame, frame_w, frame_h);
        lv_obj_align(frame, LV_ALIGN_CENTER, 0, -ver_res / 24);
        lv_obj_set_style_bg_opa(frame, 0, 0);
        lv_obj_set_style_border_color(frame, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(frame, (ver_res < 100) ? 1 : 2, 0);
        lv_obj_set_style_pad_all(frame, 0, 0);
    }

    // 4. Create Spinner
    lv_obj_t *spinner = lv_spinner_create(scr);
    if (spinner)
    {
        lv_spinner_set_anim_params(spinner, 1000, 60);
        int32_t spin_size = (ver_res * 50) / 240;
        if (spin_size < 20) spin_size = 20; // Minimum size for visibility
        
        lv_obj_set_size(spinner, spin_size, spin_size);
        int32_t spin_y = (ver_res < 100) ? -2 : -15;
        lv_obj_align(spinner, LV_ALIGN_BOTTOM_MID, 0, spin_y);
        
        int32_t arc_w = (spin_size / 8);
        if (arc_w < 2) arc_w = 2;

        if (is_monochrome) {
            lv_obj_set_style_arc_color(spinner, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(spinner, arc_w, LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(spinner, arc_w, LV_PART_MAIN);
        } else {
            lv_obj_set_style_arc_color(spinner, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(spinner, arc_w + 2, LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(spinner, arc_w + 2, LV_PART_MAIN);
            lv_obj_set_style_arc_color(spinner, lv_palette_lighten(LV_PALETTE_GREY, 1), LV_PART_MAIN);
        }
        m_logger.Info("Test spinner created (size: %d)", spin_size);
    }

    Unlock();
    m_logger.Info("LVGL Functional Test Complete");
}

bool LvglPort::SetRotation(lv_display_rotation_t rotation)
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

    // if (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270)
    // {
    //     m_logger.Error("Invalid rotation value. Must be 0, 90, 180, or 270 degrees");
    //     Unlock();
    //     return false;
    // }

    lv_disp_set_rotation(m_lvgl_display, rotation);
    Unlock();
    m_logger.Info("Display rotation set to %d degrees", rotation);
    return true;
}

