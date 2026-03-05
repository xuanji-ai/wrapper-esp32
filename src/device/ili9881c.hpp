#pragma once
#include "wrapper/display-dsi.hpp"
#include "esp_lcd_ili9881c.h"

namespace wrapper {

class Ili9881c : public DsiDisplay {
private:
    ili9881c_vendor_config_t vendor_cfg_;

public:
    using DsiDisplay::DsiDisplay; // 中文注释：已按当前代码逻辑本地化。

    bool Init(DsiBus& bus, DsiDisplayConfig& config);
};

} // 中文注释：已按当前代码逻辑本地化。
