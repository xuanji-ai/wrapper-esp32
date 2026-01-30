# 实现 LvglPort 类开发计划

根据 `M5Stack CoreS3` 的初始化流程以及现有 `I2cLcd` 和 `I2cTouch` 的结构，我将实现 `nlvgl.hpp` 和 `nlvgl.cpp`。

## 技术方案

### 1. 结构定义 (`nlvgl.hpp`)
- **命名空间**: 使用 `nix` 命名空间。
- **配置结构体**:
    - `LvglPortConfig`: 映射 `lvgl_port_cfg_t`，包含任务优先级、栈大小、内核绑定等。
    - `LvglDisplayConfig`: 映射 `lvgl_port_display_cfg_t`，包含缓冲区大小、分辨率、颜色格式等。
    - `LvglTouchConfig`: 映射 `lvgl_port_touch_cfg_t`。
- **类 `LvglPort`**:
    - 成员变量: `Logger& m_logger`, `lv_display_t* m_lvgl_display`, `lv_indev_t* m_lvgl_touch`, `bool m_initialized`。
    - 方法:
        - `Init(const LvglPortConfig& config)`: 调用 `lvgl_port_init`。
        - `Deinit()`: 调用 `lvgl_port_deinit` 及清理显示/触摸设备。
        - `AddDisplay(const I2cLcd& lcd, const LvglDisplayConfig& config)`: 将 `I2cLcd` 关联到 LVGL。
        - `AddDisplay(const SpiLcd& lcd, const LvglDisplayConfig& config)`: 支持 SPI LCD。
        - `AddTouch(const I2cTouch& touch, const LvglTouchConfig& config)`: 将 `I2cTouch` 关联到 LVGL。
        - `Lock(uint32_t timeout_ms)` / `Unlock()`: 线程安全锁封装。

### 2. 依赖类补全
- **`I2cLcd` / `SpiLcd`**: 需要添加 `GetPanelHandle()` 和 `GetIoHandle()` 方法，以便 LVGL 获取底层句柄。
- **`I2cTouch`**: 已有 `GetHandle()`。

### 3. 具体实现 (`nlvgl.cpp`)
- 按照 `m5stack_core_s3.c` L712-809 的逻辑实现初始化和去初始化。
- `Init` 时调用 `lvgl_port_init`。
- `AddDisplay` 时填充 `lvgl_port_display_cfg_t` 并调用 `lvgl_port_add_disp`。
- `AddTouch` 时填充 `lvgl_port_touch_cfg_t` 并调用 `lvgl_port_add_touch`。

## 实施步骤

1. **修改 `ndisplay.hpp`**: 为 `I2cLcd` 和 `SpiLcd` 添加获取句柄的方法。
2. **编写 `nlvgl.hpp`**: 定义配置结构体和 `LvglPort` 类。
3. **编写 `nlvgl.cpp`**: 实现相关方法。
4. **验证**: 确保逻辑与 `M5Stack CoreS3` 的 LVGL 初始化序列完全一致。

请确认此方案。