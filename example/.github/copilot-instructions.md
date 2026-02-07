# Copilot instructions (wrapper-esp32)

## 项目概览
- 这是一个 ESP-IDF 组件工程：示例应用在 example/main，组件代码在 example/wrapper-esp32。
- 目标是用 C++ 封装 ESP-IDF/ESP-Registry 组件：核心封装在 wrapper-esp32/src/wrapper，设备驱动在 wrapper-esp32/src/device，板级聚合在 wrapper-esp32/src/board。
- 典型板级初始化流程：SoC 能力 -> 总线 -> 设备 -> 中间件（LVGL），示例见 wrapper-esp32/src/board/m5stack/m5stack_core_s3.cpp。

## 关键约定与模式
- **配置结构体封装**：诸如 I2cBusConfig、SpiDisplayConfig 等都直接继承 ESP-IDF 的 *_config_t，并在构造函数中一次性填充常量参数（见 wrapper-esp32/src/wrapper/i2c.hpp、display.hpp）。新增封装时沿用该模式。
- **Init/Deinit 语义**：所有 wrapper 类都提供 Init/Deinit，返回 esp_err_t 或 bool；出错时用 Logger 记录 `esp_err_to_name(err)`（例如 wrapper-esp32/src/wrapper/lvgl.cpp）。
- **Logger 标签**：Logger 支持多级 tag（例如 "M5StackCoreS3", "i2c", "bus"），保持现有层级命名以便日志过滤（见 wrapper-esp32/src/board/m5stack/m5stack_core_s3.cpp）。
- **LVGL 访问**：使用 LvglPort.Init -> AddDisplay -> AddTouch，操作 UI 前后必须 Lock/Unlock（见 wrapper-esp32/src/wrapper/lvgl.cpp）。
- **I2C 设备**：驱动类通常继承 I2cDevice，并复用 Read/WriteReg* 辅助方法（见 wrapper-esp32/src/device/axp2101.hpp）。

## 构建与运行
- 这是标准 ESP-IDF CMake 工程（example/CMakeLists.txt）；常用流程：`idf.py build` / `idf.py flash` / `idf.py monitor`。
- 工程包含多份 sdkconfig（如 sdkconfig-m5cs3、sdkconfig-m5t5），切换目标板时优先参考这些配置文件。

## 依赖与集成点
- 组件依赖在 wrapper-esp32/idf_component.yml 中统一声明（如 esp_lvgl_port、esp_lcd_ili9341、esp32-camera、m5stack_core_s3）。添加新依赖请更新此文件。
- 设备/板级实现中会调用 ESP-IDF 或 ESP-Registry 的 new_xxx 工厂函数（例如 esp_lcd_new_panel_ili9341、esp_lcd_touch_new_i2c_ft5x06）。
