#include "wrapper/display-new.hpp"

// --- I2cDisplay ---

esp_err_t I2cDisplay::Init(
      const I2cLcdConfig &config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    esp_err_t ret = InitIo(config.io_config);
    if (ret != ESP_OK) return ret;

    // Create Panel handle
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t I2cDisplay::InitIo(const esp_lcd_panel_io_i2c_config_t &config)
{
    if (m_io_handle != nullptr) {
        m_logger.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    if (m_bus.GetHandle() == nullptr) {
        m_logger.Error("I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_i2c(m_bus.GetHandle(), &config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD I2C IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD I2C IO initialized (Addr: 0x%02X)", config.dev_addr);
    return ESP_OK;
}

esp_err_t I2cDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func)
{
    if (m_io_handle == nullptr) {
        m_logger.Error("IO handle not initialized. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_panel_handle == nullptr) {
        m_logger.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // Reset the display
    esp_err_t ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the display
    if (custom_init_panel_func == nullptr) {
        ret = esp_lcd_panel_init(m_panel_handle);
    } else {
        ret = custom_init_panel_func(m_io_handle);
    }

    if (ret != ESP_OK) {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD panel initialized");
    return ESP_OK;
}

esp_err_t I2cDisplay::Deinit()
{
    esp_err_t ret = ESP_OK;
    
    if (m_panel_handle != nullptr) {
        esp_err_t r = esp_lcd_panel_del(m_panel_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_panel_handle = nullptr;
    }
    
    if (m_io_handle != nullptr) {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        m_io_handle = nullptr;
    }
    
    if (ret == ESP_OK) {
        m_logger.Info("I2C Display deinitialized");
    }
    
    return ret;
}

// --- SpiDisplay ---

esp_err_t SpiDisplay::Init(const SpiLcdConfig &config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    esp_err_t ret = InitIo(config.io_config);
    if (ret != ESP_OK) return ret;

    // Create Panel handle
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t SpiDisplay::InitIo(const esp_lcd_panel_io_spi_config_t &config)
{
    if (m_io_handle != nullptr) {
        m_logger.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    // Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)m_bus.GetHostId(), &config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD SPI IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD SPI IO initialized (CS: %d)", config.cs_gpio_num);
    return ESP_OK;
}

esp_err_t SpiDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func)
{
    if (m_io_handle == nullptr) {
        m_logger.Error("IO handle not initialized. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_panel_handle == nullptr) {
        m_logger.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // Reset the display
    esp_err_t ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the display
    if (custom_init_panel_func == nullptr) {
        ret = esp_lcd_panel_init(m_panel_handle);
    } else {
        ret = custom_init_panel_func(m_io_handle);
    }

    if (ret != ESP_OK) {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD panel initialized");
    return ESP_OK;
}

esp_err_t SpiDisplay::Deinit()
{
    esp_err_t ret = ESP_OK;
    
    if (m_panel_handle != nullptr) {
        esp_err_t r = esp_lcd_panel_del(m_panel_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_panel_handle = nullptr;
    }
    
    if (m_io_handle != nullptr) {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        m_io_handle = nullptr;
    }
    
    if (ret == ESP_OK) {
        m_logger.Info("SPI Display deinitialized");
    }
    
    return ret;
}