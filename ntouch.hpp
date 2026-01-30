#pragma once

#include "esp_lcd_touch.h"
#include "esp_lcd_panel_io.h"
#include "ni2c.hpp"
#include "nlogger.hpp"
#include <functional>

namespace nix
{
    /**
     * @brief Define the function pointer type for creating a new touch controller handle.
     * 
     * This matches the signature of functions like esp_lcd_touch_new_i2c_ft5x06.
     */
    using I2cTouchNewFunc = std::function<esp_err_t(const esp_lcd_panel_io_handle_t, const esp_lcd_touch_config_t*, esp_lcd_touch_handle_t*)>;

    /**
     * @brief Configuration for I2C Touch controllers.
     * 
     * Combines IO configuration and controller-specific configuration.
     */
    struct I2cTouchConfig
    {
        esp_lcd_panel_io_i2c_config_t io_config;
        esp_lcd_touch_config_t touch_config;

        /**
         * @brief Construct a new I2cTouchConfig.
         * 
         * @param dev_addr I2C device address.
         * @param scl_speed_hz I2C clock speed.
         * @param x_max Maximum X coordinate.
         * @param y_max Maximum Y coordinate.
         * @param rst_gpio Reset GPIO number (GPIO_NUM_NC if not used).
         * @param int_gpio Interrupt GPIO number (GPIO_NUM_NC if not used).
         */
        I2cTouchConfig(uint16_t dev_addr,
                       uint32_t scl_speed_hz,
                       uint16_t x_max,
                       uint16_t y_max,
                       gpio_num_t rst_gpio,
                       gpio_num_t int_gpio) : io_config{}, touch_config{}
        {
            // Initialize I2C IO configuration (matching M5Stack CoreS3 requirements)
            io_config.dev_addr = dev_addr;
            io_config.on_color_trans_done = NULL;
            io_config.user_ctx = NULL;
            io_config.control_phase_bytes = 1;
            io_config.dc_bit_offset = 0;
            io_config.lcd_cmd_bits = 8;
            io_config.lcd_param_bits = 0;
            io_config.flags.dc_low_on_data = 0;
            io_config.flags.disable_control_phase = 1;
            io_config.scl_speed_hz = scl_speed_hz;

            // Initialize Touch configuration
            touch_config.x_max = x_max;
            touch_config.y_max = y_max;
            touch_config.rst_gpio_num = rst_gpio;
            touch_config.int_gpio_num = int_gpio;
            touch_config.levels.reset = 0;
            touch_config.levels.interrupt = 0;
            touch_config.flags.swap_xy = 0;
            touch_config.flags.mirror_x = 0;
            touch_config.flags.mirror_y = 0;
            touch_config.process_coordinates = NULL;
            touch_config.interrupt_callback = NULL;
        }
    };

    /**
     * @brief Class representing an I2C-based Touch controller.
     */
    class I2cTouch
    {
        Logger& m_logger;
        esp_lcd_panel_io_handle_t m_io_handle;
        esp_lcd_touch_handle_t m_touch_handle;

    public:
        /**
         * @brief Construct a new I2cTouch object.
         * 
         * @param logger Reference to a Logger instance.
         */
        I2cTouch(Logger& logger);
        
        /**
         * @brief Destroy the I2cTouch object and release resources.
         */
        ~I2cTouch();

        /**
         * @brief Initialize the touch controller.
         * 
         * @param bus The I2C bus the touch controller is connected to.
         * @param config The configuration for the touch controller.
         * @param new_touch_func Factory function to create the specific touch handle.
         * @return esp_err_t ESP_OK on success, or an error code.
         */
        esp_err_t Init(const I2cBus& bus, const I2cTouchConfig& config, I2cTouchNewFunc new_touch_func);
        
        /**
         * @brief Deinitialize the touch controller and free handles.
         * 
         * @return esp_err_t ESP_OK on success.
         */
        esp_err_t Deinit();

        /**
         * @brief Read the latest touch data from the controller.
         * 
         * @return esp_err_t ESP_OK on success.
         */
        esp_err_t ReadData();

        /**
         * @brief Get touch point data.
         * 
         * @param data Array of touch point data structures.
         * @param point_cnt Pointer to store the number of points detected.
         * @param max_point_cnt Maximum number of points to retrieve.
         * @return esp_err_t ESP_OK on success, error code otherwise.
         */
        esp_err_t GetData(esp_lcd_touch_point_data_t *data, uint8_t *point_cnt, uint8_t max_point_cnt);

        /**
         * @brief Get coordinates of touched points (deprecated wrapper for compatibility).
         * 
         * @param x Array to store X coordinates.
         * @param y Array to store Y coordinates.
         * @param strength Array to store touch strength.
         * @param point_num Pointer to store the number of points detected.
         * @param max_point_num Maximum number of points to retrieve.
         * @return true if touched, false otherwise.
         */
        bool GetCoordinates(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

        /**
         * @brief Get the underlying touch handle.
         * 
         * @return esp_lcd_touch_handle_t 
         */
        esp_lcd_touch_handle_t GetHandle() const { return m_touch_handle; }
    };

} // namespace nix
