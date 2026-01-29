#include "esp_log.h"
#include "nlogger.hpp"
#include "ni2c_master.hpp"
#include "nspi_master.hpp"
#include <vector>

extern "C" void app_main() 
{
    // Initialize Logger
    NLogger logger("app_main");
    logger.Info("Starting Nix Wrapper ESP32 Example");

    // I2C Test
    {
        logger.Info("--- I2C Master Test ---");
        // Using I2C Port 0, SDA: GPIO 21, SCL: GPIO 22 (Standard ESP32 I2C pins)
        NI2CMasterBusConfig i2c_config(I2C_NUM_0, 
                                       GPIO_NUM_21, 
                                       GPIO_NUM_22,
                                       I2C_CLK_SRC_DEFAULT,
                                       7,
                                       0,
                                       0,
                                       true,
                                       false);
        NI2CMasterBus i2c_bus(logger);
        
        if (i2c_bus.Init(i2c_config) == ESP_OK) {
            // Scan for devices
            i2c_bus.Scan();
            
            // Cleanup
            i2c_bus.Deinit();
        }
    }

    // SPI Test
    {
        logger.Info("--- SPI Master Test ---");
        // Using SPI2 (HSPI), MOSI: 13, MISO: 12, SCLK: 14, CS: 15 (Standard ESP32 SPI2 pins)
        // Note: Check your board's pinout. For ESP32 DevKit V1:
        // VSPI (SPI3): MOSI: 23, MISO: 19, SCLK: 18, CS: 5
        // HSPI (SPI2): MOSI: 13, MISO: 12, SCLK: 14, CS: 15
        
        NSpiMasterBusConfig spi_config(SPI2_HOST, 
                                     GPIO_NUM_13, // MOSI
                                     GPIO_NUM_12, // MISO
                                     GPIO_NUM_14, // SCLK
                                     4092, 
                                     SPI_DMA_CH_AUTO,
                                     SPICOMMON_BUSFLAG_MASTER);
                                     
        NSpiMasterBus spi_bus(logger);
        
        if (spi_bus.Init(spi_config) == ESP_OK) {
            logger.Info("SPI Bus initialized successfully");
            
            // SPI Bus usually doesn't support scanning like I2C, 
            // so we just init and deinit to verify driver loading.
            
            // Cleanup
            spi_bus.Deinit();
        }
    }
    
    logger.Info("Example Finished");
}
