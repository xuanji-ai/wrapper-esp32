#include "m5stack_core_s3.h"
#include "m5stack_core_s3_cfg_define.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_check.h>

static const char *TAG = "M5CoreS3";

// Static Global Variables (formerly class members)

//! Bus
static i2c_master_bus_handle_t s_i2c1_bus = NULL;
static bool s_spi_initialized = false;

//! Power
static i2c_master_dev_handle_t s_pmic_dev = NULL;
static bool s_pmic_dev_configured = false;
static i2c_master_dev_handle_t s_expand_io_dev = NULL;
static bool s_expand_io_dev_configured = false;

//! Audio
static i2s_chan_handle_t s_i2s0_tx_chan = NULL;
static bool s_i2s0_tx_chan_configured = false;
static i2s_chan_handle_t s_i2s0_rx_chan = NULL;
static bool s_i2s0_rx_chan_configured = false;

static const audio_codec_data_if_t* s_i2s0_data_if = NULL;
static const audio_codec_gpio_if_t* s_i2s0_gpio_if = NULL;

static const audio_codec_ctrl_if_t* s_spk_audio_codec_ctrl_if = NULL;
static const audio_codec_if_t* s_spk_audio_codec_if = NULL;
static const audio_codec_ctrl_if_t* s_mic_i2c_ctrl_if = NULL;
static const audio_codec_if_t* s_mic_audio_codec_if = NULL;

static esp_codec_dev_handle_t s_spk_codec_dev_handle = NULL;
static esp_codec_dev_handle_t s_mic_codec_dev_handle = NULL;

//! Display
static esp_lcd_panel_io_handle_t s_lcd_panel_io = NULL;
static esp_lcd_panel_handle_t s_lcd_panel = NULL;
static bool s_lcd_panel_configured = false;
static esp_lcd_panel_io_handle_t s_lcd_touch_io = NULL;
static esp_lcd_touch_handle_t s_lcd_touch_handle = NULL;

//! LVGL Port
static bool s_lvgl_port_initialized = false;
static lv_display_t* s_lvgl_display = NULL;
static lv_indev_t* s_lvgl_touch = NULL;

//! Camera
static sensor_t* s_camera_sensor = NULL;

// Helper Function Prototypes
static int init_i2c_bus(void);
static int init_power(void);
static int init_audio(void);
static int init_display(void);
static int init_camera(void);

static void deinit_camera(void);
static void deinit_display(void);
static void deinit_audio(void);
static void deinit_power(void);
static void deinit_i2c_bus(void);

// Implementation

static int init_i2c_bus(void)
{
    esp_err_t err = ESP_OK;
    //! Bus Initialization
    //I2C1
    if(s_i2c1_bus == NULL) 
    {
        i2c_master_bus_config_t i2c_master_bus_cfg = I2C_MASTER_BUS_CONFIG_M5CS3_I2C1();
        err = i2c_new_master_bus(&i2c_master_bus_cfg, &s_i2c1_bus);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2C1 Bus: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "I2C1 Bus initialized");
    }
    //SPI3
    if(s_spi_initialized == false) 
    {
        spi_bus_config_t spi_bus_cfg = SPI_BUS_CONFIG_M5CS3_SPI3();
        err = spi_bus_initialize(SPI3_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI3 Bus: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "SPI3 Bus initialized");
        s_spi_initialized = true;
    }
    //I2S0
    if(s_i2s0_tx_chan == NULL && s_i2s0_rx_chan == NULL) 
    {
        i2s_chan_config_t i2s_chan_cfg = I2S_CHAN_CONFIG_M5CS3_I2S0();
        err = i2s_new_channel(&i2s_chan_cfg, &s_i2s0_tx_chan, &s_i2s0_rx_chan);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to initialize I2S0 Channel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "I2S0 Channel initialized");
    }
    return 0;
}

