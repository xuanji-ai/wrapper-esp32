#include "wrapper/i2s.hpp"

using namespace wrapper;

I2sBus::~I2sBus()
{
    if (tx_chan_handle_ != NULL) {
        i2s_channel_disable(tx_chan_handle_);
        i2s_del_channel(tx_chan_handle_);
        tx_chan_handle_ = NULL;
    }
    if (rx_chan_handle_ != NULL) {
        i2s_channel_disable(rx_chan_handle_);
        i2s_del_channel(rx_chan_handle_);
        rx_chan_handle_ = NULL;
    }
}

esp_err_t I2sBus::Init(I2sBusConfig& bus_config)
{
    if (tx_chan_handle_ != NULL || rx_chan_handle_ != NULL) {
        logger_.Warning("Already initialized. Deinitializing first.");
        if (tx_chan_handle_) {
            i2s_channel_disable(tx_chan_handle_);
            i2s_del_channel(tx_chan_handle_);
            tx_chan_handle_ = NULL;
        }
        if (rx_chan_handle_) {
            i2s_channel_disable(rx_chan_handle_);
            i2s_del_channel(rx_chan_handle_);
            rx_chan_handle_ = NULL;
        }
    }

    esp_err_t ret = i2s_new_channel(&bus_config, &tx_chan_handle_, &rx_chan_handle_);
    if (ret == ESP_OK) {
        port_ = bus_config.id;
        logger_.Info("Initialized (Port: %d, Role: %d)", bus_config.id, bus_config.role);
    } else {
        logger_.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }

    port_ = bus_config.id;
    return ret;
}

esp_err_t I2sBus::ConfigureTxChannel(I2sChanStdConfig& chan_config)
{
    if (tx_chan_handle_ == NULL) {
        logger_.Error("TX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_std_mode(tx_chan_handle_, &chan_config);
    if (ret != ESP_OK) {
        logger_.Error("Failed to init STD TX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    // ret = i2s_channel_enable(tx_chan_handle_);
    // if (ret == ESP_OK) {
    //     logger_.Info("STD TX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    // } else {
    //     logger_.Error("Failed to enable TX Channel: %s", esp_err_to_name(ret));
    // }

    tx_sample_rate_hz_ = chan_config.clk_cfg.sample_rate_hz;
    return ret;
}

esp_err_t I2sBus::ConfigureTxChannel(I2SChanTdmConfig& chan_config)
{
    if (tx_chan_handle_ == NULL) {
        logger_.Error("TX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_tdm_mode(tx_chan_handle_, &chan_config);
    if (ret != ESP_OK) {
        logger_.Error("Failed to init TDM TX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    // ret = i2s_channel_enable(tx_chan_handle_);
    // if (ret == ESP_OK) {
    //     logger_.Info("TDM TX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    // } else {
    //     logger_.Error("Failed to enable TX Channel: %s", esp_err_to_name(ret));
    // }

    tx_sample_rate_hz_ = chan_config.clk_cfg.sample_rate_hz;
    return ret;
}

esp_err_t I2sBus::EnableTxChannel()
{
    if (tx_chan_handle_ == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2s_channel_enable(tx_chan_handle_);
}

esp_err_t I2sBus::EnableRxChannel()
{
    if (rx_chan_handle_ == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2s_channel_enable(rx_chan_handle_);
}

esp_err_t I2sBus::DisableTxChannel()
{
    if (tx_chan_handle_ == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2s_channel_disable(tx_chan_handle_);
}

esp_err_t I2sBus::DisableRxChannel()
{
    if (rx_chan_handle_ == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return i2s_channel_disable(rx_chan_handle_);
}

esp_err_t I2sBus::ConfigureRxChannel(I2sChanStdConfig& chan_config)
{
    if (rx_chan_handle_ == NULL) {
        logger_.Error("RX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_std_mode(rx_chan_handle_, &chan_config);
    if (ret != ESP_OK) {
        logger_.Error("Failed to init STD RX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    // ret = i2s_channel_enable(rx_chan_handle_);
    // if (ret == ESP_OK) {
    //     logger_.Info("STD RX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    // } else {
    //     logger_.Error("Failed to enable RX Channel: %s", esp_err_to_name(ret));
    // }

    rx_sample_rate_hz_ = chan_config.clk_cfg.sample_rate_hz;
    return ret;
}

esp_err_t I2sBus::ConfigureRxChannel(I2SChanTdmConfig& chan_config)
{
    if (rx_chan_handle_ == NULL) {
        logger_.Error("RX Channel handle is NULL. Call Init first.");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2s_channel_init_tdm_mode(rx_chan_handle_, &chan_config);
    if (ret != ESP_OK) {
        logger_.Error("Failed to init TDM RX mode: %s", esp_err_to_name(ret));
        return ret;
    }

    // ret = i2s_channel_enable(rx_chan_handle_);
    // if (ret == ESP_OK) {
    //     logger_.Info("TDM RX Channel enabled (Sample Rate: %lu)", chan_config.clk_cfg.sample_rate_hz);
    // } else {
    //     logger_.Error("Failed to enable RX Channel: %s", esp_err_to_name(ret));
    // }

    rx_sample_rate_hz_ = chan_config.clk_cfg.sample_rate_hz;
    return ret;
}
