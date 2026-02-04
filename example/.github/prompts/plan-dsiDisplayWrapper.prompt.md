## Plan: 为 Display Wrapper 添加 DSI 支持

在 wrapper 层新增 `DsiBus` 类封装 MIPI DSI 总线，新增 `DpiPanelConfig` 封装 DPI 面板配置（单一构造函数显式初始化所有参数），并扩展 `Display` 类支持 DSI 初始化。使用 `void* vendor_config` 泛型指针，LDO 由板级代码管理并传入 handle。

### Steps

1. **新建 [dsi.hpp](wrapper-esp32/src/wrapper/dsi.hpp)**：
   - `DsiBusConfig`（继承 `esp_lcd_dsi_bus_config_t`）：构造函数 `(bus_id, num_data_lanes, phy_clk_src, lane_bit_rate_mbps)`
   - `DsiDbiIoConfig`（继承 `esp_lcd_dbi_io_config_t`）：构造函数 `(virtual_channel, lcd_cmd_bits, lcd_param_bits)`
   - `DpiPanelConfig`（继承 `esp_lcd_dpi_panel_config_t`）：单一构造函数 `(virtual_channel, dpi_clk_src, dpi_clock_freq_mhz, in_color_format, num_fbs, h_size, v_size, hsync_pulse_width, hsync_back_porch, hsync_front_porch, vsync_pulse_width, vsync_back_porch, vsync_front_porch, use_dma2d)`
   - `DsiBus` 类：`Init(const DsiBusConfig&, esp_ldo_channel_handle_t)`、`Deinit()`、`GetHandle()`、`CreateDbiIo()`

2. **新建 [dsi.cpp](wrapper-esp32/src/wrapper/dsi.cpp)**：实现 `DsiBus::Init`（接受外部 LDO handle，调用 `esp_lcd_new_dsi_bus`）、`Deinit`（仅释放 bus，不释放 LDO）、`CreateDbiIo`（调用 `esp_lcd_new_panel_io_dbi`）

3. **修改 [display.hpp](wrapper-esp32/src/wrapper/display.hpp)**：添加 `#include "wrapper/dsi.hpp"`，添加 `DsiLcdConfig` 结构体（含 `DsiDbiIoConfig dbi_io_config`、`DpiPanelConfig dpi_panel_config`、`esp_lcd_panel_dev_config_t panel_config`、`void* vendor_config`），声明 `Init(const DsiBus&, DsiLcdConfig&, LcdNewPanelFunc, CustomLcdPanelInitFunc)` 重载

4. **修改 [display.cpp](wrapper-esp32/src/wrapper/display.cpp)**：实现 DSI Init，调用 `bus.CreateDbiIo()` 获取 io_handle，设置 `config.panel_config.vendor_config = config.vendor_config`，调用 `LcdNewPanelFunc` 创建面板并存储 handle，不调用 `esp_lcd_panel_reset/init`

5. **更新 [tab5.cpp](wrapper-esp32/src/board/m5stack/tab5.cpp)**：板级代码调用 `esp_ldo_acquire_channel` 获取 LDO handle，传入 `DsiBus::Init`；使用新配置结构体构造参数；调用 `Display::Init(dsi_bus, dsi_lcd_config, esp_lcd_new_panel_ili9881c)`；移除多余的 `esp_lcd_panel_reset/init` 调用