static int init_power(void)
{
    esp_err_t err = ESP_OK;
    //! PowerManager Devices Initialization
    if(s_pmic_dev == NULL) 
    {
        i2c_device_config_t pmic_deivce_cfg = I2C_DEVICE_CONFIG_AXP2101();
        err = i2c_master_bus_add_device(s_i2c1_bus, &pmic_deivce_cfg, &s_pmic_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize PMIC(AXP2101) Device: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "PMIC(AXP2101) Device initialized");
    }

    if(s_pmic_dev_configured == false)
    {
        uint8_t reg = 0x90;
        uint8_t data = 0x00;
        err = i2c_master_transmit_receive(s_pmic_dev, &reg, 1, &data, 1, 100);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read PMIC(AXP2101) Device register 0x90: %s", esp_err_to_name(err));
            return -1;
        }
        data |= 0b10110100;
        uint8_t cmds[][2] = {
            {0x90, data},
            {0x99, (uint8_t)(0b11110 - 5)},
            {0x97, (uint8_t)(0b11110 - 2)},
            {0x69, 0b00110101},
            {0x30, 0b111111},
            {0x90, 0xBF},
            {0x94, 33 - 5},
            {0x95, 33 - 5},
        };
        for(int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
            err = i2c_master_transmit(s_pmic_dev, cmds[i], 2, 100);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to configure PMIC(AXP2101) Device: %s", esp_err_to_name(err));
                return -1;
            }
        }
        ESP_LOGI(TAG, "PMIC(AXP2101) Device configured");
        s_pmic_dev_configured = true;
    }
  
    if(s_expand_io_dev == NULL) 
    {
        i2c_device_config_t expand_io_device_cfg = I2C_DEVICE_CONFIG_AW9523();
        err = i2c_master_bus_add_device(s_i2c1_bus, &expand_io_device_cfg, &s_expand_io_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize Exanpd IO(AW9523) Device: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "Exanpd IO(AW9523) Device initialized");
    }

    if(s_expand_io_dev_configured == false)
    {
        uint8_t cmds[][2] = {
            {0x02, 0b00000111},
            {0x03, 0b10001111},
            {0x04, 0b00011000},
            {0x05, 0b00001100},
            {0x11, 0b00010000},
            {0x12, 0b11111111},
            {0x13, 0b11111111},
        };
        for(int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
            err = i2c_master_transmit(s_expand_io_dev, cmds[i], 2, 100);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to configure Exanpd IO(AW9523) Device: %s", esp_err_to_name(err));
                return -1;
            }
        }
        ESP_LOGI(TAG, "Exanpd IO(AW9523) Device configured");
        s_expand_io_dev_configured = true;
    }
    return 0;
}

