#include "ni2s.hpp"

using namespace nix;

I2sBus::~I2sBus()
{
    if (m_tx_chan_handle != NULL) {
        i2s_channel_disable(m_tx_chan_handle);
        i2s_del_channel(m_tx_chan_handle);
        m_tx_chan_handle = NULL;
    }
    if (m_rx_chan_handle != NULL) {
        i2s_channel_disable(m_rx_chan_handle);
        i2s_del_channel(m_rx_chan_handle);
        m_rx_chan_handle = NULL;
    }
}

esp_err_t I2sBus::Init(I2sBusConfig& bus_config)
{
    if (m_tx_chan_handle != NULL || m_rx_chan_handle != NULL) {
        m_logger.Warning("Already initialized. Deinitializing first.");
        if (m_tx_chan_handle) {
            i2s_channel_disable(m_tx_chan_handle);
            i2s_del_channel(m_tx_chan_handle);
            m_tx_chan_handle = NULL;
        }
        if (m_rx_chan_handle) {
            i2s_channel_disable(m_rx_chan_handle);
            i2s_del_channel(m_rx_chan_handle);
            m_rx_chan_handle = NULL;
        }
    }

    esp_err_t ret = i2s_new_channel(&bus_config, &m_tx_chan_handle, &m_rx_chan_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Initialized (Port: %d, Role: %d)", bus_config.id, bus_config.role);
    } else {
        m_logger.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2sBus::ConfigureTxChannel(I2sChanStdConfig& chan_config)
{
    if (m_tx_chan_handle == NULL) {
        m_logger.Error("TX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_std_mode(m_tx_chan_handle, &chan_config);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init STD TX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(m_tx_chan_handle);
    if (ret == ESP_OK) {
        m_logger.Info("STD TX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    } else {
        m_logger.Error("Failed to enable TX Channel: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2sBus::ConfigureTxChannel(I2SChanTdmConfig& chan_config)
{
    if (m_tx_chan_handle == NULL) {
        m_logger.Error("TX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_tdm_mode(m_tx_chan_handle, &chan_config);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init TDM TX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(m_tx_chan_handle);
    if (ret == ESP_OK) {
        m_logger.Info("TDM TX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    } else {
        m_logger.Error("Failed to enable TX Channel: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2sBus::ConfigureRxChannel(I2sChanStdConfig& chan_config)
{
    if (m_rx_chan_handle == NULL) {
        m_logger.Error("RX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_std_mode(m_rx_chan_handle, &chan_config);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init STD RX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(m_rx_chan_handle);
    if (ret == ESP_OK) {
        m_logger.Info("STD RX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    } else {
        m_logger.Error("Failed to enable RX Channel: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t I2sBus::ConfigureRxChannel(I2SChanTdmConfig& chan_config)
{
    if (m_rx_chan_handle == NULL) {
        m_logger.Error("RX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_tdm_mode(m_rx_chan_handle, &chan_config);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to init TDM RX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(m_rx_chan_handle);
    if (ret == ESP_OK) {
        m_logger.Info("TDM RX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    } else {
        m_logger.Error("Failed to enable RX Channel: %s", esp_err_to_name(ret));
    }
    return ret;
}
