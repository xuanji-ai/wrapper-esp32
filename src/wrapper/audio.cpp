#include <cmath>
#include "wrapper/audio.hpp"

using namespace wrapper;

AudioCodec::AudioCodec(Logger& logger) : logger_(logger)
{
}

AudioCodec::~AudioCodec()
{
}

bool AudioCodec::Init(I2sBus& i2s_bus)
{
    if (i2s_data_if_ == nullptr) {
        audio_codec_i2s_cfg_t i2s_cfg = {};
        i2s_cfg.port = i2s_bus.GetPort();
        i2s_cfg.rx_handle = i2s_bus.GetRxHandle();
        i2s_cfg.tx_handle = i2s_bus.GetTxHandle();
        // 0 means default clock source
        i2s_cfg.clk_src = I2S_CLK_SRC_DEFAULT; 

        i2s_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
        if (i2s_data_if_ == nullptr) {
            logger_.Error("Failed to initialize I2S Audio Codec Interface");
            return false;
        }
        logger_.Info("I2S Audio Codec Interface initialized");
    }

    if (i2s_gpio_if_ == nullptr) {
        i2s_gpio_if_ = audio_codec_new_gpio();
        if (i2s_gpio_if_ == nullptr) {
            logger_.Error("Failed to initialize I2S Audio Codec GPIO Interface");
            return false;
        }
        logger_.Info("I2S Audio Codec GPIO Interface initialized");
    }

    i2s_bus_ = &i2s_bus;

    return true;
}

bool AudioCodec::AddSpeaker(I2cBus& i2c_bus, uint8_t addr,std::function<esp_err_t()> codec_new_func)
{
    if (spk_codec_dev_handle_ != nullptr) {
        logger_.Warning("Speaker codec device already added");
        return true;
    }

    audio_codec_i2c_cfg_t spk_i2c_cfg = {};
    spk_i2c_cfg.port = i2c_bus.GetPort();
    spk_i2c_cfg.addr = addr;
    spk_i2c_cfg.bus_handle = i2c_bus.GetHandle();
    spk_audio_codec_ctrl_if_ = audio_codec_new_i2c_ctrl(&spk_i2c_cfg);

    esp_err_t err = codec_new_func();
    if (err != ESP_OK) {
        logger_.Error("Failed to create speaker codec device: %s", esp_err_to_name(err));
        return false;
    }

    logger_.Info("Speaker codec device added successfully");
    return true;
}

bool AudioCodec::AddMicrophone(I2cBus& i2c_bus, uint8_t addr, std::function<esp_err_t()> codec_new_func)
{
    esp_err_t err;

    if (mic_codec_dev_handle_ != nullptr) {
        logger_.Warning("Microphone codec device already added");
        return true;
    }

    audio_codec_i2c_cfg_t mic_i2c_cfg = {};
    mic_i2c_cfg.port = i2c_bus.GetPort();
    mic_i2c_cfg.addr = addr;
    mic_i2c_cfg.bus_handle = i2c_bus.GetHandle();
    mic_audio_codec_ctrl_if_ = audio_codec_new_i2c_ctrl(&mic_i2c_cfg);
    
    err = codec_new_func();
    if (err != ESP_OK) {
        logger_.Error("Failed to create microphone codec device: %s", esp_err_to_name(err));
        return false;
    }

    logger_.Info("Microphone codec device added successfully");
    return true;
}