static int init_audio(void)
{
    esp_err_t err = ESP_OK;
    //! Audio Devices Initialization
    //i2c channel init
    if(s_i2s0_tx_chan_configured == false)
    {
        i2s_std_config_t i2s_std_cfg = I2S_STD_CONFIG_M5CS3_I2S0_TX();
        err = i2s_channel_init_std_mode(s_i2s0_tx_chan, &i2s_std_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2S0 TX Channel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "I2S0 TX Channel initialized");
        s_i2s0_tx_chan_configured = true;
    }

    if(s_i2s0_rx_chan_configured == false)
    {
        i2s_tdm_config_t i2s_tdm_cfg = I2S_TDM_CONFIG_M5CS3_I2S0_RX();
        err = i2s_channel_init_tdm_mode(s_i2s0_rx_chan, &i2s_tdm_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2S0 RX Channel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "I2S0 RX Channel initialized");
        s_i2s0_rx_chan_configured = true;
    }
  
    //speaker codec init
    if(s_i2s0_data_if == NULL) 
    {
        audio_codec_i2s_cfg_t spk_audio_codec_i2s_cfg = 
        {
            .port = I2S_NUM_0,
            .rx_handle = s_i2s0_rx_chan,
            .tx_handle = s_i2s0_tx_chan,
            .clk_src = 0
        };
        s_i2s0_data_if = audio_codec_new_i2s_data(&spk_audio_codec_i2s_cfg);
        if (s_i2s0_data_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize I2S0 Audio Codec Interface");
            return -1;
        }
        ESP_LOGI(TAG, "I2S0 Audio Codec Interface initialized");
    }

    if(s_i2s0_gpio_if == NULL) 
    {
        s_i2s0_gpio_if = audio_codec_new_gpio();
        if (s_i2s0_gpio_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize I2S0 Audio Codec GPIO Interface");
            return -1;
        }
        ESP_LOGI(TAG, "I2S0 Audio Codec GPIO Interface initialized");
    }

    if(s_spk_audio_codec_ctrl_if == NULL) 
    {
        audio_codec_i2c_cfg_t spk_audio_codec_i2c_cfg = AUDIO_CODEC_I2C_CFG_AW88298();
        spk_audio_codec_i2c_cfg.bus_handle = (void*)s_i2c1_bus;
        s_spk_audio_codec_ctrl_if = audio_codec_new_i2c_ctrl(&spk_audio_codec_i2c_cfg);
        if (s_spk_audio_codec_ctrl_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize AW88298 Codec Control Interface");
            return -1;
        }
        ESP_LOGI(TAG, "AW88298 Codec Control Interface initialized");
    }

    if(s_spk_audio_codec_if == NULL) 
    {
        aw88298_codec_cfg_t spk_aw88298_cfg = AW88298_CODEC_CFG();
        spk_aw88298_cfg.ctrl_if = s_spk_audio_codec_ctrl_if;
        spk_aw88298_cfg.gpio_if = s_i2s0_gpio_if;
        s_spk_audio_codec_if = aw88298_codec_new(&spk_aw88298_cfg);
        if (s_spk_audio_codec_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize AW88298 Codec Interface");
            return -1;
        }
        ESP_LOGI(TAG, "AW88298 Codec Interface initialized");
    }

    if(s_spk_codec_dev_handle == NULL) 
    {
        esp_codec_dev_cfg_t codec_dev_cfg = {
            .dev_type = ESP_CODEC_DEV_TYPE_OUT,
            .codec_if = s_spk_audio_codec_if,
            .data_if = s_i2s0_data_if,
        };
        s_spk_codec_dev_handle = esp_codec_dev_new(&codec_dev_cfg);
        if (s_spk_codec_dev_handle == NULL) {
            ESP_LOGE(TAG, "Failed to initialize Audio Codec Device");
            return -1;
        }
        ESP_LOGI(TAG, "Audio Codec Device initialized");
    }

    //microphone codec init
    if(s_mic_i2c_ctrl_if == NULL) 
    {
        audio_codec_i2c_cfg_t mic_audio_codec_i2c_cfg = AUDIO_CODEC_I2C_CFG_ES7210();
        mic_audio_codec_i2c_cfg.bus_handle = (void*)s_i2c1_bus;
        s_mic_i2c_ctrl_if = audio_codec_new_i2c_ctrl(&mic_audio_codec_i2c_cfg);
        if (s_mic_i2c_ctrl_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize ES7210 Codec Control Interface");
            return -1;
        }
        ESP_LOGI(TAG, "ES7210 Codec Control Interface initialized");
    }

    if(s_mic_audio_codec_if == NULL) 
    {
        es7210_codec_cfg_t mic_es7210_cfg = ES7210_CODEC_CFG();
        mic_es7210_cfg.ctrl_if = s_mic_i2c_ctrl_if;
        s_mic_audio_codec_if = es7210_codec_new(&mic_es7210_cfg);
        if (s_mic_audio_codec_if == NULL) {
            ESP_LOGE(TAG, "Failed to initialize ES7210 Codec Interface");
            return -1;
        }
        ESP_LOGI(TAG, "ES7210 Codec Interface initialized");
    }

    if(s_mic_codec_dev_handle == NULL) 
    {
        esp_codec_dev_cfg_t mic_codec_dev_cfg = {
            .dev_type = ESP_CODEC_DEV_TYPE_IN,
            .codec_if = s_mic_audio_codec_if,
            .data_if = s_i2s0_data_if,
        };
        s_mic_codec_dev_handle = esp_codec_dev_new(&mic_codec_dev_cfg);
        if (s_mic_codec_dev_handle == NULL) {
            ESP_LOGE(TAG, "Failed to initialize Microphone Codec Device");
            return -1;
        }
        ESP_LOGI(TAG, "Microphone Codec Device initialized");
    }
    return 0;
}

