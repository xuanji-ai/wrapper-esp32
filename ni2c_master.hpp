#pragma once

#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ssd1306.h"
#include "nlogger.hpp"
#include <vector>
#include <functional>

struct NI2CMasterBusConfig : public i2c_master_bus_config_t
{
public:
    /*
     * port: I2C_NUM_0
     * sda: GPIO_NUM_NC
     * scl: GPIO_NUM_NC
     * clk_src: I2C_CLK_SRC_DEFAULT
     * glitch_ignore: 7
     * intr_prio: 0
     * queue_depth: 0
     * enable_pullup: true
     * enable_pd: false
     */
    NI2CMasterBusConfig(i2c_port_num_t port, 
                        gpio_num_t sda, 
                        gpio_num_t scl,
                        i2c_clock_source_t clk_src,
                        uint8_t glitch_ignore,
                        int intr_prio,
                        size_t queue_depth,
                        bool enable_pullup,
                        bool enable_pd)
    {
        i2c_port = port;
        sda_io_num = sda;
        scl_io_num = scl;
        clk_source = clk_src;
        glitch_ignore_cnt = glitch_ignore;
        intr_priority = intr_prio;
        trans_queue_depth = queue_depth;
        flags.enable_internal_pullup = enable_pullup;
        flags.allow_pd = enable_pd;
    }
};

class NI2CMasterBus
{
    NLogger& m_logger;
    i2c_master_bus_handle_t m_bus_handle;
    
    // 私有方法：实际执行Probe的逻辑
    esp_err_t ProbeInternal(int addr);

public:
    NI2CMasterBus(NLogger& logger);
    ~NI2CMasterBus();
    NLogger& GetLogger();
    esp_err_t Init(const NI2CMasterBusConfig& config);
    esp_err_t Deinit();
    esp_err_t Reset();
    i2c_master_bus_handle_t GetHandle() const;
    
    esp_err_t Probe(int addr);
    
    template<typename... Args>
    esp_err_t Scan(Args... args) {
        std::vector<uint8_t> addrs = {static_cast<uint8_t>(args)...};
        return Scan(addrs);
    }
    
    esp_err_t Scan(const std::vector<uint8_t>& addrs);
    esp_err_t Scan(int start_addr, int end_addr);
    esp_err_t Scan();
};

struct NI2CMasterDeviceConfig : public i2c_device_config_t 
{
    /*
     * speed_hz: 100000
     */
    NI2CMasterDeviceConfig(uint8_t addr, uint32_t speed_hz)
    {
        dev_addr_length = I2C_ADDR_BIT_LEN_7;
        device_address = addr;
        scl_speed_hz = speed_hz;
        scl_wait_us = 0;
        flags.disable_ack_check = 0;
    }
};

class NI2CMasterDevice
{
protected:
    NLogger& m_logger;
    i2c_master_dev_handle_t m_dev_handle;
    
public:
    NI2CMasterDevice(NLogger& logger);
    ~NI2CMasterDevice();
    NLogger& GetLogger();
    esp_err_t Init(const NI2CMasterBus& bus, const NI2CMasterDeviceConfig& config);
    esp_err_t Deinit();

    esp_err_t Write(const std::vector<uint8_t>& data);
    esp_err_t Read(std::vector<uint8_t>& data, size_t len);
    esp_err_t WriteRead(const std::vector<uint8_t>& write_data, std::vector<uint8_t>& read_data, size_t read_len);
    
    // Register operations
    esp_err_t WriteReg(uint8_t reg_addr, const std::vector<uint8_t>& data);
    esp_err_t ReadReg(uint8_t reg_addr, std::vector<uint8_t>& data, size_t len);
    esp_err_t WriteReg16(uint8_t reg_addr, const std::vector<uint16_t>& data);
    esp_err_t ReadReg16(uint8_t reg_addr, std::vector<uint16_t>& data, size_t len);
};

// Define the function pointer type for creating a new panel
using NI2CMasterLcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t*, esp_lcd_panel_handle_t*)>;

struct NI2CMasterLcdConfig
{
    esp_lcd_panel_io_i2c_config_t io_config;
    esp_lcd_panel_dev_config_t panel_config;
    NI2CMasterLcdNewPanelFunc new_panel_func_;

    /*
     * control_phase_bytes: 1
     * dc_bit_offset: 0
     * lcd_cmd_bits: 8
     * lcd_param_bits: 8
     * dc_low_on_data: false
     * disable_control_phase: false
     * reset_gpio: GPIO_NUM_NC
     * rgb_order: LCD_RGB_ELEMENT_ORDER_RGB
     * data_endian: LCD_RGB_DATA_ENDIAN_BIG
     * bits_per_pixel: 16
     * reset_active_high: false
     * vendor_conf: nullptr
     * new_panel_func: esp_lcd_new_panel_ssd1306
     */
    NI2CMasterLcdConfig(uint16_t dev_addr,
                        unsigned int control_phase_bytes,
                        unsigned int dc_bit_offset,
                        unsigned int lcd_cmd_bits,
                        unsigned int lcd_param_bits,
                        bool dc_low_on_data,
                        bool disable_control_phase,
                        gpio_num_t reset_gpio,
                        lcd_rgb_element_order_t rgb_order,
                        lcd_rgb_data_endian_t data_endian,
                        uint32_t bits_per_pixel,
                        bool reset_active_high,
                        void* vendor_conf, 
                        NI2CMasterLcdNewPanelFunc new_panel_func)
    {
        // Init io_config
        io_config.dev_addr = dev_addr;
        io_config.on_color_trans_done = NULL;
        io_config.user_ctx = NULL;
        io_config.control_phase_bytes = control_phase_bytes;
        io_config.dc_bit_offset = dc_bit_offset;
        io_config.lcd_cmd_bits = lcd_cmd_bits;
        io_config.lcd_param_bits = lcd_param_bits;
        io_config.flags.dc_low_on_data = dc_low_on_data;
        io_config.flags.disable_control_phase = disable_control_phase;
        io_config.scl_speed_hz = 0; // 0 means use bus speed

        // Init panel_config
        panel_config.reset_gpio_num = reset_gpio;
        panel_config.rgb_ele_order = rgb_order;
        panel_config.data_endian = data_endian;
        panel_config.bits_per_pixel = bits_per_pixel;
        panel_config.flags.reset_active_high = reset_active_high;
        panel_config.vendor_config = vendor_conf;
        
        new_panel_func_ = new_panel_func;
    }
};

class NI2CMasterLcd : public NI2CMasterDevice
{
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

public:
    NI2CMasterLcd(NLogger& logger) : NI2CMasterDevice(logger), m_io_handle(NULL), m_panel_handle(NULL) {}
    ~NI2CMasterLcd();

    esp_err_t Init(const NI2CMasterBus& bus, const NI2CMasterLcdConfig& config);
    esp_err_t Deinit();
    esp_err_t Reset();
    esp_err_t TurnOn();
    esp_err_t TurnOff();
    esp_err_t SetDispOnOff(bool on_off);
    
    esp_err_t Mirror(bool mirror_x, bool mirror_y);
    esp_err_t SwapXY(bool swap_axes);
    esp_err_t SetGap(int x_gap, int y_gap);
    esp_err_t InvertColor(bool invert);
    esp_err_t Sleep(bool sleep);

    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t>& bitmap);
    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t>& bitmap);
    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<bool>& bitmap);
};
