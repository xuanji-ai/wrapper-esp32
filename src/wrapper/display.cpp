#include "wrapper/display.hpp"

using namespace wrapper;

// --- Display ---

Display::Display(Logger& logger) : m_logger(logger), m_io_handle(NULL), m_panel_handle(NULL) {
}

Display::~Display() {
    Deinit();
}

esp_err_t Display::Init(const I2cBus& bus, const I2cLcdConfig& config, LcdNewPanelFunc new_panel_func, CustomLcdPanelInitFunc custom_init_panel_func)
{
    esp_err_t ret = InitIo(bus, config.io_config);
    if (ret != ESP_OK) return ret;

    // Create Panel handle
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t Display::Init(const SpiBus& bus, const SpiLcdConfig& config, LcdNewPanelFunc new_panel_func, CustomLcdPanelInitFunc custom_init_panel_func)
{
    esp_err_t ret = InitIo(bus, config.io_config);
    if (ret != ESP_OK) return ret;

    // Create Panel handle
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t Display::InitIo(const I2cBus& bus, const esp_lcd_panel_io_i2c_config_t& config)
{
    if (m_io_handle != NULL) {
        m_logger.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus.GetHandle() == NULL) {
        m_logger.Error("Bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // 1. Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_i2c(bus.GetHandle(), &config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD I2C IO initialized (Addr: 0x%02X)", config.dev_addr);
    return ESP_OK;
}

esp_err_t Display::InitIo(const SpiBus& bus, const esp_lcd_panel_io_spi_config_t& config) {
    if (m_io_handle != NULL) {
        m_logger.Warning("Display IO already initialized. Deinitializing first.");
        Deinit();
    }

    // 1. Create IO handle
    esp_err_t ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)bus.GetHostId(), &config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD SPI IO initialized (CS: %d)", config.cs_gpio_num);
    return ESP_OK;
}

esp_err_t Display::InitPanel(const esp_lcd_panel_dev_config_t& panel_config, CustomLcdPanelInitFunc custom_init_panel_func)
{
    if (m_io_handle == NULL) {
        m_logger.Error("IO handle not initialized. Call InitIo first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_panel_handle == NULL) {
        m_logger.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // 3. Reset the display
    esp_err_t ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK) {
         m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
         return ret;
    }

    // 4. Initialize the display
    if (custom_init_panel_func == nullptr) {
        ret = esp_lcd_panel_init(m_panel_handle);
    } else {
        ret = custom_init_panel_func(m_io_handle);
    }

    if (ret != ESP_OK) {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // 5. Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LCD panel initialized");
    return ESP_OK;
}

esp_err_t Display::Deinit() {
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
        m_logger.Info("Display deinitialized");
    }
    return ret;
}

esp_err_t Display::Reset() {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t Display::TurnOn() {
    return SetDispOnOff(true);
}

esp_err_t Display::TurnOff() {
    return SetDispOnOff(false);
}

esp_err_t Display::SetDispOnOff(bool on_off) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t Display::Mirror(bool mirror_x, bool mirror_y) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t Display::SwapXY(bool swap_axes) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t Display::SetGap(int x_gap, int y_gap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t Display::InvertColor(bool invert) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_invert_color(m_panel_handle, invert);
}

esp_err_t Display::Sleep(bool sleep) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}

esp_err_t Display::DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 16-bit color depth (RGB565)
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t Display::DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 8-bit color depth or monochrome packed
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t Display::DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap) {
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

bool Display::TestColors(bool monochrome) {
    if (m_panel_handle == NULL) {
        m_logger.Error("Display not initialized");
        return false;
    }

    const int width = 128;
    const int height = 128;
    
    if (monochrome) {
        m_logger.Info("Starting color test for monochrome display (1 bit per pixel)...");
        const size_t buffer_size = (width * height + 7) / 8;
        
        struct TestPattern {
            const char* name;
            uint8_t fill_value;
        };
        
        TestPattern patterns[] = {
            {"All BLACK (0x00)", 0x00},
            {"All WHITE (0xFF)", 0xFF},
            {"Checkerboard 1 (0xAA)", 0xAA},
            {"Checkerboard 2 (0x55)", 0x55},
            {"Horizontal Lines (0xF0)", 0xF0},
            {"Vertical Pattern (0xCC)", 0xCC},
        };
        
        int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
        
        for (int cycle = 0; cycle < 2; cycle++) {
            m_logger.Info("Test cycle %d/%d", cycle + 1, 2);
            for (int i = 0; i < num_patterns; i++) {
                m_logger.Info("Displaying: %s", patterns[i].name);
                std::vector<uint8_t> buffer(buffer_size, patterns[i].fill_value);
                esp_err_t ret = esp_lcd_panel_draw_bitmap(m_panel_handle, 0, 0, width, height, buffer.data());
                if (ret != ESP_OK) {
                    m_logger.Error("Failed to draw pattern '%s': %s", patterns[i].name, esp_err_to_name(ret));
                    return false;
                }
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
        }
    } else {
        m_logger.Info("Starting color test for color display (RGB565, 16 bits per pixel)...");
        const size_t num_pixels = width * height;
        
        struct ColorPattern {
            const char* name;
            uint16_t color; // RGB565
        };
        
        ColorPattern patterns[] = {
            {"RED", 0xF800},
            {"GREEN", 0x07E0},
            {"BLUE", 0x001F},
            {"YELLOW", 0xFFE0},
            {"CYAN", 0x07FF},
            {"MAGENTA", 0xF81F},
            {"WHITE", 0xFFFF},
            {"BLACK", 0x0000},
        };
        
        int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
        
        for (int cycle = 0; cycle < 2; cycle++) {
            m_logger.Info("Test cycle %d/%d", cycle + 1, 2);
            for (int i = 0; i < num_patterns; i++) {
                m_logger.Info("Displaying: %s", patterns[i].name);
                std::vector<uint16_t> buffer(num_pixels, patterns[i].color);
                esp_err_t ret = esp_lcd_panel_draw_bitmap(m_panel_handle, 0, 0, width, height, buffer.data());
                if (ret != ESP_OK) {
                    m_logger.Error("Failed to draw color '%s': %s", patterns[i].name, esp_err_to_name(ret));
                    return false;
                }
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
        }
    }
    
    m_logger.Info("Color test completed successfully");
    return true;
}

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

// --- DsiDisplay ---

esp_err_t DsiDisplay::Init(
  const DsiLcdConfig &config, 
  std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
  std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func,
  std::function<void(void* vender_conf)> vender_conf_init_func)
{
    // 1. Initialize DSI bus
    esp_err_t ret = InitDsiBus(config.dsi_bus_config);
    if (ret != ESP_OK) return ret;

    // 2. Initialize DBI IO
    ret = InitDbiIo(config.dbi_io_config);
    if (ret != ESP_OK) return ret;

    // Vendor specific configuration initialization
    if ( config.panel_config.vendor_config != nullptr && vender_conf_init_func != nullptr) {
        vender_conf_init_func(config.panel_config.vendor_config);
    }

    // 3. Create Panel handle with vendor config
    // Note: Vendor config needs to be set up by caller with dsi_bus and dpi_config
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create DSI LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Initialize panel
    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t DsiDisplay::InitDsiBus(const esp_lcd_dsi_bus_config_t &config)
{
    if (m_dsi_bus_handle != nullptr) {
        m_logger.Warning("DSI bus already initialized. Deinitializing first.");
        Deinit();
    }

    // Create MIPI DSI bus
    esp_err_t ret = esp_lcd_new_dsi_bus(&config, &m_dsi_bus_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create DSI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("MIPI DSI bus initialized (lanes: %d, bitrate: %d Mbps)", 
                  config.num_data_lanes, config.lane_bit_rate_mbps);
    return ESP_OK;
}

esp_err_t DsiDisplay::InitDbiIo(const esp_lcd_dbi_io_config_t &config)
{
    if (m_io_handle != nullptr) {
        m_logger.Warning("DBI IO already initialized.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_dsi_bus_handle == nullptr) {
        m_logger.Error("DSI bus not initialized. Call InitDsiBus first.");
        return ESP_ERR_INVALID_STATE;
    }

    // Create DBI panel IO
    esp_err_t ret = esp_lcd_new_panel_io_dbi(m_dsi_bus_handle, &config, &m_io_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to create DBI panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("DBI panel IO initialized (vc: %d)", config.virtual_channel);
    return ESP_OK;
}

esp_err_t DsiDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func)
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

    m_logger.Info("DSI LCD panel initialized");
    return ESP_OK;
}

esp_err_t DsiDisplay::Deinit()
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

    if (m_dsi_bus_handle != nullptr) {
        esp_err_t r = esp_lcd_del_dsi_bus(m_dsi_bus_handle);
        if (r != ESP_OK) {
            m_logger.Error("Failed to delete DSI bus: %s", esp_err_to_name(r));
            if (ret == ESP_OK) ret = r;
        }
        m_dsi_bus_handle = nullptr;
    }
    
    if (ret == ESP_OK) {
        m_logger.Info("DSI Display deinitialized");
    }
    
    return ret;
}