static int init_display(void)
{
    esp_err_t err = ESP_OK;
    //! Display Devices Initialization
    //touch panel init
    if(s_lcd_touch_io == NULL) 
    {
        esp_lcd_panel_io_i2c_config_t touch_io_cfg = ESP_LCD_PANEL_IO_I2C_CONFIG_M5CS3_TOUCH();
        err = esp_lcd_new_panel_io_i2c(s_i2c1_bus, &touch_io_cfg, &s_lcd_touch_io);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize Touch Panel IO I2C: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "Touch Panel IO I2C initialized");
    }

    if(s_lcd_touch_handle == NULL) 
    {
        esp_lcd_touch_config_t touch_cfg = ESP_LCD_TOUCH_CONFIG_M5CS3_TOUCH();
        err = esp_lcd_touch_new_i2c_ft5x06(s_lcd_touch_io, &touch_cfg, &s_lcd_touch_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize FT5x06 Touch Controller: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "FT5x06 Touch Controller initialized");
    }

    //lcd panel init
    if(s_lcd_panel_io == NULL) 
    {
        esp_lcd_panel_io_spi_config_t plane_io_cfg = ESP_LCD_PANEL_IO_SPI_CONFIG_M5CS3_DISP();
        err = esp_lcd_new_panel_io_spi(SPI3_HOST, &plane_io_cfg, &s_lcd_panel_io);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize LCD Panel IO SPI: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "LCD Panel IO SPI initialized");
    }

    if(s_lcd_panel == NULL) 
    {
        esp_lcd_panel_dev_config_t plane_dev_cfg = ESP_LCD_PANEL_DEV_CONFIG_M5CS3_DISP();
        err = esp_lcd_new_panel_ili9341(s_lcd_panel_io, &plane_dev_cfg, &s_lcd_panel);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize ILI9341 LCD Panel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "ILI9341 LCD Panel initialized");
    }

    if(s_lcd_panel_configured == false)
    {
        err = esp_lcd_panel_reset(s_lcd_panel);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reset LCD Panel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "LCD Panel reset");

        err = esp_lcd_panel_init(s_lcd_panel);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize LCD Panel: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "LCD Panel initialized");

        err = esp_lcd_panel_invert_color(s_lcd_panel, true);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to invert LCD Panel color: %s", esp_err_to_name(err));
            return -1;
        }
        ESP_LOGI(TAG, "LCD Panel color inverted");
        s_lcd_panel_configured = true;
    }
    return 0;
}

static int init_camera(void)
{
    esp_err_t err = ESP_OK;
    const camera_config_t camera_config = CAMERA_CONFIG_M5CS3_GC0308();
    err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera initialization failed with error: %d", err);
        return -1;
    }
    s_camera_sensor = esp_camera_sensor_get();
    if (s_camera_sensor == NULL)
    {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return -1;
    }
    ESP_LOGI(TAG, "Camera initialized");
    return 0;
}

static void deinit_camera(void)
{
    esp_err_t err = ESP_OK;
    if(s_camera_sensor != NULL) 
    {
        err = esp_camera_deinit();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to deinitialize Camera: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Camera deinitialized");
            s_camera_sensor = NULL;
        }
    }
}

