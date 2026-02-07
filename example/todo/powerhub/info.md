# PowerHub Info

## Specifications (规格参数)

| Specification | Parameter |
| :--- | :--- |
| **SoC** | ESP32-S3-WROOM-1U-N16R8 @ Dual-core Xtensa LX7, up to 240MHz |
| **MCU** | STM32G031G8U6 (Co-processor) |
| **Flash** | 16MB |
| **PSRAM** | 8MB |
| **Wi-Fi** | 2.4 GHz Wi-Fi |
| **RTC** | Chip: RX8130CE, Supercap: 70000μF/3.3V |
| **Expansion Ports** | 2 x HY2.0-4P (Grove), EXT 2.54-16P Header |
| **RS485** | HT3.96-4P, with 120Ω termination switch, Input: DC 9-20V |
| **CAN** | XT30 (2+2), with 120Ω termination switch, Input: DC 9-20V |
| **USB** | 1 x Type-A, 2 x Type-C (Bottom: Input/Download, Front: Output) |
| **RGB LED** | 8 x WS2812 |
| **Antenna** | SMA Connector (2.4 GHz, 2dB gain) |
| **Battery Support** | NP-F550/750/950 (Li-ion 2S @ 7.4V) |
| **DC Input** | DC 9-20V (5.5 x 2.1 mm, Center Positive) |

## Pin Mapping (管脚映射)

### RS485 & CAN (ESP32-S3)

| Interface | Signal | ESP32-S3 Pin |
| :--- | :--- | :--- |
| **RS485** | TXD | G8 |
| **RS485** | DIR | G18 |
| **RS485** | RXD | G17 |
| **CAN** | TXD | G39 |
| **CAN** | RXD | G40 |

### User Button (ESP32-S3)

| Button | Pin |
| :--- | :--- |
| USR_SW2 (Yellow Round) | G11 |

### USB Switch Control (ESP32-S3)

| Function | Pin |
| :--- | :--- |
| USB_SWITCH_L1 | G19 |
| USB_SWITCH_L2 | G20 |

### STM32G031 I2C (ESP32-S3 Connection)

| Signal | ESP32-S3 Pin | STM32 Pin |
| :--- | :--- | :--- |
| SDA | G45 | G0_BOOT (PA?) |
| SCL | G48 | - |
| **Note**: Check schematic for exact I2C mapping, web text says "STM32G031 ESP32-S3 G45 G48 G0" | | |

*Refined from text: "STM32G031 ESP32-S3 G45 G48 G0 ... SYS_SDA SYS_CL G0_BOOT"*
*   SDA: G45
*   SCL: G48
*   G0_BOOT: G0

### RGB LED & PMU Buttons (STM32G031 Controlled)

| Function | STM32 Pin |
| :--- | :--- |
| WS2812C LED_DATA | PA7 |
| WS2812C LED_EN | PB4 |
| PMU_SW2 (Side Button) | PA4 |
| PMU_SW1 (Top Button) | PA2 |

### USB Switch & Power Control (STM32G031)

| Function | STM32 Pin |
| :--- | :--- |
| USB_CON_LV1 | PB3 |
| USB_CON_LV2 | PA15 |
| INA226_PWR (nVA_EN) | PA1 |
| DC_INPUT_DETECT (VIN_DET) | PA0 |
| USB_PWR (OEN_USB) | PB8 |
| PORT.A_PWR (OEN_GRV_R) | PC14 |
| PORT.C_PWR (OEN_GRV_B) | PC15 |
| RS485_CAN_PWR (OEN_PWROUT) | PB1 |
| SC8721_DCDC_PWR (PDCDC_EN) | PA8 |
| PDCDC_REFLOW | PC6 |

### I2C Peripherals (on STM32 I2C Bus?)

*   **INA226 Addresses**:
    *   USB: 0x40
    *   PORT.A: 0x42
    *   PORT.C: 0x43
    *   PWRCAN: 0x44
    *   PWR485: 0x45
    *   Battery: 0x46
*   **Other**:
    *   SC8721
    *   RX8130CE (RTC): 0x32

### EXT 2.54-16P Header

| Pin | Function | Pin | Function |
| :--- | :--- | :--- | :--- |
| 1 | BAT-2S | 2 | BAT-2S |
| 3 | HVIN | 4 | 5VOUT |
| 5 | GND | 6 | GND |
| 7 | G43 | 8 | RST |
| 9 | G44 | 10 | nWKUP |
| 11 | G42 | 12 | G7 |
| 13 | G41 | 14 | G6 |
| 15 | G4 | 16 | G5 |

### HY2.0-4P Ports (Grove)

| Port | ESP32-S3 Pins (White/Yellow) |
| :--- | :--- |
| **PORT.A** (Red) | G15, G16 |
| **PORT.C** (Blue) | G1, G2 |

### Other ESP32-S3 Pins
| Function | Pin |
| :--- | :--- |
| EXT 5V PWR (OEN_5VO) | G14 |
