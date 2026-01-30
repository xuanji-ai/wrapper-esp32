#include "ntouch.hpp"

using namespace nix;

I2cTouch::I2cTouch(Logger& logger) : m_logger(logger), m_io_handle(NULL), m_touch_handle(NULL)
{
}

I2cTouch::~I2cTouch()
{
    Deinit();
}

esp_err_t I2cTouch::Init(const I2cBus& bus, const I2cTouchConfig& config, I2cTouchNewFunc new_touch_func)
{
    if (m_io_handle != NULL || m_touch_handle != NULL)
    {
        m_logger.Warning("Touch already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus.GetHandle() == NULL)
    {
        m_logger.Error("Bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // 1. Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_i2c(bus.GetHandle(), &config.io_config, &m_io_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to create Touch IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Create Touch handle
    // Use the provided factory function to create the specific touch controller handle
    ret = new_touch_func(m_io_handle, &config.touch_config, &m_touch_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to create Touch handle: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(m_io_handle);
        m_io_handle = NULL;
        return ret;
    }

    m_logger.Info("Touch initialized (Addr: 0x%02X)", config.io_config.dev_addr);
    return ESP_OK;
}

esp_err_t I2cTouch::Deinit()
{
    esp_err_t ret = ESP_OK;
    if (m_touch_handle != NULL)
    {
        esp_err_t r = esp_lcd_touch_del(m_touch_handle);
        if (r != ESP_OK)
        {
            m_logger.Error("Failed to delete touch handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_touch_handle = NULL;
    }

    if (m_io_handle != NULL)
    {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK)
        {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK)
                ret = r;
        }
        m_io_handle = NULL;
    }

    if (ret == ESP_OK)
    {
        m_logger.Info("Touch deinitialized");
    }
    return ret;
}

esp_err_t I2cTouch::ReadData()
{
    if (m_touch_handle == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_touch_read_data(m_touch_handle);
}

esp_err_t I2cTouch::GetData(esp_lcd_touch_point_data_t *data, uint8_t *point_cnt, uint8_t max_point_cnt)
{
    if (m_touch_handle == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_touch_get_data(m_touch_handle, data, point_cnt, max_point_cnt);
}

bool I2cTouch::GetCoordinates(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    if (m_touch_handle == NULL)
    {
        return false;
    }
    
    // Use new API and convert to old format for backward compatibility
    esp_lcd_touch_point_data_t data[max_point_num];
    uint8_t cnt = 0;
    esp_err_t ret = esp_lcd_touch_get_data(m_touch_handle, data, &cnt, max_point_num);
    
    if (ret != ESP_OK)
    {
        return false;
    }
    
    if (point_num != NULL)
    {
        *point_num = cnt;
    }
    
    for (uint8_t i = 0; i < cnt; i++)
    {
        if (x != NULL) x[i] = data[i].x;
        if (y != NULL) y[i] = data[i].y;
        if (strength != NULL) strength[i] = data[i].strength;
    }
    
    return (cnt > 0);
}