static void deinit_display(void)
{
    esp_err_t err = ESP_OK;

    //! Display Devices Deinitialization (reverse order)
    if(s_lcd_panel != NULL) 
    {
        err = esp_lcd_panel_del(s_lcd_panel);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete LCD Panel: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "LCD Panel deleted");
            s_lcd_panel = NULL;
            s_lcd_panel_configured = false;
        }
    }

    if(s_lcd_panel_io != NULL) 
    {
        err = esp_lcd_panel_io_del(s_lcd_panel_io);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete LCD Panel IO: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "LCD Panel IO deleted");
            s_lcd_panel_io = NULL;
        }
    }

    if(s_lcd_touch_handle != NULL) 
    {
        err = esp_lcd_touch_del(s_lcd_touch_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete Touch Handle: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Touch Handle deleted");
            s_lcd_touch_handle = NULL;
        }
    }

    if(s_lcd_touch_io != NULL) 
    {
        err = esp_lcd_panel_io_del(s_lcd_touch_io);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete Touch Panel IO: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Touch Panel IO deleted");
            s_lcd_touch_io = NULL;
        }
    }
}

static void deinit_audio(void)
{
    esp_err_t err = ESP_OK;

    //! Audio Devices Deinitialization (reverse order)
    if(s_mic_codec_dev_handle != NULL) 
    {
        err = esp_codec_dev_close(s_mic_codec_dev_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to close Microphone Codec Device: %s", esp_err_to_name(err));
        }
        esp_codec_dev_delete(s_mic_codec_dev_handle);
        ESP_LOGI(TAG, "Microphone Codec Device deleted");
        s_mic_codec_dev_handle = NULL;
    }

    if(s_mic_audio_codec_if != NULL) 
    {
        audio_codec_delete_codec_if(s_mic_audio_codec_if);
        ESP_LOGI(TAG, "ES7210 Codec Interface deleted");
        s_mic_audio_codec_if = NULL;
    }

    if(s_mic_i2c_ctrl_if != NULL) 
    {
        audio_codec_delete_ctrl_if(s_mic_i2c_ctrl_if);
        ESP_LOGI(TAG, "ES7210 Codec Control Interface deleted");
        s_mic_i2c_ctrl_if = NULL;
    }

    if(s_spk_codec_dev_handle != NULL) 
    {
        err = esp_codec_dev_close(s_spk_codec_dev_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to close Speaker Codec Device: %s", esp_err_to_name(err));
        }
        esp_codec_dev_delete(s_spk_codec_dev_handle);
        ESP_LOGI(TAG, "Speaker Codec Device deleted");
        s_spk_codec_dev_handle = NULL;
    }

    if(s_spk_audio_codec_if != NULL) 
    {
        audio_codec_delete_codec_if(s_spk_audio_codec_if);
        ESP_LOGI(TAG, "AW88298 Codec Interface deleted");
        s_spk_audio_codec_if = NULL;
    }

    if(s_spk_audio_codec_ctrl_if != NULL) 
    {
        audio_codec_delete_ctrl_if(s_spk_audio_codec_ctrl_if);
        ESP_LOGI(TAG, "AW88298 Codec Control Interface deleted");
        s_spk_audio_codec_ctrl_if = NULL;
    }

    if(s_i2s0_gpio_if != NULL) 
    {
        audio_codec_delete_gpio_if(s_i2s0_gpio_if);
        ESP_LOGI(TAG, "I2S0 Audio Codec GPIO Interface deleted");
        s_i2s0_gpio_if = NULL;
    }

    if(s_i2s0_data_if != NULL) 
    {
        audio_codec_delete_data_if(s_i2s0_data_if);
        ESP_LOGI(TAG, "I2S0 Audio Codec Data Interface deleted");
        s_i2s0_data_if = NULL;
    }
}

static void deinit_power(void)
{
    esp_err_t err = ESP_OK;

    //! PowerManager Devices Deinitialization (reverse order)
    if(s_expand_io_dev != NULL) 
    {
        err = i2c_master_bus_rm_device(s_expand_io_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to remove Expand IO Device: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Expand IO(AW9523) Device removed");
            s_expand_io_dev = NULL;
            s_expand_io_dev_configured = false;
        }
    }

    if(s_pmic_dev != NULL) 
    {
        err = i2c_master_bus_rm_device(s_pmic_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to remove PMIC Device: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "PMIC(AXP2101) Device removed");
            s_pmic_dev = NULL;
            s_pmic_dev_configured = false;
        }
    }
}

