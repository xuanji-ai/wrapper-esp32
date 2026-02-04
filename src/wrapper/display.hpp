#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"

#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"

namespace wrapper
{
  using LcdPanelNewFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)>;
  using LcdPanelCustomInitFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t)>;

  struct I2cLcdConfig
  {
    esp_lcd_panel_io_i2c_config_t io_config;
    esp_lcd_panel_dev_config_t panel_config;

    I2cLcdConfig(
        // io_config parameters
        uint16_t dev_addr = 0x00,
        esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done = nullptr,
        void *user_ctx = nullptr,
        unsigned int control_phase_bytes = 1,
        unsigned int dc_bit_offset = 0,
        unsigned int lcd_cmd_bits = 8,
        unsigned int lcd_param_bits = 8,
        bool dc_low_on_data = false,
        bool disable_control_phase = true,
        uint32_t scl_speed_hz = 100000,
        // panel_config parameters
        gpio_num_t reset_gpio = GPIO_NUM_NC,
        lcd_rgb_element_order_t rgb_order = LCD_RGB_ELEMENT_ORDER_RGB,
        lcd_rgb_data_endian_t data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        uint32_t bits_per_pixel = 1,
        bool reset_active_high = false,
        void *vendor_conf = nullptr
      ) : io_config{}, panel_config{}
    {
      // Init io_config
      io_config.dev_addr = dev_addr;
      io_config.on_color_trans_done = on_color_trans_done;
      io_config.user_ctx = user_ctx;
      io_config.control_phase_bytes = control_phase_bytes;
      io_config.dc_bit_offset = dc_bit_offset;
      io_config.lcd_cmd_bits = lcd_cmd_bits;
      io_config.lcd_param_bits = lcd_param_bits;
      io_config.flags.dc_low_on_data = dc_low_on_data;
      io_config.flags.disable_control_phase = disable_control_phase;
      io_config.scl_speed_hz = scl_speed_hz;

      // Init panel_config
      panel_config.reset_gpio_num = reset_gpio;
      panel_config.rgb_ele_order = rgb_order;
      panel_config.data_endian = data_endian;
      panel_config.bits_per_pixel = bits_per_pixel;
      panel_config.flags.reset_active_high = reset_active_high;
      panel_config.vendor_config = vendor_conf;
    }
  };

  struct SpiLcdConfig
  {
    esp_lcd_panel_io_spi_config_t io_config;
    esp_lcd_panel_dev_config_t panel_config;

    SpiLcdConfig(
        gpio_num_t cs_gpio,
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
        void *vendor_conf) : io_config{}, panel_config{}
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
    }
  };

  class Display
  {
    Logger &m_logger;
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

    esp_err_t InitIo(const I2cBus &bus, const esp_lcd_panel_io_i2c_config_t &config);
    esp_err_t InitIo(const SpiBus &bus, const esp_lcd_panel_io_spi_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, LcdPanelCustomInitFunc custom_init_func = nullptr);

  public:
    Display(Logger &logger);
    ~Display();

    esp_err_t Init(const I2cBus &bus, const I2cLcdConfig &config, LcdPanelNewFunc new_panel_func, LcdPanelCustomInitFunc custom_init_func = nullptr);
    esp_err_t Init(const SpiBus &bus, const SpiLcdConfig &config, LcdPanelNewFunc new_panel_func, LcdPanelCustomInitFunc custom_init_func = nullptr);

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

    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<uint16_t> &bitmap);
    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<uint8_t> &bitmap);
    esp_err_t DrawBitmap(int x, int y, int w, int h, const std::vector<bool> &bitmap);

    esp_lcd_panel_io_handle_t GetIoHandle() const { return m_io_handle; }
    esp_lcd_panel_handle_t GetPanelHandle() const { return m_panel_handle; }

    bool TestColors(bool monochrome = false);
  };

} // namespace wrapper