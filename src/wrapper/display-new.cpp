#include "wrapper/display-new.hpp"

// --- I2cDisplay ---

namespace wrapper
{

esp_err_t I2cDisplay::Init(
      const I2cLcdConfig &config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    esp_err_t ret = InitIo(config.io_config);
    if (ret != ESP_OK) return ret;

    // Create Panel handle
    ret = new_panel_func(io_handle_, &config.panel_config, &panel_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t I2cDisplay::InitIo(const esp_lcd_panel_io_i2c_config_t &config)
{
    if (io_handle_ != nullptr) {
        logger_.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus_.GetHandle() == nullptr) {
        logger_.Error("I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_i2c(bus_.GetHandle(), &config, &io_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to create LCD I2C IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Info("LCD I2C IO initialized (Addr: 0x%02X)", config.dev_addr);
    return ESP_OK;
}

esp_err_t I2cDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (io_handle_ == nullptr) {
        logger_.Error("IO handle not initialized. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (panel_handle_ == nullptr) {
        logger_.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // Reset the display
    esp_err_t ret = esp_lcd_panel_reset(panel_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the display
    if (custom_init_panel_func == nullptr) {
        ret = esp_lcd_panel_init(panel_handle_);
    } else {
        ret = custom_init_panel_func(io_handle_);
    }

    if (ret != ESP_OK) {
        logger_.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Turn on the display
    ret = esp_lcd_panel_disp_on_off(panel_handle_, true);
    if (ret != ESP_OK) {
        logger_.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Info("LCD panel initialized");
    return ESP_OK;
}

esp_err_t I2cDisplay::Deinit()
{
    esp_err_t ret = ESP_OK;
    
    if (panel_handle_ != nullptr) {
        esp_err_t r = esp_lcd_panel_del(panel_handle_);
        if (r != ESP_OK) {
            logger_.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        panel_handle_ = nullptr;
    }
    
    if (io_handle_ != nullptr) {
        esp_err_t r = esp_lcd_panel_io_del(io_handle_);
        if (r != ESP_OK) {
            logger_.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        io_handle_ = nullptr;
    }
    
    if (ret == ESP_OK) {
        logger_.Info("I2C Display deinitialized");
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
    ret = new_panel_func(io_handle_, &config.panel_config, &panel_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t SpiDisplay::InitIo(const esp_lcd_panel_io_spi_config_t &config)
{
    if (io_handle_ != nullptr) {
        logger_.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    // Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)bus_.GetHostId(), &config, &io_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to create LCD SPI IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Info("LCD SPI IO initialized (CS: %d)", config.cs_gpio_num);
    return ESP_OK;
}

esp_err_t SpiDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config,
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (io_handle_ == nullptr) {
        logger_.Error("IO handle not initialized. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (panel_handle_ == nullptr) {
        logger_.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // Reset the display
    esp_err_t ret = esp_lcd_panel_reset(panel_handle_);
    if (ret != ESP_OK) {
        logger_.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the display
    if (custom_init_panel_func == nullptr) {
        ret = esp_lcd_panel_init(panel_handle_);
    } else {
        ret = custom_init_panel_func(io_handle_);
    }

    if (ret != ESP_OK) {
        logger_.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Turn on the display
    ret = esp_lcd_panel_disp_on_off(panel_handle_, true);
    if (ret != ESP_OK) {
        logger_.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Info("LCD panel initialized");
    return ESP_OK;
}

esp_err_t SpiDisplay::Deinit()
{
    esp_err_t ret = ESP_OK;
    
    if (panel_handle_ != nullptr) {
        esp_err_t r = esp_lcd_panel_del(panel_handle_);
        if (r != ESP_OK) {
            logger_.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        panel_handle_ = nullptr;
    }
    
    if (io_handle_ != nullptr) {
        esp_err_t r = esp_lcd_panel_io_del(io_handle_);
        if (r != ESP_OK) {
            logger_.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        io_handle_ = nullptr;
    }
    
    if (ret == ESP_OK) {
        logger_.Info("SPI Display deinitialized");
    }
    
    return ret;
}

} // namespace wrapper