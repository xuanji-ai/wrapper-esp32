#include <cmath>
#include "wrapper/audio.hpp"

using namespace wrapper;

AudioCodec::AudioCodec(Logger& logger) : m_logger(logger)
{
}

AudioCodec::~AudioCodec()
{
}

esp_err_t AudioCodec::Init(I2sBus& i2s_bus)
{
    if (m_i2s_data_if == nullptr) {
        audio_codec_i2s_cfg_t i2s_cfg = {};
        i2s_cfg.port = i2s_bus.GetPort();
        i2s_cfg.rx_handle = i2s_bus.GetRxHandle();
        i2s_cfg.tx_handle = i2s_bus.GetTxHandle();
        // 0 means default clock source
        i2s_cfg.clk_src = I2S_CLK_SRC_DEFAULT; 

        m_i2s_data_if = audio_codec_new_i2s_data(&i2s_cfg);
        if (m_i2s_data_if == nullptr) {
            m_logger.Error("Failed to initialize I2S Audio Codec Interface");
            return ESP_FAIL;
        }
        m_logger.Info("I2S Audio Codec Interface initialized");
    }

    if (m_i2s_gpio_if == nullptr) {
        m_i2s_gpio_if = audio_codec_new_gpio();
        if (m_i2s_gpio_if == nullptr) {
            m_logger.Error("Failed to initialize I2S Audio Codec GPIO Interface");
            return ESP_FAIL;
        }
        m_logger.Info("I2S Audio Codec GPIO Interface initialized");
    }

    m_i2s_bus = &i2s_bus;

    return ESP_OK;
}

esp_err_t AudioCodec::AddSpeaker(I2cBus& i2c_bus, uint8_t addr,std::function<esp_err_t()> codec_new_func)
{
    if (m_spk_codec_dev_handle != nullptr) {
        m_logger.Warning("Speaker codec device already added");
        return ESP_OK;
    }

    audio_codec_i2c_cfg_t spk_i2c_cfg = {};
    spk_i2c_cfg.port = i2c_bus.GetPort();
    spk_i2c_cfg.addr = addr;
    spk_i2c_cfg.bus_handle = i2c_bus.GetHandle();
    m_spk_audio_codec_ctrl_if = audio_codec_new_i2c_ctrl(&spk_i2c_cfg);

    esp_err_t err = codec_new_func();
    if (err != ESP_OK) {
        m_logger.Error("Failed to create speaker codec device");
        return err;
    }

    m_logger.Info("Speaker codec device added successfully");
    return ESP_OK;
}

esp_err_t AudioCodec::AddMicrophone(I2cBus& i2c_bus, uint8_t addr, std::function<esp_err_t()> codec_new_func)
{
    esp_err_t err;

    if (m_mic_codec_dev_handle != nullptr) {
        m_logger.Warning("Microphone codec device already added");
        return ESP_OK;
    }

    audio_codec_i2c_cfg_t mic_i2c_cfg = {};
    mic_i2c_cfg.port = i2c_bus.GetPort();
    mic_i2c_cfg.addr = addr;
    mic_i2c_cfg.bus_handle = i2c_bus.GetHandle();
    m_mic_audio_codec_ctrl_if = audio_codec_new_i2c_ctrl(&mic_i2c_cfg);
    
    err = codec_new_func();
    if (err != ESP_OK) {
        m_logger.Error("Failed to create microphone codec device");
        return err;
    }

    m_logger.Info("Microphone codec device added successfully");
    return ESP_OK;
}

esp_err_t AudioCodec::SetSpeakerVolume(int vol)
{
    return esp_codec_dev_set_out_vol(m_spk_codec_dev_handle, vol);
}

esp_err_t AudioCodec::GetSpeakerVolume(int &vol)
{
    return esp_codec_dev_get_out_vol(m_spk_codec_dev_handle, &vol);
}

esp_err_t AudioCodec::SetSpeakerMute(bool mute)
{
    return esp_codec_dev_set_out_mute(m_spk_codec_dev_handle, mute);
}

esp_err_t AudioCodec::IsSpeakerMuted(bool &mute)
{
    return esp_codec_dev_get_out_mute(m_spk_codec_dev_handle, &mute);
}

esp_err_t AudioCodec::SetMicrophoneGain(float gain)
{
    return esp_codec_dev_set_in_gain(m_mic_codec_dev_handle, gain);
}

esp_err_t AudioCodec::GetMicrophoneGain(float &gain)
{
    return esp_codec_dev_get_in_gain(m_mic_codec_dev_handle, &gain);
}

esp_err_t AudioCodec::SetMicrophoneMute(bool mute)
{
    return esp_codec_dev_set_in_mute(m_mic_codec_dev_handle, mute);
}

