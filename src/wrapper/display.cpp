#include "wrapper/display.hpp"

using namespace wrapper;

bool I2cDisplay::InitIo(const esp_lcd_panel_io_i2c_config_t &config)
{
    if (io_handle_ != nullptr)
        return true;

    esp_err_t ret = esp_lcd_new_panel_io_i2c(bus_.GetHandle(), &config, &io_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to create I2C panel IO: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool I2cDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (custom_init_panel_func != nullptr)
    {
        esp_err_t ret = custom_init_panel_func(io_handle_);
        if (ret != ESP_OK)
        {
            logger_.Error("Failed to custom init panel: %s", esp_err_to_name(ret));
            return false;
        }
    }

    esp_err_t ret = esp_lcd_panel_reset(panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to reset panel: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_lcd_panel_init(panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to init panel: %s", esp_err_to_name(ret));
        return false;
    }

    return true;
}

bool I2cDisplay::Init(
    const I2cDisplayConfig &config,
    std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func,
    std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (!InitIo(config.io_config))
        return false;

    esp_err_t ret = new_panel_func(io_handle_, &config.panel_config, &panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to create new panel: %s", esp_err_to_name(ret));
        return false;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

bool I2cDisplay::Deinit()
{
    if (panel_handle_ != nullptr)
    {
        if (esp_lcd_panel_del(panel_handle_) != ESP_OK)
        {
            logger_.Error("Failed to delete panel");
            return false;
        }
        panel_handle_ = nullptr;
    }

    if (io_handle_ != nullptr)
    {
        if (esp_lcd_panel_io_del(io_handle_) != ESP_OK)
        {
            logger_.Error("Failed to delete panel IO");
            return false;
        }
        io_handle_ = nullptr;
    }
    return true;
}

bool SpiDisplay::InitIo(const esp_lcd_panel_io_spi_config_t &config)
{
    if (io_handle_ != nullptr)
        return true;

    esp_err_t ret = esp_lcd_new_panel_io_spi(bus_.GetHostId(), &config, &io_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to create SPI panel IO: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool SpiDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (custom_init_panel_func != nullptr)
    {
        esp_err_t ret = custom_init_panel_func(io_handle_);
        if (ret != ESP_OK)
        {
            logger_.Error("Failed to custom init panel: %s", esp_err_to_name(ret));
            return false;
        }
    }

    esp_err_t ret = esp_lcd_panel_reset(panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to reset panel: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_lcd_panel_init(panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to init panel: %s", esp_err_to_name(ret));
        return false;
    }

    return true;
}

bool SpiDisplay::Init(const SpiDisplayConfig &config,
                      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func,
                      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func)
{
    if (!InitIo(config.io_config))
        return false;

    esp_err_t ret = new_panel_func(io_handle_, &config.panel_config, &panel_handle_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to create new panel: %s", esp_err_to_name(ret));
        return false;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

bool SpiDisplay::Deinit()
{
    if (panel_handle_ != nullptr)
    {
        if (esp_lcd_panel_del(panel_handle_) != ESP_OK)
        {
            logger_.Error("Failed to delete panel");
            return false;
        }
        panel_handle_ = nullptr;
    }

    if (io_handle_ != nullptr)
    {
        if (esp_lcd_panel_io_del(io_handle_) != ESP_OK)
        {
            logger_.Error("Failed to delete panel IO");
            return false;
        }
        io_handle_ = nullptr;
    }
    return true;
}
