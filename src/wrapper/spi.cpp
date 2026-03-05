#include "wrapper/spi.hpp"
#include <cstring>

// 中文注释：已按当前代码逻辑本地化。

using namespace wrapper;

SpiBus::SpiBus(Logger& logger) : logger_(logger), host_id_(SPI2_HOST), initialized_(false), 
    config_(SPI2_HOST,                   // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             GPIO_NUM_NC,                 // 中文注释：已按当前代码逻辑本地化。
             false,                       // 中文注释：已按当前代码逻辑本地化。
             4092,                        // 中文注释：已按当前代码逻辑本地化。
             SPICOMMON_BUSFLAG_MASTER,    // 中文注释：已按当前代码逻辑本地化。
             ESP_INTR_CPU_AFFINITY_AUTO,  // 中文注释：已按当前代码逻辑本地化。
             0,                           // 中文注释：已按当前代码逻辑本地化。
             SPI_DMA_CH_AUTO) {           // 中文注释：已按当前代码逻辑本地化。
}

SpiBus::~SpiBus() {
    Deinit();
}

Logger& SpiBus::GetLogger() {
    return logger_;
}

spi_host_device_t SpiBus::GetHostId() const {
    return host_id_;
}

bool SpiBus::Init(const SpiBusConfig& config) {
    if (initialized_) {
        logger_.Warning("Already initialized. Deinitializing first.");
        Deinit();
    }
    
    // 中文注释：已按当前代码逻辑本地化。
    config_ = config;

    esp_err_t ret = spi_bus_initialize(config.host_id, &config, config.dma_chan);
    if (ret == ESP_OK) {
        host_id_ = config.host_id;
        initialized_ = true;
        logger_.Info("Initialized (Host: %d, MOSI: %d, MISO: %d, SCLK: %d)", 
                     config.host_id, config.mosi_io_num, config.miso_io_num, config.sclk_io_num);
        return true;
    } else {
        logger_.Error("Failed to initialize: %s", esp_err_to_name(ret));
        return false;
    }
}

bool SpiBus::Deinit() {
    if (initialized_) {
        esp_err_t ret = spi_bus_free(host_id_);
        if (ret == ESP_OK) {
            logger_.Info("Deinitialized");
            initialized_ = false;
            return true;
        } else {
            logger_.Error("Failed to deinitialize: %s", esp_err_to_name(ret));
            return false;
        }
    }
    return true;
}

bool SpiBus::Reset() {
    if (!initialized_) {
        logger_.Error("Cannot reset: Not initialized");
        return false;
    }

    logger_.Info("Resetting...");
    
    // 中文注释：已按当前代码逻辑本地化。
    if (!Deinit()) {
        return false;
    }
    
    // 中文注释：已按当前代码逻辑本地化。
    return Init(config_);
}

// 中文注释：已按当前代码逻辑本地化。

SpiDevice::SpiDevice(Logger& logger) : logger_(logger), dev_handle_(NULL) {
}

SpiDevice::~SpiDevice() {
    Deinit();
}

Logger& SpiDevice::GetLogger() {
    return logger_;
}

bool SpiDevice::Init(const SpiBus& bus, const SpiDeviceConfig& config) {
    if (dev_handle_ != NULL) {
        logger_.Warning("Device already initialized. Deinitializing first.");
        Deinit();
    }

    // 中文注释：已按当前代码逻辑本地化。
    // 中文注释：已按当前代码逻辑本地化。
    // 中文注释：已按当前代码逻辑本地化。

    esp_err_t ret = spi_bus_add_device(bus.GetHostId(), &config, &dev_handle_);
    if (ret == ESP_OK) {
        logger_.Info("Device initialized (CS: %d, Speed: %d Hz)", config.spics_io_num, config.clock_speed_hz);
        return true;
    } else {
        logger_.Error("Failed to add device: %s", esp_err_to_name(ret));
        return false;
    }
}

bool SpiDevice::Deinit() {
    if (dev_handle_ != NULL) {
        esp_err_t ret = spi_bus_remove_device(dev_handle_);
        if (ret == ESP_OK) {
            logger_.Info("Device deinitialized");
            dev_handle_ = NULL;
            return true;
        } else {
            logger_.Error("Failed to remove device: %s", esp_err_to_name(ret));
            return false;
        }
    }
    return true;
}

bool SpiDevice::Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data) {
    if (dev_handle_ == NULL) {
        return false;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = tx_data.size() * 8; // 中文注释：已按当前代码逻辑本地化。
    t.tx_buffer = tx_data.data();
    
    rx_data.resize(tx_data.size());
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(dev_handle_, &t) == ESP_OK;
}

bool SpiDevice::Write(const std::vector<uint8_t>& data) {
    if (dev_handle_ == NULL) {
        return false;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = data.size() * 8;
    t.tx_buffer = data.data();
    t.rx_buffer = NULL; // 中文注释：已按当前代码逻辑本地化。

    return spi_device_transmit(dev_handle_, &t) == ESP_OK;
}

bool SpiDevice::Read(size_t len, std::vector<uint8_t>& rx_data) {
    if (dev_handle_ == NULL) {
        return false;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = len * 8;
    t.tx_buffer = NULL; // 中文注释：已按当前代码逻辑本地化。
    
    rx_data.resize(len);
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(dev_handle_, &t) == ESP_OK;
}