esp_err_t AudioCodec::TestSpeaker()
{
    if (m_i2s_bus == nullptr) {
        m_logger.Error("I2S bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Starting speaker test: 1kHz sine wave");

    // Generate 1 second of 1kHz sine wave at 48kHz sample rate, 16-bit, stereo
    const int sample_rate = m_i2s_bus->GetTxSampleRate();
    if (sample_rate == 0) {
        m_logger.Error("I2S TX sample rate is 0");
        return ESP_ERR_INVALID_STATE;
    }

    const int duration_sec = 1;
    const int frequency = 1000;
    const int amplitude = 10000;
    const int num_samples = sample_rate * duration_sec;
    const int num_channels = 2;
    
    // Allocate buffer for 1 second of audio
    // 16-bit (2 bytes) * 2 channels * num_samples
    size_t buffer_size = num_samples * num_channels * sizeof(int16_t);
    int16_t* buffer = (int16_t*)malloc(buffer_size);
    if (buffer == nullptr) {
        m_logger.Error("Failed to allocate memory for speaker test");
        return ESP_ERR_NO_MEM;
    }

    // Generate sine wave
    for (int i = 0; i < num_samples; ++i) {
        int16_t sample = (int16_t)(amplitude * sin(2 * M_PI * frequency * i / sample_rate));
        buffer[i * 2] = sample;     // Left channel
        buffer[i * 2 + 1] = sample; // Right channel
    }

    // Write to codec
    esp_err_t ret = esp_codec_dev_write(m_spk_codec_dev_handle, buffer, buffer_size);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to write to speaker codec: %s", esp_err_to_name(ret));
    } else {
        m_logger.Info("Speaker test completed");
    }

    free(buffer);
    return ret;
}

esp_err_t AudioCodec::TestMicrophone()
{
    if (m_i2s_bus == nullptr) {
        m_logger.Error("I2S bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Starting microphone test: Recording 3 seconds...");

    const int sample_rate = m_i2s_bus->GetRxSampleRate(); // Assuming 48kHz
    if (sample_rate == 0) {
        m_logger.Error("I2S RX sample rate is 0");
        return ESP_ERR_INVALID_STATE;
    }

    const int duration_sec = 3;
    const int num_channels = 2; // Assuming stereo
    const int bytes_per_sample = sizeof(int16_t);
    
    size_t buffer_size = sample_rate * duration_sec * num_channels * bytes_per_sample;
    int16_t* buffer = (int16_t*)malloc(buffer_size);
    
    if (buffer == nullptr) {
        m_logger.Error("Failed to allocate memory for microphone test");
        return ESP_ERR_NO_MEM;
    }

    // Record
    esp_err_t ret = esp_codec_dev_read(m_mic_codec_dev_handle, buffer, buffer_size);
    // Note: esp_codec_dev_read might return error or partial read depending on implementation
    // For simplicity, we assume blocking read or sufficient data available if configured correctly.
    // However, esp_codec_dev_read prototype is (handle, data, len). It returns ESP_OK on success.
    
    if (ret != ESP_OK) {
        m_logger.Error("Failed to read from microphone codec: %s", esp_err_to_name(ret));
        free(buffer);
        return ret;
    }
    
    m_logger.Info("Recording completed. Playing back...");

    // Playback
    ret = esp_codec_dev_write(m_spk_codec_dev_handle, buffer, buffer_size);
    if (ret != ESP_OK) {
        m_logger.Error("Failed to write to speaker codec: %s", esp_err_to_name(ret));
    } else {
        m_logger.Info("Playback completed");
    }

    free(buffer);
    return ret;
}

esp_err_t AudioCodec::IsMicrophoneMuted(bool &mute)
{
    return esp_codec_dev_get_in_mute(m_mic_codec_dev_handle, &mute);
}

esp_err_t AudioCodec::EnableSpeaker(bool enable)
{
    if (m_spk_enabled == enable) {
        return ESP_OK;
    }
    
    esp_err_t ret = ESP_OK;
    if (enable) {
        if (m_i2s_bus == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 2,
            .channel_mask = 0,
            .sample_rate = m_i2s_bus->GetTxSampleRate(),
            .mclk_multiple = 0,
        };
        ret = esp_codec_dev_open(m_spk_codec_dev_handle, &fs);
    } else {
        ret = esp_codec_dev_close(m_spk_codec_dev_handle);
    }
    
    if (ret == ESP_OK) {
        m_spk_enabled = enable;
    }
    return ret;
}

esp_err_t AudioCodec::IsSpeakerEnabled(bool &enable)
{
    enable = m_spk_enabled;
    return ESP_OK;
}

esp_err_t AudioCodec::EnableMicrophone(bool enable)
{
    if (m_mic_enabled == enable) {
        return ESP_OK;
    }

    esp_err_t ret = ESP_OK;
    if (enable) {
        if (m_i2s_bus == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 2,
            .channel_mask = 0,
            .sample_rate = m_i2s_bus->GetRxSampleRate(),
            .mclk_multiple = 0,
        };
        ret = esp_codec_dev_open(m_mic_codec_dev_handle, &fs);
    } else {
        ret = esp_codec_dev_close(m_mic_codec_dev_handle);
    }

    if (ret == ESP_OK) {
        m_mic_enabled = enable;
    }
    return ret;
}

esp_err_t AudioCodec::IsMicrophoneEnabled(bool &enable)
{
    enable = m_mic_enabled;
    return ESP_OK;
}

esp_err_t AudioCodec::Write(const void *data, size_t size)
{
    return esp_codec_dev_write(m_spk_codec_dev_handle, (void*)data, size);
}

esp_err_t AudioCodec::Read(void *data, size_t size)
{
    return esp_codec_dev_read(m_mic_codec_dev_handle, data, size);
}
