I will implement `NSpiMasterLcd` in `nspi.hpp` and `nspi_master.cpp`, following the pattern of `I2cMasterLcd`.

**Plan:**

1.  **Modify `nspi.hpp`**:
    *   Add necessary includes: `esp_lcd_panel_io.h`, `esp_lcd_panel_ops.h`, `esp_lcd_panel_dev.h`, `functional`.
    *   Define `NSpiMasterLcdNewPanelFunc` type alias.
    *   Define `NSpiMasterLcdConfig` struct:
        *   Inherit/contain `esp_lcd_panel_io_spi_config_t` and `esp_lcd_panel_dev_config_t`.
        *   Add `new_panel_func_` member.
        *   Add a constructor that initializes all members, with comments indicating recommended default values (as per previous request style).
    *   Define `NSpiMasterLcd` class:
        *   Inherit from `NSpiMasterDevice`.
        *   Add `m_io_handle` and `m_panel_handle`.
        *   Declare `Init`, `Deinit`, `Reset`, `TurnOn`, `TurnOff` and other display operation methods (mirror, swap, gap, invert, sleep, draw bitmap).

2.  **Modify `nspi_master.cpp`**:
    *   Implement `NSpiMasterLcd` methods:
        *   `Init`: Create IO handle using `esp_lcd_new_panel_io_spi`, create panel handle using `new_panel_func_`, then reset, init, and turn on the panel.
        *   `Deinit`: Delete panel and IO handles.
        *   `Reset`, `TurnOn`, `TurnOff`, `DrawBitmap`, etc.: Wrapper calls to `esp_lcd_panel_*` functions.

3.  **Verify**:
    *   Compile the project to ensure no syntax errors.
