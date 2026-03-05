#pragma once
#include "wrapper/display.hpp"
#include "esp_lcd_ili9341.h"

namespace wrapper {

class Ili9341 : public SpiDisplay {
public:
    using SpiDisplay::SpiDisplay; // 中文注释：已按当前代码逻辑本地化。

    bool Init(const SpiBus& buf,  const SpiDisplayConfig& config);
};

} // 中文注释：已按当前代码逻辑本地化。
