#include "wrapper/ledc.hpp"

namespace wrapper
{

// --- LedcTimer ---

LedcTimer::LedcTimer(Logger &logger)
    : logger_(logger), speed_mode_(LEDC_LOW_SPEED_MODE), timer_num_(LEDC_TIMER_0), initialized_(false)
{
}

LedcTimer::~LedcTimer()
{
    Deinit();
}

esp_err_t LedcTimer::Init(const LedcTimerConfig &config)
{
    if (initialized_)
    {
        logger_.Warning("LEDC Timer already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_timer_config(&config);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    speed_mode_ = config.speed_mode;
    timer_num_ = config.timer_num;
    initialized_ = true;

    logger_.Info("LEDC Timer initialized (mode: %d, timer: %d, freq: %lu Hz)",
                  speed_mode_, timer_num_, config.freq_hz);
    return ESP_OK;
}

esp_err_t LedcTimer::Deinit()
{
    if (!initialized_)
    {
        return ESP_OK;
    }

    // Use deconfigure flag to deinit timer
    ledc_timer_config_t deinit_config = {};
    deinit_config.speed_mode = speed_mode_;
    deinit_config.timer_num = timer_num_;
    deinit_config.deconfigure = true;

    esp_err_t ret = ledc_timer_config(&deinit_config);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to deinitialize LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    initialized_ = false;
    logger_.Info("LEDC Timer deinitialized");
    return ESP_OK;
}

esp_err_t LedcTimer::Pause()
{
    if (!initialized_)
    {
        logger_.Error("LEDC Timer not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_timer_pause(speed_mode_, timer_num_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to pause LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Debug("LEDC Timer paused");
    return ESP_OK;
}

esp_err_t LedcTimer::Resume()
{
    if (!initialized_)
    {
        logger_.Error("LEDC Timer not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_timer_resume(speed_mode_, timer_num_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to resume LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Debug("LEDC Timer resumed");
    return ESP_OK;
}

esp_err_t LedcTimer::SetFreq(uint32_t freq_hz)
{
    if (!initialized_)
    {
        logger_.Error("LEDC Timer not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_set_freq(speed_mode_, timer_num_, freq_hz);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to set LEDC timer frequency to %lu Hz: %s", freq_hz, esp_err_to_name(ret));
        return ret;
    }

    logger_.Debug("LEDC Timer frequency set to %lu Hz", freq_hz);
    return ESP_OK;
}

// --- LedcChannel ---

LedcChannel::LedcChannel(Logger &logger)
    : logger_(logger), speed_mode_(LEDC_LOW_SPEED_MODE), channel_(LEDC_CHANNEL_0), initialized_(false)
{
}

LedcChannel::~LedcChannel()
{
    Deinit();
}

esp_err_t LedcChannel::Init(const LedcChannelConfig &config)
{
    if (initialized_)
    {
        logger_.Warning("LEDC Channel already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_channel_config(&config);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    speed_mode_ = config.speed_mode;
    channel_ = config.channel;
    initialized_ = true;

    logger_.Info("LEDC Channel initialized (mode: %d, channel: %d, gpio: %d)",
                  speed_mode_, channel_, config.gpio_num);
    return ESP_OK;
}

esp_err_t LedcChannel::Deinit()
{
    if (!initialized_)
    {
        return ESP_OK;
    }

    // Stop the channel before deinit
    esp_err_t ret = Stop(0);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to stop LEDC channel during deinit: %s", esp_err_to_name(ret));
        return ret;
    }

    initialized_ = false;
    logger_.Info("LEDC Channel deinitialized");
    return ESP_OK;
}

esp_err_t LedcChannel::SetDuty(uint32_t duty)
{
    if (!initialized_)
    {
        logger_.Error("LEDC Channel not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_set_duty(speed_mode_, channel_, duty);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to set LEDC duty to %lu: %s", duty, esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t LedcChannel::SetDutyAndUpdate(uint32_t duty)
{
    esp_err_t ret = SetDuty(duty);
    if (ret != ESP_OK)
    {
        return ret;
    }

    return UpdateDuty();
}

esp_err_t LedcChannel::UpdateDuty()
{
    if (!initialized_)
    {
        logger_.Error("LEDC Channel not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_update_duty(speed_mode_, channel_);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to update LEDC duty: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t LedcChannel::Stop(uint32_t idle_level)
{
    if (!initialized_)
    {
        logger_.Error("LEDC Channel not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ledc_stop(speed_mode_, channel_, idle_level);
    if (ret != ESP_OK)
    {
        logger_.Error("Failed to stop LEDC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    logger_.Debug("LEDC Channel stopped");
    return ESP_OK;
}

} // namespace wrapper
