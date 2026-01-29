#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ssd1306.h"
#include "nlogger.hpp"
#include <vector>
#include <functional>

struct NSpiMasterBusConfig : public spi_bus_config_t
{
    spi_host_device_t host_id;
    spi_dma_chan_t dma_chan;

    /*
     * host: SPI2_HOST
     * mosi: GPIO_NUM_NC
     * miso: GPIO_NUM_NC
     * sclk: GPIO_NUM_NC
     * max_transfer: 4092
     * dma: SPI_DMA_CH_AUTO
     * bus_flags: SPICOMMON_BUSFLAG_MASTER
     */
    NSpiMasterBusConfig(spi_host_device_t host,
                        gpio_num_t mosi,
                        gpio_num_t miso,
                        gpio_num_t sclk,
                        int max_transfer,
                        spi_dma_chan_t dma,
                        uint32_t bus_flags)
    {
        mosi_io_num = mosi;
        miso_io_num = miso;
        sclk_io_num = sclk;
        quadwp_io_num = GPIO_NUM_NC;
        quadhd_io_num = GPIO_NUM_NC;
        data4_io_num = GPIO_NUM_NC;
        data5_io_num = GPIO_NUM_NC;
        data6_io_num = GPIO_NUM_NC;
        data7_io_num = GPIO_NUM_NC;
        max_transfer_sz = max_transfer;
        flags = bus_flags;
        intr_flags = 0;
        data_io_default_level = false;
        isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;

        host_id = host;
        dma_chan = dma;
    }
};

class NSpiMasterBus
{
    NLogger& m_logger;
    spi_host_device_t m_host_id;
    bool m_initialized;
    NSpiMasterBusConfig m_config;

public:
    NSpiMasterBus(NLogger& logger);
    ~NSpiMasterBus();
    NLogger& GetLogger();
    spi_host_device_t GetHostId() const;
    
    esp_err_t Init(const NSpiMasterBusConfig& config);
    esp_err_t Deinit();
    esp_err_t Reset();
};

struct NSpiMasterDeviceConfig : public spi_device_interface_config_t
{
    /*
     * cs: GPIO_NUM_NC
     * clock_speed_hz: 10 * 1000 * 1000
     * mode: 0
     */
    NSpiMasterDeviceConfig(gpio_num_t cs, int clock_speed_hz, uint8_t mode)
    {
        command_bits = 0;
        address_bits = 0;
        dummy_bits = 0;
        this->mode = mode;
        duty_cycle_pos = 128;
        cs_ena_pretrans = 0;
        cs_ena_posttrans = 0;
        this->clock_speed_hz = clock_speed_hz;
        input_delay_ns = 0;
        spics_io_num = cs;
        flags = 0;
        queue_size = 3;
        pre_cb = NULL;
        post_cb = NULL;
    }
};

class NSpiMasterDevice
{
protected:
    NLogger& m_logger;
    spi_device_handle_t m_dev_handle;

public:
    NSpiMasterDevice(NLogger& logger);
    ~NSpiMasterDevice();
    NLogger& GetLogger();
    
    esp_err_t Init(const NSpiMasterBus& bus, const NSpiMasterDeviceConfig& config);
    esp_err_t Deinit();
    
    // Simple transfer (write and read simultaneously)
    esp_err_t Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data);
    
    // Write only
    esp_err_t Write(const std::vector<uint8_t>& data);
    
    // Read only (sends dummy data)
    esp_err_t Read(size_t len, std::vector<uint8_t>& rx_data);
};

// Define the function pointer type for creating a new panel
using NSpiMasterLcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t*, esp_lcd_panel_handle_t*)>;

struct NSpiMasterLcdConfig
{
    esp_lcd_panel_io_spi_config_t io_config;
    esp_lcd_panel_dev_config_t panel_config;
    NSpiMasterLcdNewPanelFunc new_panel_func_;

    /*
     * cs_gpio: GPIO_NUM_NC
     * dc_gpio: GPIO_NUM_NC
     * clock_speed_hz: 10 * 1000 * 1000
     * spi_mode: 0
     * lcd_cmd_bits: 8
     * lcd_param_bits: 8
     * reset_gpio: GPIO_NUM_NC
     * rgb_order: LCD_RGB_ELEMENT_ORDER_RGB
     * data_endian: LCD_RGB_DATA_ENDIAN_BIG
     * bits_per_pixel: 16
     * reset_active_high: false
     * vendor_conf: nullptr
     * new_panel_func: esp_lcd_new_panel_ssd1306
     */
    NSpiMasterLcdConfig(gpio_num_t cs_gpio,
                        gpio_num_t dc_gpio,
                        int clock_speed_hz,
                        int spi_mode,
                        int lcd_cmd_bits,
                        int lcd_param_bits,
                        gpio_num_t reset_gpio,
                        lcd_rgb_element_order_t rgb_order,
                        lcd_rgb_data_endian_t data_endian,
                        uint32_t bits_per_pixel,
                        bool reset_active_high,
                        void* vendor_conf,
                        NSpiMasterLcdNewPanelFunc new_panel_func)
    {
        // Init io_config
        io_config.cs_gpio_num = cs_gpio;
        io_config.dc_gpio_num = dc_gpio;
        io_config.spi_mode = spi_mode;
        io_config.pclk_hz = clock_speed_hz;
        io_config.trans_queue_depth = 10;
        io_config.on_color_trans_done = NULL;
        io_config.user_ctx = NULL;
        io_config.lcd_cmd_bits = lcd_cmd_bits;
        io_config.lcd_param_bits = lcd_param_bits;
        io_config.cs_ena_pretrans = 0;
        io_config.cs_ena_posttrans = 0;
        
        // Clear flags
        io_config.flags.dc_high_on_cmd = 0;
        io_config.flags.dc_low_on_data = 0;
        io_config.flags.dc_low_on_param = 0;
        io_config.flags.octal_mode = 0;
        io_config.flags.quad_mode = 0;
        io_config.flags.sio_mode = 0;
        io_config.flags.lsb_first = 0;
        io_config.flags.cs_high_active = 0;

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

class NSpiMasterLcd : public NSpiMasterDevice
{
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

public:
    NSpiMasterLcd(NLogger& logger) : NSpiMasterDevice(logger), m_io_handle(NULL), m_panel_handle(NULL) {}
    ~NSpiMasterLcd();

    esp_err_t Init(const NSpiMasterBus& bus, const NSpiMasterLcdConfig& config);
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
