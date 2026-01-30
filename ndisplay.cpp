#include "ndisplay.hpp"

using namespace nix;
// --- I2cLcd ---

I2cLcd::I2cLcd(Logger& logger) : m_logger(logger), m_io_handle(NULL), m_panel_handle(NULL) {
}

I2cLcd::~I2cLcd() {
    Deinit();
}

esp_err_t I2cLcd::Init(const I2cBus& bus, const I2cLcdConfig& config) {
    if (m_io_handle != NULL || m_panel_handle != NULL) {
        m_logger.Warning("LCD already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus.GetHandle() == NULL) {
        m_logger.Error("Bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // 1. Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_i2c(bus.GetHandle(), &config.io_config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Create Panel handle
    // Use the provided function to create the panel handle
    ret = config.new_panel_func_(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(m_io_handle);
        m_io_handle = NULL;
        return ret;
    }

    // 3. Reset the display
    ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK) {
         m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
         Deinit();
         return ret;
    }

    // 4. Initialize the display
    ret = esp_lcd_panel_init(m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        Deinit();
        return ret;
    }

    // 5. Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        Deinit();
        return ret;
    }

    m_logger.Info("LCD initialized (Addr: 0x%02X)", config.io_config.dev_addr);
    return ESP_OK;
}

esp_err_t I2cLcd::Deinit() {
    esp_err_t ret = ESP_OK;
    if (m_panel_handle != NULL) {
        esp_err_t r = esp_lcd_panel_del(m_panel_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_panel_handle = NULL;
    }
    
    if (m_io_handle != NULL) {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        m_io_handle = NULL;
    }
    
    if (ret == ESP_OK) {
        m_logger.Info("LCD deinitialized");
    }
    return ret;
}

esp_err_t I2cLcd::Reset() {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t I2cLcd::TurnOn() {
    return SetDispOnOff(true);
}

esp_err_t I2cLcd::TurnOff() {
    return SetDispOnOff(false);
}

esp_err_t I2cLcd::SetDispOnOff(bool on_off) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t I2cLcd::Mirror(bool mirror_x, bool mirror_y) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t I2cLcd::SwapXY(bool swap_axes) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t I2cLcd::SetGap(int x_gap, int y_gap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t I2cLcd::InvertColor(bool invert) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_invert_color(m_panel_handle, invert);
}

esp_err_t I2cLcd::Sleep(bool sleep) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}

esp_err_t I2cLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 16-bit color depth (RGB565)
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t I2cLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 8-bit color depth or monochrome packed
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t I2cLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    size_t byte_size = (w * h + 7) / 8;
    std::vector<uint8_t> packed_bitmap(byte_size, 0);
    
    for (int i = 0; i < w * h; ++i) {
        if (i < static_cast<int>(bitmap.size()) && bitmap[i]) {
            packed_bitmap[i / 8] |= (1 << (i % 8)); // Little endian bit packing
        }
    }
    
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, packed_bitmap.data());
}

// --- SpiLcd ---

SpiLcd::~SpiLcd() {
    Deinit();
}

esp_err_t SpiLcd::Init(const SpiBus& bus, const SpiLcdConfig& config, SpiLcdNewPanelFunc new_panel_func) {
    if (m_io_handle != NULL || m_panel_handle != NULL) {
        m_logger.Warning("LCD already initialized. Deinitializing first.");
        Deinit();
    }

    // 1. Create IO handle
    // Cast spi_host_device_t to esp_lcd_spi_bus_handle_t (which is int)
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)bus.GetHostId(), &config.io_config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Create Panel handle
    // Use the provided function to create the panel handle
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(m_io_handle);
        m_io_handle = NULL;
        return ret;
    }

    // 3. Reset the display
    ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK) {
         m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
         Deinit();
         return ret;
    }

    // 4. Initialize the display
    ret = esp_lcd_panel_init(m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        Deinit();
        return ret;
    }

    // 5. Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        Deinit();
        return ret;
    }

    m_logger.Info("LCD initialized (CS: %d)", config.io_config.cs_gpio_num);
    return ESP_OK;
}

esp_err_t SpiLcd::Deinit() {
    esp_err_t ret = ESP_OK;
    if (m_panel_handle != NULL) {
        esp_err_t r = esp_lcd_panel_del(m_panel_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_panel_handle = NULL;
    }
    
    if (m_io_handle != NULL) {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        m_io_handle = NULL;
    }
    
    if (ret == ESP_OK) {
        m_logger.Info("LCD deinitialized");
    }
    return ret;
}

esp_err_t SpiLcd::Reset() {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t SpiLcd::TurnOn() {
    return SetDispOnOff(true);
}

esp_err_t SpiLcd::TurnOff() {
    return SetDispOnOff(false);
}

esp_err_t SpiLcd::SetDispOnOff(bool on_off) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t SpiLcd::Mirror(bool mirror_x, bool mirror_y) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t SpiLcd::SwapXY(bool swap_axes) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t SpiLcd::SetGap(int x_gap, int y_gap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t SpiLcd::InvertColor(bool invert) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_invert_color(m_panel_handle, invert);
}

esp_err_t SpiLcd::Sleep(bool sleep) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}

esp_err_t SpiLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 16-bit color depth (RGB565)
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t SpiLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 8-bit color depth or monochrome packed
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t SpiLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Implementation for packed bits (monochrome):
    size_t byte_size = (w * h + 7) / 8;
    std::vector<uint8_t> packed_bitmap(byte_size, 0);
    
    for (int i = 0; i < w * h; ++i) {
        if (i < static_cast<int>(bitmap.size()) && bitmap[i]) {
            packed_bitmap[i / 8] |= (1 << (i % 8)); // Little endian bit packing
        }
    }
    
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, packed_bitmap.data());
}