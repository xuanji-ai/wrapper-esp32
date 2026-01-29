#include "nspi_master.hpp"
#include <cstring>

// --- NSpiMasterBus ---

NSpiMasterBus::NSpiMasterBus(NLogger& logger) : m_logger(logger), m_host_id(SPI2_HOST), m_initialized(false), 
    m_config(SPI2_HOST, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, 4092, SPI_DMA_CH_AUTO, SPICOMMON_BUSFLAG_MASTER) {
}

NSpiMasterBus::~NSpiMasterBus() {
    Deinit();
}

NLogger& NSpiMasterBus::GetLogger() {
    return m_logger;
}

spi_host_device_t NSpiMasterBus::GetHostId() const {
    return m_host_id;
}

esp_err_t NSpiMasterBus::Init(const NSpiMasterBusConfig& config) {
    if (m_initialized) {
        m_logger.Warning("Already initialized. Deinitializing first.");
        Deinit();
    }
    
    // Save config for Reset
    m_config = config;

    esp_err_t ret = spi_bus_initialize(config.host_id, &config, config.dma_chan);
    if (ret == ESP_OK) {
        m_host_id = config.host_id;
        m_initialized = true;
        m_logger.Info("Initialized (Host: %d, MOSI: %d, MISO: %d, SCLK: %d)", 
                     config.host_id, config.mosi_io_num, config.miso_io_num, config.sclk_io_num);
    } else {
        m_logger.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t NSpiMasterBus::Deinit() {
    if (m_initialized) {
        esp_err_t ret = spi_bus_free(m_host_id);
        if (ret == ESP_OK) {
            m_logger.Info("Deinitialized");
            m_initialized = false;
        } else {
            m_logger.Error("Failed to deinitialize: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t NSpiMasterBus::Reset() {
    if (!m_initialized) {
        m_logger.Error("Cannot reset: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Resetting...");
    
    // Deinit current bus
    esp_err_t ret = Deinit();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Re-init with saved config
    return Init(m_config);
}

// --- NSpiMasterDevice ---

NSpiMasterDevice::NSpiMasterDevice(NLogger& logger) : m_logger(logger), m_dev_handle(NULL) {
}

NSpiMasterDevice::~NSpiMasterDevice() {
    Deinit();
}

NLogger& NSpiMasterDevice::GetLogger() {
    return m_logger;
}

esp_err_t NSpiMasterDevice::Init(const NSpiMasterBus& bus, const NSpiMasterDeviceConfig& config) {
    if (m_dev_handle != NULL) {
        m_logger.Warning("Device already initialized. Deinitializing first.");
        Deinit();
    }

    // SPI bus must be initialized before adding device
    // We check this implicitly by bus.GetHostId(), but really the user should ensure bus is Init'd.
    // Unlike I2C new driver, SPI driver relies on Host ID.

    esp_err_t ret = spi_bus_add_device(bus.GetHostId(), &config, &m_dev_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Device initialized (CS: %d, Speed: %d Hz)", config.spics_io_num, config.clock_speed_hz);
    } else {
        m_logger.Error("Failed to add device: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t NSpiMasterDevice::Deinit() {
    if (m_dev_handle != NULL) {
        esp_err_t ret = spi_bus_remove_device(m_dev_handle);
        if (ret == ESP_OK) {
            m_logger.Info("Device deinitialized");
            m_dev_handle = NULL;
        } else {
            m_logger.Error("Failed to remove device: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t NSpiMasterDevice::Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = tx_data.size() * 8; // length is in bits
    t.tx_buffer = tx_data.data();
    
    rx_data.resize(tx_data.size());
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(m_dev_handle, &t);
}

esp_err_t NSpiMasterDevice::Write(const std::vector<uint8_t>& data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = data.size() * 8;
    t.tx_buffer = data.data();
    t.rx_buffer = NULL; // No receive

    return spi_device_transmit(m_dev_handle, &t);
}

esp_err_t NSpiMasterDevice::Read(size_t len, std::vector<uint8_t>& rx_data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = len * 8;
    t.tx_buffer = NULL; // No transmit (will send 0s or random depending on half-duplex settings, usually 0 if not set)
    
    rx_data.resize(len);
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(m_dev_handle, &t);
}

// --- NSpiMasterLcd ---

NSpiMasterLcd::~NSpiMasterLcd() {
    Deinit();
}

esp_err_t NSpiMasterLcd::Init(const NSpiMasterBus& bus, const NSpiMasterLcdConfig& config) {
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

    m_logger.Info("LCD initialized (CS: %d)", config.io_config.cs_gpio_num);
    return ESP_OK;
}

esp_err_t NSpiMasterLcd::Deinit() {
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

esp_err_t NSpiMasterLcd::Reset() {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t NSpiMasterLcd::TurnOn() {
    return SetDispOnOff(true);
}

esp_err_t NSpiMasterLcd::TurnOff() {
    return SetDispOnOff(false);
}

esp_err_t NSpiMasterLcd::SetDispOnOff(bool on_off) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t NSpiMasterLcd::Mirror(bool mirror_x, bool mirror_y) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t NSpiMasterLcd::SwapXY(bool swap_axes) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t NSpiMasterLcd::SetGap(int x_gap, int y_gap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t NSpiMasterLcd::InvertColor(bool invert) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_invert_color(m_panel_handle, invert);
}

esp_err_t NSpiMasterLcd::Sleep(bool sleep) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}

esp_err_t NSpiMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 16-bit color depth (RGB565)
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t NSpiMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 8-bit color depth or monochrome packed
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t NSpiMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap) {
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
