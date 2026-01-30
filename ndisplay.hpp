#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"

#include "ni2c.hpp"
#include "nspi.hpp"

namespace nix
{
  // Define the function pointer type for creating a new panel
  using I2cLcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t*, esp_lcd_panel_handle_t*)>;

  struct I2cLcdConfig
  {
      esp_lcd_panel_io_i2c_config_t io_config;
      esp_lcd_panel_dev_config_t panel_config;
      I2cLcdNewPanelFunc new_panel_func_;

      I2cLcdConfig(uint16_t dev_addr,
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
                          I2cLcdNewPanelFunc new_panel_func) : io_config{}, panel_config{}
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

  class I2cLcd 
  {
      Logger& m_logger;
      esp_lcd_panel_io_handle_t m_io_handle;
      esp_lcd_panel_handle_t m_panel_handle;

  public:
      I2cLcd(Logger& logger);
      ~I2cLcd();

      esp_err_t Init(const I2cBus& bus, const I2cLcdConfig& config);
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

      esp_lcd_panel_io_handle_t GetIoHandle() const { return m_io_handle; }
      esp_lcd_panel_handle_t GetPanelHandle() const { return m_panel_handle; }
  };

  // Define the function pointer type for creating a new panel
  using SpiLcdNewPanelFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_panel_dev_config_t*, esp_lcd_panel_handle_t*)>;

  struct SpiLcdConfig
  {
      esp_lcd_panel_io_spi_config_t io_config;
      esp_lcd_panel_dev_config_t panel_config;

      SpiLcdConfig(gpio_num_t cs_gpio,
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
                          void* vendor_conf
                          ) : io_config{}, panel_config{}
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

  class SpiLcd
  {
      Logger& m_logger;
      esp_lcd_panel_io_handle_t m_io_handle;
      esp_lcd_panel_handle_t m_panel_handle;

  public:
      SpiLcd(Logger& logger) : m_logger(logger), m_io_handle(NULL), m_panel_handle(NULL) {}
      ~SpiLcd();

      esp_err_t Init(const SpiBus& bus, const SpiLcdConfig& config, SpiLcdNewPanelFunc new_panel_func);
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

      esp_lcd_panel_io_handle_t GetIoHandle() const { return m_io_handle; }
      esp_lcd_panel_handle_t GetPanelHandle() const { return m_panel_handle; }
  };

  class SpiLcdIli9341 : public SpiLcd
  {
  public:
      SpiLcdIli9341(Logger& logger);
      ~SpiLcdIli9341();
  };

} // namespace nix