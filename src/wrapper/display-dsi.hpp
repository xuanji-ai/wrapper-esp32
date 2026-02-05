#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"

#include "wrapper/logger.hpp"

namespace wrapper
{

  using LcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t *, esp_lcd_panel_handle_t *)>;
  using CustomLcdPanelInitFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t)>;

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
   * @brief DSI (MIPI-DSI) LCD Display wrapper class
   * 
   * Specialized display class for MIPI-DSI based LCD panels
   */
  class DsiDisplay
  {
  private:
    Logger &m_logger;
    esp_lcd_dsi_bus_handle_t m_dsi_bus_handle;
    esp_lcd_panel_io_handle_t m_io_handle;
    esp_lcd_panel_handle_t m_panel_handle;

    esp_err_t InitDsiBus(const esp_lcd_dsi_bus_config_t &config);
    esp_err_t InitDbiIo(const esp_lcd_dbi_io_config_t &config);
    esp_err_t InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func = nullptr);

  public:
    DsiDisplay(Logger &logger);
    ~DsiDisplay();

    esp_err_t Init(
      const DsiLcdConfig &config, 
      LcdNewPanelFunc new_panel_func, 
      CustomLcdPanelInitFunc custom_init_panel_func,
      std::function<void(void* vendor_conf)> vendor_conf_init_func);
    esp_err_t Deinit();

    // Panel IO operations
    esp_err_t IoTxParam(int lcd_cmd, const void *param, size_t param_size);
    esp_err_t IoTxColor(int lcd_cmd, const void *color, size_t color_size);

    // Panel operations
    esp_err_t Reset();
    esp_err_t PanelInit();
    esp_err_t DrawBitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);
    esp_err_t Mirror(bool mirror_x, bool mirror_y);
    esp_err_t SwapXY(bool swap_axes);
    esp_err_t SetGap(int x_gap, int y_gap);
    esp_err_t InvertColor(bool invert_color_data);
    esp_err_t DispOnOff(bool on_off);
    esp_err_t DispSleep(bool sleep);

    // Handle getters
    esp_lcd_dsi_bus_handle_t GetDsiBusHandle() const { return m_dsi_bus_handle; }
    esp_lcd_panel_io_handle_t GetIoHandle() const { return m_io_handle; }
    esp_lcd_panel_handle_t GetPanelHandle() const { return m_panel_handle; }
  };

} // namespace wrapper
