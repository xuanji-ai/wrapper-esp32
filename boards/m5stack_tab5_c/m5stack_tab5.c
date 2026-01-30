#include "m5stack_tab5.h"
#include "m5stack_tab5_disp_init_data.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "esp_io_expander_pi4ioe5v6408.h"
#include "esp_lcd_ili9881c.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_codec_dev_defaults.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "esp_ldo_regulator.h"
#include "es8388.h"
#include "es7210.h"
#include <string.h>

static const char *TAG = "m5stack_tab5";

// Static Global Variables
static i2c_master_bus_handle_t s_i2c_bus = NULL;

static esp_io_expander_handle_t s_io_expander = NULL;
static esp_io_expander_handle_t s_io_expander1 = NULL;

static esp_lcd_panel_io_handle_t s_lcd_io = NULL;
static esp_lcd_panel_handle_t s_lcd_panel = NULL;
static void *s_lcd_dsi_bus = NULL; // esp_lcd_dsi_bus_handle_t
static void *s_lcd_phy_pwr = NULL; // esp_ldo_channel_handle_t

static esp_lcd_touch_handle_t s_touch = NULL;

static i2s_chan_handle_t s_i2s_tx = NULL;
static i2s_chan_handle_t s_i2s_rx = NULL;
static const audio_codec_data_if_t *s_audio_data_if = NULL;
static const audio_codec_ctrl_if_t *s_audio_ctrl_if = NULL;
static const audio_codec_if_t *s_audio_codec_spk = NULL;
static const audio_codec_if_t *s_audio_codec_mic = NULL;
static esp_codec_dev_handle_t s_codec_dev_spk = NULL;
static esp_codec_dev_handle_t s_codec_dev_mic = NULL;

static sdmmc_card_t *s_sd_card = NULL;
static void *s_sd_pwr_ctrl = NULL; // sd_pwr_ctrl_handle_t

#define BSP_CHECK(x, ret, format, ...) do { \
    if (!(x)) { \
        ESP_LOGE(TAG, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return ret; \
    } \
} while(0)

#define BSP_CHECK_ERR(x, format, ...) do { \
    esp_err_t err_rc_ = (x); \
    if (err_rc_ != ESP_OK) { \
        ESP_LOGE(TAG, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return err_rc_; \
    } \
} while(0)

esp_err_t m5stack_tab5_i2c_init(void)
{
    if (s_i2c_bus) {
        return ESP_OK;
    }

    i2c_master_bus_config_t i2c_conf = I2C_MASTER_BUS_CONFIG_M5TAB5();
    BSP_CHECK_ERR(i2c_new_master_bus(&i2c_conf, &s_i2c_bus), "I2C Init Failed");

    return ESP_OK;
}

esp_err_t m5stack_tab5_io_expander_init(void)
{
    if (s_io_expander && s_io_expander1) {
        return ESP_OK;
    }

    BSP_CHECK_ERR(m5stack_tab5_i2c_init(), "I2C Init Failed");

    BSP_CHECK_ERR(esp_io_expander_new_i2c_pi4ioe5v6408(s_i2c_bus, IO_EXPANDER_0_ADDR_M5TAB5, &s_io_expander), "IO Expander 0 Init Failed");
    BSP_CHECK_ERR(esp_io_expander_new_i2c_pi4ioe5v6408(s_i2c_bus, IO_EXPANDER_1_ADDR_M5TAB5, &s_io_expander1), "IO Expander 1 Init Failed");

    return ESP_OK;
}

static esp_err_t m5stack_tab5_enable_dsi_phy_power(void)
{
    esp_ldo_channel_config_t ldo_cfg = LDO_DSI_PHY_CONFIG_M5TAB5();
    
    if (esp_ldo_acquire_channel(&ldo_cfg, (esp_ldo_channel_handle_t*)&s_lcd_phy_pwr) == ESP_OK) {
        ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
    } else {
        ESP_LOGW(TAG, "Failed to acquire LDO for DSI PHY (might be already enabled)");
    }
    return ESP_OK;
}

