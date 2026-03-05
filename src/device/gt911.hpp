#pragma once

#if __has_include("esp_lcd_touch_gt911.h")

#include "wrapper/touch.hpp"
#include "esp_lcd_touch_gt911.h"

namespace wrapper {

class Gt911 : public I2cTouch {
public:
    using I2cTouch::I2cTouch; // 中文注释：已按当前代码逻辑本地化。

    bool Init(const I2cBus& bus, const I2cTouchConfig& config);
};

} // 中文注释：已按当前代码逻辑本地化。

#endif
