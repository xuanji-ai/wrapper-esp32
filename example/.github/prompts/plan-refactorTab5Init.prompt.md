# Plan: 重构 M5StackTab5 为统一 Init() 方法

**TL;DR**: 移除 `InitBus`/`InitDevice`/`InitMiddleware`，整合到 `Init()`，按官方 BSP 顺序初始化：I2C → IO Expander → 电源 → 背光 → **DSI PHY LDO** → DSI 总线 → Panel → Touch → I2S/Audio → LVGL。`Display::Init()` 已内部调用 reset/init，无需重复。

---

## Steps

### 1. 更新 tab5.hpp
- 删除 `InitBus`、`InitDevice`、`InitMiddleware` 三个方法声明 (第 35-37 行)
- 仅保留 `bool Init();`

### 2. 更新 tab5.cpp 头文件区域
- 添加 `#include "esp_ldo_regulator.h"`
- 添加 LDO 常量定义:
  ```cpp
  #define BSP_MIPI_DSI_PHY_PWR_LDO_CHAN       (3)
  #define BSP_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)
  ```
- 在 namespace 内添加静态变量:
  ```cpp
  static esp_ldo_channel_handle_t dsi_phy_pwr_ldo = NULL;
  ```

### 3. 修改 lvgl_display_config 缓冲区
- 将 `BSP_LCD_H_RES * BSP_LCD_V_RES` (921600) 改为 `BSP_LCD_H_RES * 100` (72000)

### 4. 添加错误处理宏
在文件顶部 (TAG 定义后) 添加:
```cpp
#define INIT_CHECK(expr, msg) do { \
    esp_err_t _err = (expr); \
    if (_err != ESP_OK) { \
        ESP_LOGE(TAG, "%s failed: %s", msg, esp_err_to_name(_err)); \
        return false; \
    } \
} while(0)
```

### 5. 重写 Init() 方法
按以下顺序执行（每步失败记录日志并返回 false）:

| 步骤 | 操作 | 说明 |
|------|------|------|
| 1 | `i2c_bus0.Init(i2c_bus0_config)` | I2C 总线 |
| 2 | `esp_io_expander_new_i2c_pi4ioe5v6408()` x2 | IO Expander (0x43, 0x44) |
| 3 | `esp_io_expander_set_dir/level/output_mode()` | 启用 LCD/Touch/Speaker 电源 |
| 4 | `SetBacklight(0)` | 初始化背光 (关闭) |
| 5 | `esp_ldo_acquire_channel()` | **DSI PHY LDO 通道 3 @ 2500mV** |
| 6 | `esp_lcd_new_dsi_bus()` | MIPI DSI 总线 |
| 7 | `esp_lcd_new_panel_io_dbi()` | DBI IO |
| 8 | `esp_lcd_new_panel_ili9881c()` | 创建 Panel |
| 9 | `ili9881c.Init()` | Display wrapper (内含 reset/init) |
| 10 | `esp_lcd_panel_disp_on_off(panel, true)` | 开启显示 |
| 11 | `SetBacklight(100)` | 开启背光 |
| 12 | GPIO_NUM_23 配置为输出低电平 | GT911 硬件修复 |
| 13 | `gt911.Init()` | 触摸 |
| 14 | `i2s_bus.Init()` + `ConfigureTx/RxChannel()` | I2S 总线 |
| 15 | `audio_codec.Init()` + `AddSpeaker/Microphone()` | 音频 |
| 16 | `lvgl_port.Init()` + `AddDisplay()` + `AddTouch()` | LVGL |

### 6. 删除旧方法实现
- 移除 `InitBus()` 的代码块 (约第 460-490 行)
- 移除 `InitDevice()` 的代码块 (约第 492-580 行)
- 移除 `InitMiddleware()` 的代码块 (约第 582-595 行)

---

## 关键修复点

### 🔴 DSI PHY LDO 电源 (最关键)
官方 BSP 在创建 DSI 总线前启用 LDO 通道 3 @ 2500mV:
```cpp
esp_ldo_channel_config_t ldo_cfg = {
    .chan_id = BSP_MIPI_DSI_PHY_PWR_LDO_CHAN,
    .voltage_mv = BSP_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
};
INIT_CHECK(esp_ldo_acquire_channel(&ldo_cfg, &dsi_phy_pwr_ldo), "Acquire DSI PHY LDO");
```

### 🔴 初始化顺序
当前代码在 `InitBus()` 中先创建 DSI 总线，但此时:
- IO Expander 尚未初始化
- LCD 电源尚未启用
- **DSI PHY LDO 从未启用**

### 🟡 开启显示
在 `ili9881c.Init()` 后添加:
```cpp
esp_lcd_panel_disp_on_off(panel_handle, true);
```

### 🟡 LVGL 缓冲区过大
当前 `BSP_LCD_H_RES * BSP_LCD_V_RES` = 921600 像素 ≈ 1.8MB
改为 `BSP_LCD_H_RES * 100` = 72000 像素 ≈ 144KB

---

## 文件变更清单

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `tab5.hpp` | 删除 | 移除 3 个方法声明 |
| `tab5.cpp` | 添加 | LDO 头文件和常量 |
| `tab5.cpp` | 修改 | lvgl_display_config 缓冲区大小 |
| `tab5.cpp` | 添加 | INIT_CHECK 宏 |
| `tab5.cpp` | 重写 | Init() 方法 |
| `tab5.cpp` | 删除 | InitBus/InitDevice/InitMiddleware |