void m5stack_tab5_display_backlight_set(int brightness_percent)
{
    if (brightness_percent > 100) brightness_percent = 100;
    if (brightness_percent < 0) brightness_percent = 0;

    uint32_t duty_cycle = (1023 * brightness_percent) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void m5stack_tab5_display_on(void)
{
    if (s_lcd_panel) {
        esp_lcd_panel_disp_on_off(s_lcd_panel, true);
        m5stack_tab5_display_backlight_set(100);
    }
}

void m5stack_tab5_display_off(void)
{
    if (s_lcd_panel) {
        m5stack_tab5_display_backlight_set(0);
        esp_lcd_panel_disp_on_off(s_lcd_panel, false);
    }
}

esp_err_t m5stack_tab5_display_init(void)
{
    if (s_lcd_panel) {
        return ESP_OK;
    }

    BSP_CHECK_ERR(m5stack_tab5_io_expander_init(), "IO Expander Init Failed");

    // Enable LCD power via IO Expander
    esp_io_expander_set_dir(s_io_expander, M5TAB5_LCD_EN_PIN, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(s_io_expander, M5TAB5_LCD_EN_PIN, 1);
    
    // Backlight Init (LEDC)
    ledc_timer_config_t ledc_timer = LEDC_TIMER_CONFIG_M5TAB5();
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = LEDC_CHANNEL_CONFIG_M5TAB5();
    ledc_channel_config(&ledc_channel);

    m5stack_tab5_enable_dsi_phy_power();

    // MIPI DSI Bus
    esp_lcd_dsi_bus_config_t bus_config = DSI_BUS_CONFIG_M5TAB5();
    BSP_CHECK_ERR(esp_lcd_new_dsi_bus(&bus_config, (esp_lcd_dsi_bus_handle_t*)&s_lcd_dsi_bus), "DSI Bus Init Failed");

    // Panel IO (DBI)
    esp_lcd_dbi_io_config_t dbi_config = DBI_IO_CONFIG_M5TAB5();
    BSP_CHECK_ERR(esp_lcd_new_panel_io_dbi(s_lcd_dsi_bus, &dbi_config, &s_lcd_io), "Panel IO Init Failed");

    // DPI Panel Config
    esp_lcd_dpi_panel_config_t dpi_config = DPI_PANEL_CONFIG_M5TAB5();

    ili9881c_vendor_config_t vendor_config = ILI9881C_VENDOR_CONFIG_M5TAB5();
    // Set runtime handles
    vendor_config.mipi_config.dsi_bus = s_lcd_dsi_bus;
    vendor_config.mipi_config.dpi_config = &dpi_config;

    esp_lcd_panel_dev_config_t panel_config = LCD_PANEL_DEV_CONFIG_M5TAB5();
    // Set runtime handles
    panel_config.vendor_config = &vendor_config;

    BSP_CHECK_ERR(esp_lcd_new_panel_ili9881c(s_lcd_io, &panel_config, &s_lcd_panel), "Panel New Failed");

    esp_lcd_panel_reset(s_lcd_panel);
    esp_lcd_panel_init(s_lcd_panel);
    esp_lcd_panel_invert_color(s_lcd_panel, false);
    
    // Turn on display
    m5stack_tab5_display_on();

    return ESP_OK;
}

esp_err_t m5stack_tab5_touch_init(void)
{
    if (s_touch) {
        return ESP_OK;
    }

    BSP_CHECK_ERR(m5stack_tab5_io_expander_init(), "IO Expander Init Failed");
    
    // Enable Touch power
    esp_io_expander_set_dir(s_io_expander, M5TAB5_TOUCH_EN_PIN, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(s_io_expander, M5TAB5_TOUCH_EN_PIN, 1);
    
    // Fix for interrupt pin
    gpio_config_t int_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 1,
        .pin_bit_mask = BIT64(GPIO_NUM_23),
    };
    gpio_config(&int_gpio_config);
    gpio_set_level(GPIO_NUM_23, 0);

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = TOUCH_PANEL_IO_I2C_CONFIG_M5TAB5();
    
    BSP_CHECK_ERR(esp_lcd_new_panel_io_i2c(s_i2c_bus, &tp_io_config, &tp_io_handle), "Touch IO Init Failed");

    esp_lcd_touch_config_t tp_cfg = TOUCH_PANEL_CONFIG_M5TAB5();
    BSP_CHECK_ERR(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &s_touch), "Touch New Failed");

    return ESP_OK;
}

esp_err_t m5stack_tab5_audio_init(void)
{
    if (s_i2s_tx) {
        return ESP_OK;
    }

    BSP_CHECK_ERR(m5stack_tab5_io_expander_init(), "IO Expander Init Failed");
    
    // Enable Speaker
    esp_io_expander_set_dir(s_io_expander, M5TAB5_SPEAKER_EN_PIN, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(s_io_expander, M5TAB5_SPEAKER_EN_PIN, 1);

    i2s_chan_config_t chan_cfg = I2S_CHAN_CONFIG_M5TAB5();
    BSP_CHECK_ERR(i2s_new_channel(&chan_cfg, &s_i2s_tx, &s_i2s_rx), "I2S New Channel Failed");

    i2s_std_config_t std_cfg = I2S_STD_CONFIG_M5TAB5();

    BSP_CHECK_ERR(i2s_channel_init_std_mode(s_i2s_tx, &std_cfg), "I2S TX Init Failed");
    BSP_CHECK_ERR(i2s_channel_init_std_mode(s_i2s_rx, &std_cfg), "I2S RX Init Failed");
    
    BSP_CHECK_ERR(i2s_channel_enable(s_i2s_tx), "I2S TX Enable Failed");
    BSP_CHECK_ERR(i2s_channel_enable(s_i2s_rx), "I2S RX Enable Failed");

    // Codec Init
    audio_codec_i2s_cfg_t i2s_cfg = AUDIO_CODEC_I2S_CONFIG_M5TAB5();
    // Set runtime handles
    i2s_cfg.rx_handle = s_i2s_rx;
    i2s_cfg.tx_handle = s_i2s_tx;
    s_audio_data_if = audio_codec_new_i2s_data(&i2s_cfg);

    audio_codec_i2c_cfg_t i2c_cfg = AUDIO_CODEC_I2C_CONFIG_M5TAB5();
    // Set runtime handles
    i2c_cfg.bus_handle = s_i2c_bus;
    s_audio_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);

    // ES8388 Speaker
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    es8388_codec_cfg_t es8388_cfg = ES8388_CODEC_CONFIG_M5TAB5();
    // Set runtime handles
    es8388_cfg.ctrl_if = s_audio_ctrl_if;
    es8388_cfg.gpio_if = gpio_if;
    
    s_audio_codec_spk = es8388_codec_new(&es8388_cfg);
    
    esp_codec_dev_cfg_t dev_cfg_spk = CODEC_DEV_CONFIG_M5TAB5();
    // Set runtime handles
    dev_cfg_spk.codec_if = s_audio_codec_spk;
    dev_cfg_spk.data_if = s_audio_data_if;
    
    s_codec_dev_spk = esp_codec_dev_new(&dev_cfg_spk);

    return ESP_OK;
}

esp_err_t m5stack_tab5_sdcard_init(void)
{
    if (s_sd_card) {
        return ESP_OK;
    }

    // SD Power LDO
    sd_pwr_ctrl_ldo_config_t ldo_config = LDO_SD_PWR_CONFIG_M5TAB5();
    sd_pwr_ctrl_handle_t pwr_ctrl_handle;
    if (sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle) == ESP_OK) {
        s_sd_pwr_ctrl = pwr_ctrl_handle;
    }

    sdmmc_host_t host = SDMMC_HOST_CONFIG_M5TAB5();
    // Set runtime handles
    host.pwr_ctrl_handle = s_sd_pwr_ctrl;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_M5TAB5();

    esp_vfs_fat_sdmmc_mount_config_t mount_config = SD_MOUNT_CONFIG_M5TAB5();

    BSP_CHECK_ERR(esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &s_sd_card), "SD Card Mount Failed");

    return ESP_OK;
}