bool AudioCodec::SetSpeakerVolume(int vol)
{
    esp_err_t ret = esp_codec_dev_set_out_vol(spk_codec_dev_handle_, vol);
    if (ret != ESP_OK) {
        logger_.Error("Failed to set speaker volume: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::GetSpeakerVolume(int &vol)
{
    esp_err_t ret = esp_codec_dev_get_out_vol(spk_codec_dev_handle_, &vol);
    if (ret != ESP_OK) {
        logger_.Error("Failed to get speaker volume: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::SetSpeakerMute(bool mute)
{
    esp_err_t ret = esp_codec_dev_set_out_mute(spk_codec_dev_handle_, mute);
    if (ret != ESP_OK) {
        logger_.Error("Failed to set speaker mute: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::IsSpeakerMuted(bool &mute)
{
    esp_err_t ret = esp_codec_dev_get_out_mute(spk_codec_dev_handle_, &mute);
    if (ret != ESP_OK) {
        logger_.Error("Failed to get speaker mute state: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::SetMicrophoneGain(float gain)
{
    esp_err_t ret = esp_codec_dev_set_in_gain(mic_codec_dev_handle_, gain);
    if (ret != ESP_OK) {
        logger_.Error("Failed to set microphone gain: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::GetMicrophoneGain(float &gain)
{
    esp_err_t ret = esp_codec_dev_get_in_gain(mic_codec_dev_handle_, &gain);
    if (ret != ESP_OK) {
        logger_.Error("Failed to get microphone gain: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::SetMicrophoneMute(bool mute)
{
    esp_err_t ret = esp_codec_dev_set_in_mute(mic_codec_dev_handle_, mute);
    if (ret != ESP_OK) {
        logger_.Error("Failed to set microphone mute: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::TestSpeaker()
{
    if (i2s_bus_ == nullptr) {
        logger_.Error("I2S bus not initialized");
        return false;
    }

    logger_.Info("Starting speaker test: 1kHz sine wave");

    // Generate 1 second of 1kHz sine wave at 48kHz sample rate, 16-bit, stereo
    const int sample_rate = i2s_bus_->GetTxSampleRate();
    if (sample_rate == 0) {
        logger_.Error("I2S TX sample rate is 0");
        return false;
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
        logger_.Error("Failed to allocate memory for speaker test");
        return false;
    }

    // Generate sine wave
    for (int i = 0; i < num_samples; ++i) {
        int16_t sample = (int16_t)(amplitude * sin(2 * M_PI * frequency * i / sample_rate));
        buffer[i * 2] = sample;     // Left channel
        buffer[i * 2 + 1] = sample; // Right channel
    }

    // Write to codec
    esp_err_t ret = esp_codec_dev_write(spk_codec_dev_handle_, buffer, buffer_size);
    if (ret != ESP_OK) {
        logger_.Error("Failed to write to speaker codec: %s", esp_err_to_name(ret));
    } else {
        logger_.Info("Speaker test completed");
    }

    free(buffer);
    return (ret == ESP_OK);
}

bool AudioCodec::TestMicrophone()
{
    if (i2s_bus_ == nullptr) {
        logger_.Error("I2S bus not initialized");
        return false;
    }

    logger_.Info("Starting microphone test: Recording 3 seconds...");

    const int sample_rate = i2s_bus_->GetRxSampleRate(); // Assuming 48kHz
    if (sample_rate == 0) {
        logger_.Error("I2S RX sample rate is 0");
        return false;
    }

    const int duration_sec = 3;
    const int num_channels = 2; // Assuming stereo
    const int bytes_per_sample = sizeof(int16_t);
    
    size_t buffer_size = sample_rate * duration_sec * num_channels * bytes_per_sample;
    int16_t* buffer = (int16_t*)malloc(buffer_size);
    
    if (buffer == nullptr) {
        logger_.Error("Failed to allocate memory for microphone test");
        return false;
    }

    // Record
    esp_err_t ret = esp_codec_dev_read(mic_codec_dev_handle_, buffer, buffer_size);
    
    if (ret != ESP_OK) {
        logger_.Error("Failed to read from microphone codec: %s", esp_err_to_name(ret));
        free(buffer);
        return false;
    }
    
    logger_.Info("Recording completed. Playing back...");

    // Playback
    ret = esp_codec_dev_write(spk_codec_dev_handle_, buffer, buffer_size);
    if (ret != ESP_OK) {
        logger_.Error("Failed to write to speaker codec: %s", esp_err_to_name(ret));
    } else {
        logger_.Info("Playback completed");
    }

    free(buffer);
    return (ret == ESP_OK);
}

bool AudioCodec::IsMicrophoneMuted(bool &mute)
{
    esp_err_t ret = esp_codec_dev_get_in_mute(mic_codec_dev_handle_, &mute);
    if (ret != ESP_OK) {
        logger_.Error("Failed to get microphone mute state: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::EnableSpeaker(bool enable)
{
    if (spk_enabled_ == enable) {
        return true;
    }
    
    esp_err_t ret = ESP_OK;
    if (enable) {
        if (i2s_bus_ == nullptr) {
            logger_.Error("I2S bus not initialized");
            return false;
        }
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 2,
            .channel_mask = 0,
            .sample_rate = i2s_bus_->GetTxSampleRate(),
            .mclk_multiple = 0,
        };
        ret = esp_codec_dev_open(spk_codec_dev_handle_, &fs);
    } else {
        ret = esp_codec_dev_close(spk_codec_dev_handle_);
    }
    
    if (ret == ESP_OK) {
        spk_enabled_ = enable;
        return true;
    } else {
        logger_.Error("Failed to %s speaker: %s", enable ? "enable" : "disable", esp_err_to_name(ret));
        return false;
    }
}

bool AudioCodec::IsSpeakerEnabled(bool &enable)
{
    enable = spk_enabled_;
    return true;
}

bool AudioCodec::EnableMicrophone(bool enable)
{
    if (mic_enabled_ == enable) {
        return true;
    }

    esp_err_t ret = ESP_OK;
    if (enable) {
        if (i2s_bus_ == nullptr) {
            logger_.Error("I2S bus not initialized");
            return false;
        }
        esp_codec_dev_sample_info_t fs = {
            .bits_per_sample = 16,
            .channel = 2,
            .channel_mask = 0,
            .sample_rate = i2s_bus_->GetRxSampleRate(),
            .mclk_multiple = 0,
        };
        ret = esp_codec_dev_open(mic_codec_dev_handle_, &fs);
    } else {
        ret = esp_codec_dev_close(mic_codec_dev_handle_);
    }

    if (ret == ESP_OK) {
        mic_enabled_ = enable;
        return true;
    } else {
        logger_.Error("Failed to %s microphone: %s", enable ? "enable" : "disable", esp_err_to_name(ret));
        return false;
    }
}

bool AudioCodec::IsMicrophoneEnabled(bool &enable)
{
    enable = mic_enabled_;
    return true;
}

bool AudioCodec::Write(const void *data, size_t size)
{
    esp_err_t ret = esp_codec_dev_write(spk_codec_dev_handle_, (void*)data, size);
    if (ret != ESP_OK) {
        logger_.Error("Failed to write audio data: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool AudioCodec::Read(void *data, size_t size)
{
    esp_err_t ret = esp_codec_dev_read(mic_codec_dev_handle_, data, size);
    if (ret != ESP_OK) {
        logger_.Error("Failed to read audio data: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}
