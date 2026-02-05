#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ops.h"

#include "wrapper/logger.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/spi.hpp"

namespace wrapper
{
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

} // namespace wrapper