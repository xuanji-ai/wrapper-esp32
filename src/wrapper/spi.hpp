#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ssd1306.h"
#include "wrapper/logger.hpp"
#include <vector>
#include <functional>

namespace wrapper
{

struct SpiBusConfig : public spi_bus_config_t
{
    spi_host_device_t host_id;
    spi_dma_chan_t dma_chan;

    SpiBusConfig(spi_host_device_t host,
                 int mosi,
                 int miso,
                 int sclk,
                 int quadwp,
                 int quadhd,
                 int data4,
                 int data5,
                 int data6,
                 int data7,
                 bool data_default_level,
                 int max_transfer,
                 uint32_t bus_flags,
                 esp_intr_cpu_affinity_t isr_cpu,
                 int intr_flag,
                 spi_dma_chan_t dma) : spi_bus_config_t{}
    {
        mosi_io_num = mosi;
        miso_io_num = miso;
        sclk_io_num = sclk;
        quadwp_io_num = quadwp;
        quadhd_io_num = quadhd;
        data4_io_num = data4;
        data5_io_num = data5;
        data6_io_num = data6;
        data7_io_num = data7;
        data_io_default_level = data_default_level;
        max_transfer_sz = max_transfer;
        flags = bus_flags;
        isr_cpu_id = isr_cpu;
        intr_flags = intr_flag;

        host_id = host;
        dma_chan = dma;
    }
};

class SpiBus
{
    Logger& logger_;
    spi_host_device_t host_id_;
    bool initialized_;
    SpiBusConfig config_;

public:
    SpiBus(Logger& logger);
    ~SpiBus();
    Logger& GetLogger();
    spi_host_device_t GetHostId() const;
    // ops
    bool Init(const SpiBusConfig& config);
    bool Deinit();
    bool Reset();
};

struct SpiDeviceConfig : public spi_device_interface_config_t
{
    SpiDeviceConfig(gpio_num_t cs, int clock_speed_hz, uint8_t mode) : spi_device_interface_config_t{}
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
    Logger& logger_;
    spi_device_handle_t dev_handle_;

public:
    SpiDevice(Logger& logger);
    ~SpiDevice();
    Logger& GetLogger();
    //ops
    bool Init(const SpiBus& bus, const SpiDeviceConfig& config);
    bool Deinit();
    
    // Simple transfer (write and read simultaneously)
    bool Transfer(const std::vector<uint8_t>& tx_data, std::vector<uint8_t>& rx_data);
    
    // Write only
    bool Write(const std::vector<uint8_t>& data);
    
    // Read only (sends dummy data)
    bool Read(size_t len, std::vector<uint8_t>& rx_data);
};

} // namespace wrapper