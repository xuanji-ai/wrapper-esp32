#include "ldo.hpp"
#include "esp_err.h"

namespace wrapper
{

esp_err_t LdoRegulator::Init(const LdoChannelConfig &config)
{
    if (m_channel_handle != nullptr) {
        m_logger.Warning("LDO channel already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_ldo_acquire_channel(&config, &m_channel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to acquire LDO channel %d: %s", config.chan_id, esp_err_to_name(ret));
        m_channel_handle = nullptr;
        return ret;
    }

    m_logger.Info("LDO channel %d acquired successfully (voltage: %dmV)", config.chan_id, config.voltage_mv);
    return ESP_OK;
}

esp_err_t LdoRegulator::Deinit()
{
    if (m_channel_handle == nullptr) {
        return ESP_OK;
    }

    esp_err_t ret = esp_ldo_release_channel(m_channel_handle);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to release LDO channel: %s", esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LDO channel released successfully");
    m_channel_handle = nullptr;
    return ESP_OK;
}

esp_err_t LdoRegulator::AdjustVoltage(int voltage_mv)
{
    if (m_channel_handle == nullptr) {
        m_logger.Error("LDO channel not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_ldo_channel_adjust_voltage(m_channel_handle, voltage_mv);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to adjust LDO voltage to %dmV: %s", voltage_mv, esp_err_to_name(ret));
        return ret;
    }

    m_logger.Info("LDO voltage adjusted to %dmV", voltage_mv);
    return ESP_OK;
}

} // namespace wrapper
