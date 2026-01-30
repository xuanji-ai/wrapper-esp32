# 实现 Touch 类开发计划 (修订版)

根据您的要求，我将修改计划，**不使用派生类**。我们将实现一个通用的 `I2cTouch` 类，通过配置中的工厂函数来支持不同的触摸控制器（如 FT5x06）。

## 技术方案

### 1. 结构定义 (`ntouch.hpp`)
- **命名空间**: 使用 `nix` 命名空间。
- **配置结构体 `I2cTouchConfig`**:
    - 包含 `esp_lcd_panel_io_i2c_config_t` (I2C IO 配置)。
    - 包含 `esp_lcd_touch_config_t` (触摸控制器通用配置)。
    - 包含一个函数对象 `new_touch_func_`，用于在初始化时调用具体的驱动创建函数（如 `esp_lcd_touch_new_i2c_ft5x06`）。
    - 提供构造函数，支持从具体的参数初始化这些底层配置。
- **类 `I2cTouch`**:
    - 成员变量：`Logger& m_logger`, `esp_lcd_panel_io_handle_t m_io_handle`, `esp_lcd_touch_handle_t m_touch_handle`。
    - 公有方法：
        - `Init(const I2cBus& bus, const I2cTouchConfig& config)`: 执行初始化流程。
        - `Deinit()`: 释放资源。
        - `ReadData()`: 调用底层驱动读取触摸数据。
        - `GetCoordinates(...)`: 获取触摸坐标点。
        - `GetHandle()`: 返回底层触摸句柄。

### 2. 具体实现 (`ntouch.cpp`)
- **`Init` 流程**:
    1. 检查是否已初始化，必要时先 `Deinit`。
    2. 调用 `esp_lcd_new_panel_io_i2c` 创建 IO 句柄。
    3. 调用 `config.new_touch_func_` 创建具体的触摸控制器句柄。
    4. 记录初始化结果日志。
- **数据交互与清理**:
    - 实现 `ReadData` 和 `GetCoordinates` 封装。
    - `Deinit` 负责调用 `esp_lcd_touch_del` 和 `esp_lcd_panel_io_del`。

## 实施步骤

1. **编写 `ntouch.hpp`**: 定义 `I2cTouchConfig` 和 `I2cTouch` 类。
2. **编写 `ntouch.cpp`**: 实现相关方法。
3. **验证**: 确保代码逻辑与 `m5stack_core_s3.c` 的初始化序列一致。

请确认此修订后的方案。