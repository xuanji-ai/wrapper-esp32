# 修复 Tab5 Audio I2C NACK 和 LVGL Crash 的计划

## 问题分析
1.  **Audio I2C NACK (ES7210 @ 0x40)**:
    -   `wrapper/i2s.cpp` 中 `ConfigureTxChannel` 和 `ConfigureRxChannel` 注释掉了 `i2s_channel_enable` 的调用。
    -   这意味着在调用 `audio_codec.AddMicrophone` 时，I2S 时钟 (MCLK) 没有运行。
    -   ES7210 需要 MCLK 才能响应 I2C 命令，因此导致初始化失败。
    -   另外，官方 BSP 启用了所有 IO Expander 电源引脚 (Camera, WiFi, USB)，当前代码只启用了部分。虽然可能不是主要原因，但为了稳定性应保持一致。

2.  **LVGL Crash (`assert failed: lvgl_port_lock`)**:
    -   由于 `M5StackTab5::Init()` 中 Audio 初始化失败，宏 `INIT_CHECK` 直接返回 `false`。
    -   导致后续的 `lvgl_port.Init()` 未被执行。
    -   `app_main` 中调用 `wrapper.GetLvgl().Test()` 时，尝试获取未初始化的互斥锁，导致断言失败。

## 修复步骤

### 1. 修改 `src/board/m5stack/tab5.cpp`

#### A. 完善 IO Expander 电源控制
初始化两个 IO Expander，并启用所有相关电源引脚，确保硬件供电完全符合官方 BSP 设定：
- `BSP_LCD_EN` (IO_EXPANDER_PIN_NUM_4)
- `BSP_TOUCH_EN` (IO_EXPANDER_PIN_NUM_5)
- `BSP_SPEAKER_EN` (IO_EXPANDER_PIN_NUM_1)
- `BSP_CAMERA_EN` (IO_EXPANDER_PIN_NUM_6)
- `BSP_USB_EN` (IO_EXPANDER_PIN_NUM_3)
- `BSP_WIFI_EN` (IO_EXPANDER_PIN_NUM_0)

#### B. 修正 Audio 初始化顺序
在添加 Codec 设备之前，显式启动 I2S 通道：
```cpp
INIT_CHECK(i2s_bus.EnableTxChannel(), "Enable I2S TX");
INIT_CHECK(i2s_bus.EnableRxChannel(), "Enable I2S RX");
```

#### C. 改进错误处理
引入 `INIT_CHECK_WARN` 宏或修改逻辑，使得 Audio 初始化失败不会中断整个 `Init()` 流程，确保 LVGL 能够正常初始化，屏幕能够点亮。

```cpp
// 示例逻辑
if (audio_codec.AddSpeaker(...) != ESP_OK) {
    ESP_LOGW(TAG, "Speaker init failed, continuing...");
}
```

## 验证
编译并烧录，检查：
1.  ES7210 是否不再报错 I2C NACK。
2.  LVGL 是否正常启动，屏幕显示 UI。
3.  麦克风和扬声器测试是否通过。