static void deinit_i2c_bus(void)
{
    esp_err_t err = ESP_OK;

    //! Bus Deinitialization (reverse order)
    if(s_i2s0_tx_chan != NULL || s_i2s0_rx_chan != NULL) 
    {
        if(s_i2s0_tx_chan != NULL) {
            err = i2s_del_channel(s_i2s0_tx_chan);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to delete I2S0 TX Channel: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "I2S0 TX Channel deleted");
                s_i2s0_tx_chan = NULL;
                s_i2s0_tx_chan_configured = false;
            }
        }
    
        if(s_i2s0_rx_chan != NULL) {
            err = i2s_del_channel(s_i2s0_rx_chan);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to delete I2S0 RX Channel: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "I2S0 RX Channel deleted");
                s_i2s0_rx_chan = NULL;
                s_i2s0_rx_chan_configured = false;
            }
        }
    }

    if(s_spi_initialized) 
    {
        err = spi_bus_free(SPI3_HOST);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to free SPI3 Bus: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "SPI3 Bus freed");
            s_spi_initialized = false;
        }
    }

    if(s_i2c1_bus != NULL) 
    {
        err = i2c_del_master_bus(s_i2c1_bus);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete I2C1 Bus: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "I2C1 Bus deleted");
            s_i2c1_bus = NULL;
        }
    }
}

// Public API Implementation

int m5_core_s3_init(void)
{
    if(init_i2c_bus() != 0) {
        return -1;
    }
    if(init_power() != 0) {
        return -1;
    }
    if(init_audio() != 0) {
        return -1;
    }
    if(init_display() != 0) {
        return -1;
    }
    ESP_LOGI(TAG, "M5CoreS3 Board Initialized");
    return 0;
}

int m5_core_s3_deinit(void)
{
    deinit_camera();
    deinit_display();
    deinit_audio();
    deinit_power();
    deinit_i2c_bus();

    ESP_LOGI(TAG, "M5CoreS3 Board Deinitialized");
    return 0;
}

void m5_core_s3_display_enable(void)
{
    if (s_lcd_panel) {
        esp_lcd_panel_disp_on_off(s_lcd_panel, true);
        ESP_LOGI(TAG, "Display enabled");
    }
}

void m5_core_s3_display_disable(void)
{
    if (s_lcd_panel) {
        esp_lcd_panel_disp_on_off(s_lcd_panel, false);
        ESP_LOGI(TAG, "Display disabled");
    }
}

void m5_core_s3_display_set_brightness(int level)
{
    if(level < 0) level = 0;
    if(level > 100) level = 100;

    const uint8_t reg_val = 20 + ((8 * level) / 100);
    uint8_t data[2] = { 0x99, reg_val };
    esp_err_t err = i2c_master_transmit(s_pmic_dev, data, sizeof(data), 100);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set display brightness: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Display brightness set to %d%%", level);
    }
}

