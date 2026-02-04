#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"

#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"

namespace wrapper
{

  using LcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)>;
  using LcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)>;
  using CustomLcdPanelInitFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t)>;

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
        // io_config parameters
        int cs_gpio,
        int dc_gpio,
        int spi_mode,
        int clock_speed_hz,
        int lcd_cmd_bits,
        int lcd_param_bits,
        size_t trans_queue_depth = 10,
        esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done = nullptr,
        void *user_ctx = nullptr,
        int cs_ena_pretrans = 0,
        int cs_ena_posttrans = 0,
        // io_config flags
        unsigned int dc_high_on_cmd = 0,
        unsigned int dc_low_on_data = 0,
        unsigned int dc_low_on_param = 0,
        unsigned int octal_mode = 0,
        unsigned int quad_mode = 0,
        unsigned int sio_mode = 0,
        unsigned int lsb_first = 0,
        unsigned int cs_high_active = 0,
        // panel_config parameters
        gpio_num_t reset_gpio = GPIO_NUM_NC,
        lcd_rgb_element_order_t rgb_order = LCD_RGB_ELEMENT_ORDER_RGB,
        lcd_rgb_data_endian_t data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        uint32_t bits_per_pixel = 16,
        bool reset_active_high = false,
        void *vendor_conf = nullptr) : io_config{}, panel_config{}
    {
      // Init io_config
      io_config.cs_gpio_num = cs_gpio;
      io_config.dc_gpio_num = dc_gpio;
      io_config.spi_mode = spi_mode;
      io_config.pclk_hz = clock_speed_hz;
      io_config.trans_queue_depth = trans_queue_depth;
      io_config.on_color_trans_done = on_color_trans_done;
      io_config.user_ctx = user_ctx;
      io_config.lcd_cmd_bits = lcd_cmd_bits;
      io_config.lcd_param_bits = lcd_param_bits;
      io_config.cs_ena_pretrans = cs_ena_pretrans;
      io_config.cs_ena_posttrans = cs_ena_posttrans;

      // Init flags
      io_config.flags.dc_high_on_cmd = dc_high_on_cmd;
      io_config.flags.dc_low_on_data = dc_low_on_data;
      io_config.flags.dc_low_on_param = dc_low_on_param;
      io_config.flags.octal_mode = octal_mode;
      io_config.flags.quad_mode = quad_mode;
      io_config.flags.sio_mode = sio_mode;
      io_config.flags.lsb_first = lsb_first;
      io_config.flags.cs_high_active = cs_high_active;

      // Init panel_config
      panel_config.reset_gpio_num = reset_gpio;
      panel_config.rgb_ele_order = rgb_order;
      panel_config.data_endian = data_endian;
      panel_config.bits_per_pixel = bits_per_pixel;
      panel_config.flags.reset_active_high = reset_active_high;
      panel_config.vendor_config = vendor_conf;
    }
  };

  struct DsiLcdConfig
  {
    esp_lcd_dsi_bus_config_t dsi_bus_config;
    esp_lcd_dbi_io_config_t dbi_io_config;
    esp_lcd_dpi_panel_config_t dpi_config;
    esp_lcd_panel_dev_config_t panel_config;

    DsiLcdConfig(
        // dsi_bus_config parameters
        uint8_t bus_id,
        uint8_t num_data_lanes,
        mipi_dsi_phy_clock_source_t phy_clk_src,
        uint32_t lane_bit_rate_mbps,
        // dbi_io_config parameters
        uint8_t virtual_channel_dbi,
        int lcd_cmd_bits,
        int lcd_param_bits,
        // dpi_config parameters
        uint8_t virtual_channel_dpi,
        mipi_dsi_dpi_clock_source_t dpi_clk_src,
        uint32_t dpi_clock_freq_mhz,
        lcd_color_rgb_pixel_format_t in_color_format,
        uint32_t h_size,
        uint32_t v_size,
        uint32_t hsync_back_porch,
        uint32_t hsync_pulse_width,
        uint32_t hsync_front_porch,
        uint32_t vsync_back_porch,
        uint32_t vsync_pulse_width,
        uint32_t vsync_front_porch,
        size_t num_fbs,
        bool use_dma2d,
        // panel_config parameters
        gpio_num_t reset_gpio,
        lcd_rgb_element_order_t rgb_order,
        lcd_rgb_data_endian_t data_endian,
        uint32_t bits_per_pixel,
        bool reset_active_high,
        void *vendor_conf
      ) : dsi_bus_config{}, dbi_io_config{}, dpi_config{}, panel_config{}
    {
      // Init dsi_bus_config
      dsi_bus_config.bus_id = bus_id;
      dsi_bus_config.num_data_lanes = num_data_lanes;
      dsi_bus_config.phy_clk_src = phy_clk_src;
      dsi_bus_config.lane_bit_rate_mbps = lane_bit_rate_mbps;

      // Init dbi_io_config
      dbi_io_config.virtual_channel = virtual_channel_dbi;
      dbi_io_config.lcd_cmd_bits = lcd_cmd_bits;
      dbi_io_config.lcd_param_bits = lcd_param_bits;

      // Init dpi_config
      dpi_config.virtual_channel = virtual_channel_dpi;
      dpi_config.dpi_clk_src = dpi_clk_src;
      dpi_config.dpi_clock_freq_mhz = dpi_clock_freq_mhz;
      dpi_config.pixel_format = in_color_format;
      dpi_config.video_timing.h_size = h_size;
      dpi_config.video_timing.v_size = v_size;
      dpi_config.video_timing.hsync_back_porch = hsync_back_porch;
      dpi_config.video_timing.hsync_pulse_width = hsync_pulse_width;
      dpi_config.video_timing.hsync_front_porch = hsync_front_porch;
      dpi_config.video_timing.vsync_back_porch = vsync_back_porch;
      dpi_config.video_timing.vsync_pulse_width = vsync_pulse_width;
      dpi_config.video_timing.vsync_front_porch = vsync_front_porch;
      dpi_config.num_fbs = num_fbs;
      dpi_config.flags.use_dma2d = use_dma2d;

      // Init panel_config
      panel_config.reset_gpio_num = reset_gpio;
      panel_config.rgb_ele_order = rgb_order;
      panel_config.data_endian = data_endian;
      panel_config.bits_per_pixel = bits_per_pixel;
      panel_config.flags.reset_active_high = reset_active_high;
      panel_config.vendor_config = vendor_conf;
    }
  };

  /**
   * @brief LCD Display Base wrapper class
   * 
   * Combines panel IO and panel operations for complete display control
   */
  class DisplayBase
  {
  protected:
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

  public:
    DisplayBase(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle) 
      : m_io_handle(io_handle), m_panel_handle(panel_handle) {}
    
    virtual ~DisplayBase() = default;

    // Panel IO operations
    esp_err_t IoTxParam(int lcd_cmd, const void *param, size_t param_size) {
      return esp_lcd_panel_io_tx_param(m_io_handle, lcd_cmd, param, param_size);
    }
    
    esp_err_t IoTxColor(int lcd_cmd, const void *color, size_t color_size) {
      return esp_lcd_panel_io_tx_color(m_io_handle, lcd_cmd, color, color_size);
    }

    // Panel operations
    esp_err_t Reset() { return esp_lcd_panel_reset(m_panel_handle); }
    esp_err_t Init() { return esp_lcd_panel_init(m_panel_handle); }
    
    esp_err_t DrawBitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data) {
      return esp_lcd_panel_draw_bitmap(m_panel_handle, x_start, y_start, x_end, y_end, color_data);
    }
    
    esp_err_t Mirror(bool mirror_x, bool mirror_y) {
      return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
    }
    
    esp_err_t SwapXY(bool swap_axes) {
      return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
    }
    
    esp_err_t SetGap(int x_gap, int y_gap) {
      return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
    }
    
    esp_err_t InvertColor(bool invert_color_data) {
      return esp_lcd_panel_invert_color(m_panel_handle, invert_color_data);
    }
    
    esp_err_t DispOnOff(bool on_off) {
      return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
    }
    
    esp_err_t DispSleep(bool sleep) {
      return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
    }

    // Resource cleanup
    esp_err_t DelPanel() { return esp_lcd_panel_del(m_panel_handle); }
    esp_err_t DelIo() { return esp_lcd_panel_io_del(m_io_handle); }

    // Handle getters
    esp_lcd_panel_io_handle_t GetIoHandle() const { return m_io_handle; }
    esp_lcd_panel_handle_t GetPanelHandle() const { return m_panel_handle; }
  };

  /**
   * @brief I2C LCD Display wrapper class
   * 
   * Specialized display class for I2C-based LCD panels
   */
  class I2cDisplay : public DisplayBase
  {
  private:
    Logger &m_logger;
    const I2cBus &m_bus;

    esp_err_t InitIo(const esp_lcd_panel_io_i2c_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

  public:
    I2cDisplay(Logger &logger, const I2cBus &bus) 
      : DisplayBase(nullptr, nullptr), m_logger(logger), m_bus(bus) {}
    
    ~I2cDisplay() { Deinit(); }

    esp_err_t Init(
      const I2cLcdConfig &config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func = nullptr);
    esp_err_t Deinit();
  };

  class SpiDisplay : public DisplayBase
  {
  private:
    Logger &m_logger;
    const SpiBus &m_bus;

    esp_err_t InitIo(const esp_lcd_panel_io_spi_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

  public:
    SpiDisplay(Logger &logger, const SpiBus &bus) 
      : DisplayBase(nullptr, nullptr), m_logger(logger), m_bus(bus) {}
    
    ~SpiDisplay() { Deinit(); }

    esp_err_t Init(const SpiLcdConfig &config,
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func = nullptr);
    esp_err_t Deinit();
  };

  /**
   * @brief DSI (MIPI-DSI) LCD Display wrapper class
   * 
   * Specialized display class for MIPI-DSI based LCD panels
   */
  class DsiDisplay : public DisplayBase
  {
  private:
    Logger &m_logger;
    esp_lcd_dsi_bus_handle_t m_dsi_bus_handle;

    esp_err_t InitDsiBus(const esp_lcd_dsi_bus_config_t &config);
    esp_err_t InitDbiIo(const esp_lcd_dbi_io_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

  public:
    DsiDisplay(Logger &logger) 
      : DisplayBase(nullptr, nullptr), m_logger(logger), m_dsi_bus_handle(nullptr) {}
    
    ~DsiDisplay() { Deinit(); }

    esp_err_t Init(
      const DsiLcdConfig &config, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)> new_panel_func, 
      std::function<esp_err_t(const esp_lcd_panel_io_handle_t)> custom_init_panel_func,
      std::function<void(void* vender_conf)> vender_conf_init_func);
    esp_err_t Deinit();

    esp_lcd_dsi_bus_handle_t GetDsiBusHandle() const { return m_dsi_bus_handle; }
  };

  class Display
  {
    Logger &m_logger;
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

    esp_err_t InitIo(const I2cBus &bus, const esp_lcd_panel_io_i2c_config_t &config);
    esp_err_t InitIo(const SpiBus &bus, const esp_lcd_panel_io_spi_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

  public:
    Display(Logger &logger);
    ~Display();

    esp_err_t Init(const I2cBus &bus, const I2cLcdConfig &config, LcdNewPanelFunc new_panel_func, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);
    esp_err_t Init(const SpiBus &bus, const SpiLcdConfig &config, LcdNewPanelFunc new_panel_func, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

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