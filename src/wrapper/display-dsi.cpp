#include "wrapper/display-dsi.hpp"

using namespace wrapper;

DsiDisplay::DsiDisplay(Logger &logger)
    : m_logger(logger), m_dsi_bus_handle(nullptr), m_io_handle(nullptr), m_panel_handle(nullptr)
{
}

DsiDisplay::~DsiDisplay()
{
    Deinit();
}

esp_err_t DsiDisplay::Init(
    const DsiLcdConfig &config,
    LcdNewPanelFunc new_panel_func,
    CustomLcdPanelInitFunc custom_init_panel_func,
    std::function<void(void *vendor_conf)> vendor_conf_init_func)
{
    // 1. Initialize DSI bus
    esp_err_t ret = InitDsiBus(config.dsi_bus_config);
    if (ret != ESP_OK)
        return ret;

    // 2. Initialize DBI IO
    ret = InitDbiIo(config.dbi_io_config);
    if (ret != ESP_OK)
        return ret;

    // Vendor specific configuration initialization
    if (config.panel_config.vendor_config != nullptr && vendor_conf_init_func != nullptr)
    {
        vendor_conf_init_func(config.panel_config.vendor_config);
    }

    // 3. Create Panel handle with vendor config
    // Note: Vendor config needs to be set up by caller with dsi_bus and dpi_config
    ret = new_panel_func(m_io_handle, &config.panel_config, &m_panel_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to create DSI LCD panel handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Initialize panel
    return InitPanel(config.panel_config, custom_init_panel_func);
}

esp_err_t DsiDisplay::InitDsiBus(const esp_lcd_dsi_bus_config_t &config)
{
    if (m_dsi_bus_handle != nullptr)
    {
        m_logger.Warning("DSI bus already initialized. Deinitializing first.");
        Deinit();
    }

    // Create MIPI DSI bus
    esp_err_t ret = esp_lcd_new_dsi_bus(&config, &m_dsi_bus_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to create DSI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("MIPI DSI bus initialized (lanes: %d, bitrate: %d Mbps)",
                  config.num_data_lanes, config.lane_bit_rate_mbps);
    return ESP_OK;
}

esp_err_t DsiDisplay::InitDbiIo(const esp_lcd_dbi_io_config_t &config)
{
    if (m_io_handle != nullptr)
    {
        m_logger.Warning("DBI IO already initialized.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_dsi_bus_handle == nullptr)
    {
        m_logger.Error("DSI bus not initialized. Call InitDsiBus first.");
        return ESP_ERR_INVALID_STATE;
    }

    // Create DBI panel IO
    esp_err_t ret = esp_lcd_new_panel_io_dbi(m_dsi_bus_handle, &config, &m_io_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to create DBI panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("DBI panel IO initialized (vc: %d)", config.virtual_channel);
    return ESP_OK;
}

esp_err_t DsiDisplay::InitPanel(const esp_lcd_panel_dev_config_t &panel_config, CustomLcdPanelInitFunc custom_init_panel_func)
{
    if (m_io_handle == nullptr)
    {
        m_logger.Error("IO handle not initialized. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_panel_handle == nullptr)
    {
        m_logger.Error("Panel handle not created. Create it before calling InitPanel.");
        return ESP_ERR_INVALID_STATE;
    }

    // Reset the display
    esp_err_t ret = esp_lcd_panel_reset(m_panel_handle);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to reset LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the display
    if (custom_init_panel_func == nullptr)
    {
        ret = esp_lcd_panel_init(m_panel_handle);
    }
    else
    {
        ret = custom_init_panel_func(m_io_handle);
    }

    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to init LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Turn on the display
    ret = esp_lcd_panel_disp_on_off(m_panel_handle, true);
    if (ret != ESP_OK)
    {
        m_logger.Error("Failed to turn on LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("DSI LCD panel initialized");
    return ESP_OK;
}

esp_err_t DsiDisplay::Deinit()
{
    esp_err_t ret = ESP_OK;

    if (m_panel_handle != nullptr)
    {
        esp_err_t r = esp_lcd_panel_del(m_panel_handle);
        if (r != ESP_OK)
        {
            m_logger.Error("Failed to delete panel handle: %s", esp_err_to_name(r));
            ret = r;
        }
        m_panel_handle = nullptr;
    }

    if (m_io_handle != nullptr)
    {
        esp_err_t r = esp_lcd_panel_io_del(m_io_handle);
        if (r != ESP_OK)
        {
            m_logger.Error("Failed to delete IO handle: %s", esp_err_to_name(r));
            if (ret == ESP_OK)
                ret = r;
        }
        m_io_handle = nullptr;
    }

    if (m_dsi_bus_handle != nullptr)
    {
        esp_err_t r = esp_lcd_del_dsi_bus(m_dsi_bus_handle);
        if (r != ESP_OK)
        {
            m_logger.Error("Failed to delete DSI bus: %s", esp_err_to_name(r));
            if (ret == ESP_OK)
                ret = r;
        }
        m_dsi_bus_handle = nullptr;
    }

    if (ret == ESP_OK)
    {
        m_logger.Info("DSI Display deinitialized");
    }

    return ret;
}

// Panel IO operations
esp_err_t DsiDisplay::IoTxParam(int lcd_cmd, const void *param, size_t param_size)
{
    return esp_lcd_panel_io_tx_param(m_io_handle, lcd_cmd, param, param_size);
}

esp_err_t DsiDisplay::IoTxColor(int lcd_cmd, const void *color, size_t color_size)
{
    return esp_lcd_panel_io_tx_color(m_io_handle, lcd_cmd, color, color_size);
}

// Panel operations
esp_err_t DsiDisplay::Reset()
{
    return esp_lcd_panel_reset(m_panel_handle);
}

esp_err_t DsiDisplay::PanelInit()
{
    return esp_lcd_panel_init(m_panel_handle);
}

esp_err_t DsiDisplay::DrawBitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    return esp_lcd_panel_draw_bitmap(m_panel_handle, x_start, y_start, x_end, y_end, color_data);
}

esp_err_t DsiDisplay::Mirror(bool mirror_x, bool mirror_y)
{
    return esp_lcd_panel_mirror(m_panel_handle, mirror_x, mirror_y);
}

esp_err_t DsiDisplay::SwapXY(bool swap_axes)
{
    return esp_lcd_panel_swap_xy(m_panel_handle, swap_axes);
}

esp_err_t DsiDisplay::SetGap(int x_gap, int y_gap)
{
    return esp_lcd_panel_set_gap(m_panel_handle, x_gap, y_gap);
}

esp_err_t DsiDisplay::InvertColor(bool invert_color_data)
{
    return esp_lcd_panel_invert_color(m_panel_handle, invert_color_data);
}

esp_err_t DsiDisplay::DispOnOff(bool on_off)
{
    return esp_lcd_panel_disp_on_off(m_panel_handle, on_off);
}

esp_err_t DsiDisplay::DispSleep(bool sleep)
{
    return esp_lcd_panel_disp_sleep(m_panel_handle, sleep);
}
