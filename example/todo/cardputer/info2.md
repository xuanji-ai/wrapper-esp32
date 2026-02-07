你提到 **PDM 麦克风没有 LRCLK 管脚**，这是完全正常的 —— **PDM（Pulse Density Modulation）麦克风本身就不需要 LRCLK（左右声道时钟）信号**。

---

### ✅ PDM 与 I2S 的区别（关键点）

| 特性 | **PDM 麦克风** | **I2S 麦克风/音频编解码器** |
|------|----------------|----------------------------|
| 数据线 | **DATA（或 DOUT）** | SD（串行数据） |
| 时钟线 | **CLK（或 PDM_CLK）** | BCLK（位时钟） + **LRCLK（帧时钟/左右声道选择）** |
| 声道支持 | 单声道（常见），部分双麦可输出立体声 | 天然支持立体声（通过 LRCLK 切换左右声道） |
| 是否需要 LRCLK | ❌ **不需要** | ✅ **必须有** |

---

### 🔍 为什么 PDM 不需要 LRCLK？

- PDM 是一种 **1-bit 过采样** 的数字音频格式。
- 它通过 **高频率的脉冲密度** 来表示模拟信号幅度。
- **单声道 PDM 麦克风** 只输出一个声道的数据流，因此只需要：
  - **PDM_CLK**：由主控（如 ESP32-S3）提供时钟（通常 1–3.2 MHz）
  - **PDM_DATA**：麦克风输出的 1-bit 数据流

> 如果你使用 **两个 PDM 麦克风**（一个左、一个右），可以通过 **同一个 CLK 但不同 DATA 线** 实现“伪立体声”，但每个麦克风仍是单声道，且 **仍然不需要 LRCLK**。

---

### 🛠️ ESP32-S3 如何连接 PDM 麦克风？

ESP32-S3 内置 **PDM 控制器**（属于 I2S 外设的一部分，但工作在 PDM 模式）：

#### 典型接线（单麦）：
| PDM 麦克风 | ESP32-S3 |
|-----------|---------|
| CLK       | GPIO（输出，由 ESP32 提供时钟）|
| DATA      | GPIO（输入，连接到 I2S DIN 引脚）|
| GND       | GND |
| VDD       | 3.3V（注意有些麦只支持 1.8V！）|

> ⚠️ 注意：**ESP32-S3 的 PDM 模式只能作为主机（Master）提供 CLK**，不能从外部接收 CLK（即不支持 Slave 模式下的 PDM 输入）。所以你的麦克风必须是 **被动型（无内部时钟）**，由 ESP32 提供时钟。

---

### 📌 在代码中如何配置（以 ESP-IDF 为例）？

```c
#include "driver/i2s.h"

#define PDM_MIC_CLK   GPIO_NUM_5
#define PDM_MIC_DATA  GPIO_NUM_6

i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM,
    .sample_rate = 16000,          // 最终音频采样率
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // PDM 单麦通常用 LEFT 或 RIGHT
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,     // PDM 模式下 BCLK = CLK
    .ws_io_num = I2S_PIN_NO_CHANGE,      // LRCLK 不使用
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = PDM_MIC_DATA          // DATA 引脚
};

// 设置 PDM 时钟引脚（ESP32-S3 特有）
i2s_set_pdm_clock(PDM_MIC_CLK, PDM_MIC_CLK); // 有些版本用 i2s_set_pin() 后自动处理

i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
i2s_set_pin(I2S_NUM_0, &pin_config);
i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
```

> 💡 注意：在 PDM 模式下，`ws_io_num`（即 LRCLK）设为 `I2S_PIN_NO_CHANGE` 或 `-1`，**不会被使用**。

---

### ❓常见疑问

**Q：我的 PDM 麦克风只有两个引脚（CLK 和 DATA），正常吗？**  
A：完全正常！典型 PDM 麦克风（如 Knowles SPH0645LM4H、Infineon IM67D130A）就是两线制（+电源和地）。

**Q：能用一个 PDM 麦克风录立体声吗？**  
A：不能。单个 PDM 麦克风是单声道。要立体声需两个独立 PDM 麦克风，分别接两个 DATA 引脚（ESP32-S3 支持双通道 PDM 输入）。

---

### 总结

✅ **PDM 麦克风没有 LRCLK 是正确的设计**，它仅需 **CLK + DATA** 两根信号线。  
✅ ESP32-S3 原生支持 PDM 输入，无需外接 ADC。  
✅ 开发时忽略 LRCLK，正确配置 I2S 为 PDM 模式即可。

如果你提供具体麦克风型号（如 SPH0641、MP34DT01 等）或遇到录音无声/噪声大等问题，我可以进一步帮你调试！