esp_err_t m5stack_tab5_init(void)
{
    BSP_CHECK_ERR(m5stack_tab5_i2c_init(), "I2C Init Failed");
    BSP_CHECK_ERR(m5stack_tab5_io_expander_init(), "IO Expander Init Failed");
    BSP_CHECK_ERR(m5stack_tab5_display_init(), "Display Init Failed");
    BSP_CHECK_ERR(m5stack_tab5_touch_init(), "Touch Init Failed");
    BSP_CHECK_ERR(m5stack_tab5_audio_init(), "Audio Init Failed");
    BSP_CHECK_ERR(m5stack_tab5_sdcard_init(), "SD Card Init Failed");
    
    // Enable other peripherals
    esp_io_expander_set_dir(s_io_expander1, M5TAB5_USB_EN_PIN, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(s_io_expander1, M5TAB5_USB_EN_PIN, 1);
    
    esp_io_expander_set_dir(s_io_expander1, M5TAB5_WIFI_EN_PIN, IO_EXPANDER_OUTPUT);
    esp_io_expander_set_level(s_io_expander1, M5TAB5_WIFI_EN_PIN, 1);
    
    return ESP_OK;
}

esp_err_t m5stack_tab5_deinit(void)
{
    // Implementation for deinit would go here
    return ESP_OK;
}

#if __has_include("esp_lvgl_port.h")
esp_err_t m5stack_tab5_lvgl_init(void)
{
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    BSP_CHECK_ERR(lvgl_port_init(&lvgl_cfg), "LVGL Port Init Failed");

    const lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = s_lcd_panel,
        .io_handle = s_lcd_io,
        .buffer_size = M5TAB5_LCD_H_RES * 100, // Example buffer size
        .double_buffer = 1,
        .hres = M5TAB5_LCD_H_RES,
        .vres = M5TAB5_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
        }
    };
    
    lvgl_port_add_disp_dsi(&disp_cfg, NULL);

    if (s_touch) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = NULL,
            .handle = s_touch,
        };
        lvgl_port_add_touch(&touch_cfg);
    }

    return ESP_OK;
}

bool m5stack_tab5_lvgl_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void m5stack_tab5_lvgl_unlock(void)
{
    lvgl_port_unlock();
}
#endif
