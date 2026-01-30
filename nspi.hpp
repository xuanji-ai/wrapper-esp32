#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ssd1306.h"
#include "nlogger.hpp"
#include <vector>
#include <functional>

namespace nix
{

struct SpiBusConfig : public spi_bus_config_t
{
    spi_host_device_t host_id;
    spi_dma_chan_t dma_chan;

    SpiBusConfig(spi_host_device_t host,
                        gpio_num_t mosi,
                        gpio_num_t miso,
                        gpio_num_t sclk,
                        int max_transfer,
                        spi_dma_chan_t dma,
                        uint32_t bus_flags)
    {
        mosi_io_num = mosi;
        miso_io_num = miso;
        sclk_io_num = sclk;
        quadwp_io_num = GPIO_NUM_NC;
        quadhd_io_num = GPIO_NUM_NC;
        data4_io_num = GPIO_NUM_NC;
        data5_io_num = GPIO_NUM_NC;
        data6_io_num = GPIO_NUM_NC;
        data7_io_num = GPIO_NUM_NC;
        max_transfer_sz = max_transfer;
        flags = bus_flags;
        intr_flags = 0;
        data_io_default_level = false;
        isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;

        host_id = host;
        dma_chan = dma;
    }
};

class SpiBus
{
    Logger& m_logger;
    spi_host_device_t m_host_id;
    bool m_initialized;
    SpiBusConfig m_config;

public:
    SpiBus(Logger& logger);
    ~SpiBus();
    Logger& GetLogger();
    spi_host_device_t GetHostId() const;
    
    esp_err_t Init(const SpiBusConfig& config);
    esp_err_t Deinit();
    esp_err_t Reset();
};

struct SpiDeviceConfig : public spi_device_interface_config_t
{
    SpiDeviceConfig(gpio_num_t cs, int clock_speed_hz, uint8_t mode)
    {
        command_bits = 0;
        address_bits = 0;
        dummy_bits = 0;
        this->mode = mode;
        duty_cycle_pos = 128;
        cs_ena_pretrans = 0;
        cs_ena_posttrans = 0;
        this->clock_speed_hz = clock_speed_hz;
        input_delay_ns = 0;
        spics_io_num = cs;
        flags = 0;
        queue_size = 3;
        pre_cb = NULL;
        post_cb = NULL;
    }
};

class SpiDevice
{
protected:
    Logger& m_logger;
    spi_device_handle_t m_dev_handle;

public:
    SpiDevice(Logger& logger);
    ~SpiDevice();
    Logger& GetLogger();
    
    esp_err_t Init(const SpiBus& bus, const SpiDeviceConfig& config);
    esp_err_t Deinit();
    
    // Simple transfer (write and read simultaneously)
    esp_err_t Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data);
    
    // Write only
    esp_err_t Write(const std::vector<uint8_t>& data);
    
    // Read only (sends dummy data)
    esp_err_t Read(size_t len, std::vector<uint8_t>& rx_data);
};

} // namespace nix