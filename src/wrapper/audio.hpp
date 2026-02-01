#pragma once

#include <functional>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
#include "wrapper/logger.hpp"
#include "wrapper/i2c.hpp"
#include "wrapper/i2s.hpp"

namespace wrapper
{
  class AudioCodec
  {
    //common
    Logger &m_logger;
    I2sBus *m_i2s_bus = nullptr;
    const audio_codec_data_if_t *m_i2s_data_if = nullptr;
    const audio_codec_gpio_if_t *m_i2s_gpio_if = nullptr;
    //speaker
    const audio_codec_ctrl_if_t *m_spk_audio_codec_ctrl_if = nullptr;
    const audio_codec_if_t *m_spk_audio_codec_if = nullptr;
    esp_codec_dev_handle_t m_spk_codec_dev_handle = nullptr;
    bool m_spk_enabled = false;
    //microphone
    const audio_codec_ctrl_if_t *m_mic_audio_codec_ctrl_if = nullptr;
    const audio_codec_if_t *m_mic_audio_codec_if = nullptr;
    esp_codec_dev_handle_t m_mic_codec_dev_handle = nullptr;  
    bool m_mic_enabled = false;

  public:
    AudioCodec(Logger &logger);
    ~AudioCodec();

    Logger & GetLogger() const { return m_logger; }
    const audio_codec_data_if_t * GetDataInterface() const { return m_i2s_data_if; }
    const audio_codec_gpio_if_t * GetGpioInterface() const { return m_i2s_gpio_if; }
    const audio_codec_ctrl_if_t * GetSpeakerCtrlInterface() const { return m_spk_audio_codec_ctrl_if; }
    const audio_codec_ctrl_if_t * GetMicrophoneCtrlInterface() const { return m_mic_audio_codec_ctrl_if; }

    void SetSpeakerCodecInterface(const audio_codec_if_t *codec_if) { m_spk_audio_codec_if = codec_if;  }
    void SetMicrophoneCodecInterface(const audio_codec_if_t *codec_if) { m_mic_audio_codec_if = codec_if;  }
    void SetSpeakerCodecDeviceHandle(esp_codec_dev_handle_t handle) { m_spk_codec_dev_handle = handle;  }
    void SetMicrophoneCodecDeviceHandle(esp_codec_dev_handle_t handle) { m_mic_codec_dev_handle = handle;  }

    esp_err_t Init(I2sBus &i2m_bus);

    esp_err_t AddSpeaker(I2cBus &i2c_bus, uint8_t addr, std::function<esp_err_t()> codec_new_func);
    esp_err_t AddMicrophone(I2cBus &i2c_bus, uint8_t addr, std::function<esp_err_t()> codec_new_func);

    esp_err_t SetSpeakerVolume(int vol);
    esp_err_t GetSpeakerVolume(int &vol);

    esp_err_t SetSpeakerMute(bool mute);
    esp_err_t IsSpeakerMuted(bool &mute);

    esp_err_t SetMicrophoneGain(float gain);
    esp_err_t GetMicrophoneGain(float &gain);

    esp_err_t SetMicrophoneMute(bool mute);
    esp_err_t IsMicrophoneMuted(bool &mute);

    esp_err_t EnableSpeaker(bool enable);
    esp_err_t IsSpeakerEnabled(bool &enable);

    esp_err_t EnableMicrophone(bool enable);
    esp_err_t IsMicrophoneEnabled(bool &enable);

    esp_err_t Write(const void *data, size_t size);
    esp_err_t Read(void *data, size_t size);

    template<typename T>
    esp_err_t Write(const std::vector<T>& data)
    {
        return Write(data.data(), data.size() * sizeof(T));
    }

    template<typename T>
    esp_err_t Read(std::vector<T>& data, size_t count)
    {
        data.resize(count);
        return Read(data.data(), count * sizeof(T));
    }
    
    esp_err_t TestSpeaker();
    esp_err_t TestMicrophone();
  };

} // namespace wrapper