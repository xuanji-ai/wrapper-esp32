#include "ni2c_master.hpp"
#include <iomanip>
#include <sstream>

// --- NI2CMasterBus ---

NI2CMasterBus::NI2CMasterBus(NLogger& logger) : m_logger(logger), m_bus_handle(NULL) {
}

NI2CMasterBus::~NI2CMasterBus() {
    Deinit();
}

NLogger& NI2CMasterBus::GetLogger() {
    return m_logger;
}

i2c_master_bus_handle_t NI2CMasterBus::GetHandle() const {
    return m_bus_handle;
}

esp_err_t NI2CMasterBus::Init(const NI2CMasterBusConfig& config) {
    if (m_bus_handle != NULL) {
        m_logger.Warning("Already initialized. Deinitializing first.");
        Deinit();
    }

    esp_err_t ret = i2c_new_master_bus(&config, &m_bus_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Initialized (Port: %d, SDA: %d, SCL: %d)", 
                     config.i2c_port, config.sda_io_num, config.scl_io_num);
    } else {
        m_logger.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t NI2CMasterBus::Deinit() {
    if (m_bus_handle != NULL) {
        esp_err_t ret = i2c_del_master_bus(m_bus_handle);
        if (ret == ESP_OK) {
            m_logger.Info("Deinitialized");
            m_bus_handle = NULL;
        } else {
            m_logger.Error("Failed to deinitialize: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t NI2CMasterBus::Reset() {
    if (m_bus_handle == NULL) {
        m_logger.Error("Cannot reset: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = i2c_master_bus_reset(m_bus_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Reset successfully");
    } else {
        m_logger.Error("Failed to reset: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t NI2CMasterBus::Probe(int addr) {
    return ProbeInternal(addr);
}

esp_err_t NI2CMasterBus::ProbeInternal(int addr) {
    if (m_bus_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Default timeout 50ms for probing
    return i2c_master_probe(m_bus_handle, static_cast<uint16_t>(addr), 50);
}

esp_err_t NI2CMasterBus::Scan(const std::vector<uint8_t>& addrs) {
    esp_err_t last_err = ESP_OK;
    bool found_any = false;
    for (uint8_t addr : addrs) {
        esp_err_t ret = ProbeInternal(addr);
        if (ret == ESP_OK) {
            found_any = true;
            m_logger.Info("Found device at address 0x%02X", addr);
        } else if (ret != ESP_ERR_NOT_FOUND) {
            last_err = ret;
            m_logger.Error("Failed to probe address 0x%02X: %s", addr, esp_err_to_name(ret));
        }
    }
    return (last_err != ESP_OK) ? last_err : (found_any ? ESP_OK : ESP_ERR_NOT_FOUND);
}

esp_err_t NI2CMasterBus::Scan(int start_addr, int end_addr) {
    if (start_addr > end_addr) {
        m_logger.Error("Invalid scan range: 0x%02X - 0x%02X", start_addr, end_addr);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (m_bus_handle == NULL) {
         m_logger.Error("Cannot scan: Not initialized");
         return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Scanning from 0x%02X to 0x%02X...", start_addr, end_addr);
    
    int found_count = 0;
    
    // 打印表头
    m_logger.Info("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    
    for (int i = 0; i < 128; i += 16) {
        // 检查当前行是否在扫描范围内
        if (i + 15 < start_addr || i > end_addr) {
             continue;
        }

        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << i << ":";
        
        for (int j = 0; j < 16; j++) {
            int addr = i + j;
            if (addr < start_addr || addr > end_addr) {
                ss << "   "; // 超出范围
            } else {
                esp_err_t ret = ProbeInternal(addr);
                if (ret == ESP_OK) {
                    ss << " " << std::hex << std::setw(2) << std::setfill('0') << addr;
                    found_count++;
                } else if (ret == ESP_ERR_TIMEOUT) {
                    ss << " UU";
                } else {
                    ss << " --";
                }
            }
        }
        m_logger.Info("%s", ss.str().c_str());
    }
    
    m_logger.Info("Scan complete. Found %d devices.", found_count);
    return (found_count > 0) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t NI2CMasterBus::Scan() {
    return Scan(0x00, 0x7F);
}

// --- NI2CMasterDevice ---

NI2CMasterDevice::NI2CMasterDevice(NLogger& logger) : m_logger(logger), m_dev_handle(NULL) {
}

NI2CMasterDevice::~NI2CMasterDevice() {
    Deinit();
}

NLogger& NI2CMasterDevice::GetLogger() {
    return m_logger;
}

esp_err_t NI2CMasterDevice::Init(const NI2CMasterBus& bus, const NI2CMasterDeviceConfig& config) {
    if (m_dev_handle != NULL) {
        m_logger.Warning("Device already initialized. Deinitializing first.");
        Deinit();
    }

    if (bus.GetHandle() == NULL) {
        m_logger.Error("Bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2c_master_bus_add_device(bus.GetHandle(), &config, &m_dev_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Device initialized (Addr: 0x%02X)", config.device_address);
    } else {
        m_logger.Error("Failed to add device: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t NI2CMasterDevice::Deinit() {
    if (m_dev_handle != NULL) {
        esp_err_t ret = i2c_master_bus_rm_device(m_dev_handle);
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

esp_err_t NI2CMasterDevice::Write(const std::vector<uint8_t>& data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2c_master_transmit(m_dev_handle, data.data(), data.size(), -1);
}

esp_err_t NI2CMasterDevice::Read(std::vector<uint8_t>& data, size_t len) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    data.resize(len);
    return i2c_master_receive(m_dev_handle, data.data(), len, -1);
}

esp_err_t NI2CMasterDevice::WriteRead(const std::vector<uint8_t>& write_data, std::vector<uint8_t>& read_data, size_t read_len) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    read_data.resize(read_len);
    return i2c_master_transmit_receive(m_dev_handle, write_data.data(), write_data.size(), read_data.data(), read_len, -1);
}

esp_err_t NI2CMasterDevice::WriteReg(uint8_t reg_addr, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.reserve(1 + data.size());
    buffer.push_back(reg_addr);
    buffer.insert(buffer.end(), data.begin(), data.end());
    return Write(buffer);
}

esp_err_t NI2CMasterDevice::ReadReg(uint8_t reg_addr, std::vector<uint8_t>& data, size_t len) {
    std::vector<uint8_t> reg = {reg_addr};
    return WriteRead(reg, data, len);
}

esp_err_t NI2CMasterDevice::WriteReg16(uint8_t reg_addr, const std::vector<uint16_t>& data) {
    std::vector<uint8_t> buffer;
    buffer.reserve(1 + data.size() * 2);
    buffer.push_back(reg_addr);
    for (uint16_t val : data) {
        // Big Endian (Network Byte Order) is common for I2C registers
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back(val & 0xFF);
    }
    return Write(buffer);
}

esp_err_t NI2CMasterDevice::ReadReg16(uint8_t reg_addr, std::vector<uint16_t>& data, size_t len) {
    std::vector<uint8_t> read_buffer;
    esp_err_t ret = ReadReg(reg_addr, read_buffer, len * 2);
    if (ret != ESP_OK) {
        return ret;
    }
    
    data.clear();
    data.reserve(len);
    for (size_t i = 0; i < read_buffer.size(); i += 2) {
        uint16_t val = (static_cast<uint16_t>(read_buffer[i]) << 8) | read_buffer[i + 1];
        data.push_back(val);
    }
    return ESP_OK;
}

// --- NI2CMasterLcd ---

NI2CMasterLcd::~NI2CMasterLcd() {
    Deinit();
}

esp_err_t NI2CMasterLcd::Init(const NI2CMasterBus& bus, const NI2CMasterLcdConfig& config) {
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

esp_err_t NI2CMasterLcd::Deinit() {
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

esp_err_t NI2CMasterLcd::Reset() {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t NI2CMasterLcd::TurnOn() {
    return SetDispOnOff(true);
}

esp_err_t NI2CMasterLcd::TurnOff() {
    return SetDispOnOff(false);
}

esp_err_t NI2CMasterLcd::SetDispOnOff(bool on_off) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t NI2CMasterLcd::Mirror(bool mirror_x, bool mirror_y) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t NI2CMasterLcd::SwapXY(bool swap_axes) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t NI2CMasterLcd::SetGap(int x_gap, int y_gap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t NI2CMasterLcd::InvertColor(bool invert) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_invert_color(m_panel_handle, invert);
}

esp_err_t NI2CMasterLcd::Sleep(bool sleep) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}

esp_err_t NI2CMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 16-bit color depth (RGB565)
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t NI2CMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Assuming 8-bit color depth or monochrome packed
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x, y, x + w, y + h, bitmap.data());
}

esp_err_t NI2CMasterLcd::DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap) {
    if (m_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Need to pack bools into bytes/words depending on the display controller's format.
    // This is controller specific. For SSD1306 (monochrome), 1 bit per pixel is often used but usually packed in bytes.
    // esp_lcd_panel_draw_bitmap usually expects a raw buffer matching the configured color depth.
    // For simplicity, let's assume we convert bools to 0x00/0xFF bytes (8-bit mode) or 0x0000/0xFFFF (16-bit mode).
    // But since we don't know the BPP here easily without checking config (which we don't store), 
    // let's try to assume monochrome packed for SSD1306 if possible, OR just fail for now/convert to uint8_t.
    
    // A safer generic implementation would be to convert to the most common format.
    // Let's assume we convert to uint8_t (0=off, 1=on) and let the driver handle it if it supports 8-bit palette or similar?
    // Actually, SSD1306 driver in ESP-IDF expects packed bits.
    
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
