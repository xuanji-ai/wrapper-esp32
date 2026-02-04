#pragma once 

#include "esp_ldo_regulator.h"
#include "logger.hpp"

namespace wrapper
{
  struct LdoChannelConfig : public esp_ldo_channel_config_t
  {
    LdoChannelConfig(
        int chan_id,
        int voltage_mv,
        bool adjustable = false,
        bool owned_by_hw = false
    ) : esp_ldo_channel_config_t{}
    {
      this->chan_id = chan_id;
      this->voltage_mv = voltage_mv;
      this->flags.adjustable = adjustable ? 1U : 0U;
      this->flags.owned_by_hw = owned_by_hw ? 1U : 0U;
      this->flags.bypass = 0U; // Deprecated
    }
  };

  class LdoRegulator
  {
  private:
    Logger &m_logger;
    esp_ldo_channel_handle_t m_channel_handle;

  public:
    LdoRegulator(Logger &logger) : m_logger(logger), m_channel_handle(nullptr) {}
    ~LdoRegulator() { Deinit(); }

    esp_err_t Init(const LdoChannelConfig &config);
    esp_err_t Deinit();
    esp_err_t AdjustVoltage(int voltage_mv);
    esp_ldo_channel_handle_t GetHandle() const { return m_channel_handle; }
  };
}