int m5_core_s3_lvgl_port_init(void)
{
    esp_err_t err;

    err = esp_lcd_panel_disp_on_off(s_lcd_panel, true);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to turn on display: %s", esp_err_to_name(err));
    }

    if(s_lvgl_port_initialized == false)
    {
        lvgl_port_cfg_t lv_port_cfg = LVGL_PORT_CONFIG_M5CS3();
        err = lvgl_port_init(&lv_port_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize LVGL port: %s", esp_err_to_name(err));
            return -1;
        }
        s_lvgl_port_initialized = true;
        ESP_LOGI(TAG, "LVGL port initialized");
    }
  
    if(s_lvgl_display == NULL)
    {
        lvgl_port_display_cfg_t lv_port_disp_cfg = LVGL_PORT_DISPLAY_CONFIG_M5CS3();
        lv_port_disp_cfg.io_handle = s_lcd_panel_io;
        lv_port_disp_cfg.panel_handle = s_lcd_panel;
        s_lvgl_display = lvgl_port_add_disp(&lv_port_disp_cfg);
        if(s_lvgl_display == NULL)
        {
            ESP_LOGE(TAG, "Failed to add LVGL display");
            return -1;
        }
    }

    if(s_lvgl_touch == NULL)
    {
        lvgl_port_touch_cfg_t lv_port_touch_cfg = LVGL_PORT_TOUCH_CONFIG_M5CS3();
        lv_port_touch_cfg.disp = s_lvgl_display;
        lv_port_touch_cfg.handle = s_lcd_touch_handle;
        s_lvgl_touch = lvgl_port_add_touch(&lv_port_touch_cfg);
        if(s_lvgl_touch == NULL)
        {
            ESP_LOGE(TAG, "Failed to add LVGL touch");
            return -1;
        }
    }

    err = esp_lcd_panel_disp_on_off(s_lcd_panel, true);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to turn on display: %s", esp_err_to_name(err));
    }
    return 0;
}

