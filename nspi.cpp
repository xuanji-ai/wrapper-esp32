#include "nspi.hpp"
#include <cstring>

// --- SpiBus ---

using namespace nix;

SpiBus::SpiBus(Logger& logger) : m_logger(logger), m_host_id(SPI2_HOST), m_initialized(false), 
    m_config(SPI2_HOST, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, 4092, SPI_DMA_CH_AUTO, SPICOMMON_BUSFLAG_MASTER) {
}

SpiBus::~SpiBus() {
    Deinit();
}

Logger& SpiBus::GetLogger() {
    return m_logger;
}

spi_host_device_t SpiBus::GetHostId() const {
    return m_host_id;
}

esp_err_t SpiBus::Init(const SpiBusConfig& config) {
    if (m_initialized) {
        m_logger.Warning("Already initialized. Deinitializing first.");
        Deinit();
    }
    
    // Save config for Reset
    m_config = config;

    esp_err_t ret = spi_bus_initialize(config.host_id, &config, config.dma_chan);
    if (ret == ESP_OK) {
        m_host_id = config.host_id;
        m_initialized = true;
        m_logger.Info("Initialized (Host: %d, MOSI: %d, MISO: %d, SCLK: %d)", 
                     config.host_id, config.mosi_io_num, config.miso_io_num, config.sclk_io_num);
    } else {
        m_logger.Error("Failed to initialize: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t SpiBus::Deinit() {
    if (m_initialized) {
        esp_err_t ret = spi_bus_free(m_host_id);
        if (ret == ESP_OK) {
            m_logger.Info("Deinitialized");
            m_initialized = false;
        } else {
            m_logger.Error("Failed to deinitialize: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t SpiBus::Reset() {
    if (!m_initialized) {
        m_logger.Error("Cannot reset: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    m_logger.Info("Resetting...");
    
    // Deinit current bus
    esp_err_t ret = Deinit();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Re-init with saved config
    return Init(m_config);
}

// --- SpiDevice ---

SpiDevice::SpiDevice(Logger& logger) : m_logger(logger), m_dev_handle(NULL) {
}

SpiDevice::~SpiDevice() {
    Deinit();
}

Logger& SpiDevice::GetLogger() {
    return m_logger;
}

esp_err_t SpiDevice::Init(const SpiBus& bus, const SpiDeviceConfig& config) {
    if (m_dev_handle != NULL) {
        m_logger.Warning("Device already initialized. Deinitializing first.");
        Deinit();
    }

    // SPI bus must be initialized before adding device
    // We check this implicitly by bus.GetHostId(), but really the user should ensure bus is Init'd.
    // Unlike I2C new driver, SPI driver relies on Host ID.

    esp_err_t ret = spi_bus_add_device(bus.GetHostId(), &config, &m_dev_handle);
    if (ret == ESP_OK) {
        m_logger.Info("Device initialized (CS: %d, Speed: %d Hz)", config.spics_io_num, config.clock_speed_hz);
    } else {
        m_logger.Error("Failed to add device: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t SpiDevice::Deinit() {
    if (m_dev_handle != NULL) {
        esp_err_t ret = spi_bus_remove_device(m_dev_handle);
        if (ret == ESP_OK) {
            m_logger.Info("Device deinitialized");
            m_dev_handle = NULL;
        } else {
            m_logger.Error("Failed to remove device: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    return ESP_OK;
}

esp_err_t SpiDevice::Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = tx_data.size() * 8; // length is in bits
    t.tx_buffer = tx_data.data();
    
    rx_data.resize(tx_data.size());
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(m_dev_handle, &t);
}

esp_err_t SpiDevice::Write(const std::vector<uint8_t>& data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = data.size() * 8;
    t.tx_buffer = data.data();
    t.rx_buffer = NULL; // No receive

    return spi_device_transmit(m_dev_handle, &t);
}

esp_err_t SpiDevice::Read(size_t len, std::vector<uint8_t>& rx_data) {
    if (m_dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t t;
    std::memset(&t, 0, sizeof(t));
    
    t.length = len * 8;
    t.tx_buffer = NULL; // No transmit (will send 0s or random depending on half-duplex settings, usually 0 if not set)
    
    rx_data.resize(len);
    t.rx_buffer = rx_data.data();

    return spi_device_transmit(m_dev_handle, &t);
}