void m5_core_s3_lvgl_port_deinit(void)
{
    esp_err_t err;

    err = esp_lcd_panel_disp_on_off(s_lcd_panel, false);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to turn off display: %s", esp_err_to_name(err));
    }

    if(s_lvgl_touch != NULL)
    {
        lvgl_port_remove_touch(s_lvgl_touch);
        s_lvgl_touch = NULL;
    }

    if(s_lvgl_display != NULL)
    {
        lvgl_port_remove_disp(s_lvgl_display);
        s_lvgl_display = NULL;
    }

    if(s_lvgl_port_initialized)
    {
        err = lvgl_port_deinit();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to deinitialize LVGL port: %s", esp_err_to_name(err));
        } else {
            s_lvgl_port_initialized = false;
            ESP_LOGI(TAG, "LVGL port deinitialized");
            // Give LVGL worker task a moment to exit cleanly before next init
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

bool m5_core_s3_lvgl_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void m5_core_s3_lvgl_unlock(void)
{
    lvgl_port_unlock();
}

int m5_core_s3_self_check(void)
{
    ESP_LOGI(TAG, "=== M5CoreS3 Board Self-Check Started ===");
  
    //! Bus Status Check
    ESP_LOGI(TAG, "--- Bus Status ---");
    if(s_i2c1_bus != NULL) {
        ESP_LOGI(TAG, "[OK] I2C1 Bus initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] I2C1 Bus not initialized");
    }
  
    if(s_spi_initialized) {
        ESP_LOGI(TAG, "[OK] SPI3 Bus initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] SPI3 Bus not initialized");
    }
  
    if(s_i2s0_tx_chan != NULL && s_i2s0_rx_chan != NULL) {
        ESP_LOGI(TAG, "[OK] I2S0 Channels created");
        if(s_i2s0_tx_chan_configured) {
            ESP_LOGI(TAG, "[OK] I2S0 TX Channel configured");
        } else {
            ESP_LOGW(TAG, "[NOT CONFIG] I2S0 TX Channel not configured");
        }
        if(s_i2s0_rx_chan_configured) {
            ESP_LOGI(TAG, "[OK] I2S0 RX Channel configured");
        } else {
            ESP_LOGW(TAG, "[NOT CONFIG] I2S0 RX Channel not configured");
        }
    } else {
        ESP_LOGW(TAG, "[NOT INIT] I2S0 Channels not created");
    }

    //! PowerManager Status Check
    ESP_LOGI(TAG, "--- Power Manager Status ---");
    if(s_pmic_dev != NULL) {
        ESP_LOGI(TAG, "[OK] PMIC(AXP2101) Device initialized");
        if(s_pmic_dev_configured) {
            ESP_LOGI(TAG, "[OK] PMIC(AXP2101) Device configured");
        } else {
            ESP_LOGW(TAG, "[NOT CONFIG] PMIC(AXP2101) Device not configured");
        }
    } else {
        ESP_LOGW(TAG, "[NOT INIT] PMIC(AXP2101) Device not initialized");
    }
  
    if(s_expand_io_dev != NULL) {
        ESP_LOGI(TAG, "[OK] Expand IO(AW9523) Device initialized");
        if(s_expand_io_dev_configured) {
            ESP_LOGI(TAG, "[OK] Expand IO(AW9523) Device configured");
        } else {
            ESP_LOGW(TAG, "[NOT CONFIG] Expand IO(AW9523) Device not configured");
        }
    } else {
        ESP_LOGW(TAG, "[NOT INIT] Expand IO(AW9523) Device not initialized");
    }

    //! Audio Devices Status Check
    ESP_LOGI(TAG, "--- Audio Devices Status ---");
    if(s_i2s0_data_if != NULL) {
        ESP_LOGI(TAG, "[OK] I2S0 Audio Codec Data Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] I2S0 Audio Codec Data Interface not initialized");
    }
  
    if(s_i2s0_gpio_if != NULL) {
        ESP_LOGI(TAG, "[OK] I2S0 Audio Codec GPIO Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] I2S0 Audio Codec GPIO Interface not initialized");
    }
  
    if(s_spk_audio_codec_ctrl_if != NULL) {
        ESP_LOGI(TAG, "[OK] AW88298 Codec Control Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] AW88298 Codec Control Interface not initialized");
    }
  
    if(s_spk_audio_codec_if != NULL) {
        ESP_LOGI(TAG, "[OK] AW88298 Codec Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] AW88298 Codec Interface not initialized");
    }
  
    if(s_spk_codec_dev_handle != NULL) {
        ESP_LOGI(TAG, "[OK] Speaker Codec Device initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] Speaker Codec Device not initialized");
    }
  
    if(s_mic_i2c_ctrl_if != NULL) {
        ESP_LOGI(TAG, "[OK] ES7210 Codec Control Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] ES7210 Codec Control Interface not initialized");
    }
  
    if(s_mic_audio_codec_if != NULL) {
        ESP_LOGI(TAG, "[OK] ES7210 Codec Interface initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] ES7210 Codec Interface not initialized");
    }
  
    if(s_mic_codec_dev_handle != NULL) {
        ESP_LOGI(TAG, "[OK] Microphone Codec Device initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] Microphone Codec Device not initialized");
    }

    //! Display Devices Status Check
    ESP_LOGI(TAG, "--- Display Devices Status ---");
    if(s_lcd_touch_io != NULL) {
        ESP_LOGI(TAG, "[OK] Touch Panel IO initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] Touch Panel IO not initialized");
    }
  
    if(s_lcd_touch_handle != NULL) {
        ESP_LOGI(TAG, "[OK] FT5x06 Touch Controller initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] FT5x06 Touch Controller not initialized");
    }
  
    if(s_lcd_panel_io != NULL) {
        ESP_LOGI(TAG, "[OK] LCD Panel IO initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] LCD Panel IO not initialized");
    }
  
    if(s_lcd_panel != NULL) {
        ESP_LOGI(TAG, "[OK] ILI9341 LCD Panel initialized");
        if(s_lcd_panel_configured) {
            ESP_LOGI(TAG, "[OK] LCD Panel configured and ready");
        } else {
            ESP_LOGW(TAG, "[NOT CONFIG] LCD Panel not configured");
        }
    } else {
        ESP_LOGW(TAG, "[NOT INIT] ILI9341 LCD Panel not initialized");
    }

    //! Camera Status Check
    ESP_LOGI(TAG, "--- Camera Status ---");
    if(s_camera_sensor != NULL) {
        ESP_LOGI(TAG, "[OK] Camera initialized");
    } else {
        ESP_LOGW(TAG, "[NOT INIT] Camera not initialized");
    }

    ESP_LOGI(TAG, "=== M5CoreS3 Board Self-Check Completed ===");
    return 0;
}
