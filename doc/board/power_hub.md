# M5Stack PowerHub Hardware Specifications

## 1. 核心参数 (Core Specifications)
- **Main SoC**: ESP32-S3-WROOM-1U-N16R8
  - Processor: Dual-core Xtensa LX7 @ 240MHz
  - Flash: 16MB
  - PSRAM: 8MB
  - Wi-Fi: 2.4 GHz
- **Co-processor (MCU)**: STM32G031G8U6 (负责电源管理与 IO 扩展)

## 2. 供电与电池 (Power & Battery)
- **供电输入 (Power Input)**:
  - **DC Jack**: 9 ~ 20V (5.5 x 2.1 mm, Center Positive)
  - **Battery**: 2S Lithium Battery (Compatible with NP-F550/750/950, 7.4V)
  - **Reverse/Reflow**: 支持通过 RS485/CAN 接口反向供电 (DC 9~20V)
- **电池特性**:
  - Low voltage protection (< 6V)
  - 涓流充电/正常充电自动切换
- **输出能力 (Output Capability)**:
  - **USB/Grove/Ext 5V**: ~4.7V - 5.0V，电流视供电模式而定 (Max approx 1.3A on USB/Grove ports under battery power)
  - **RS485/CAN Output**: ~11.5V - 11.7V (Max approx 1.4A - 1.7A)

## 3. 接口与扩展 (Interfaces & Expansion)
- **通信接口 (Communication)**:
  - **RS485**: HT3.96-4P 接线端子, 带 120Ω 终端电阻开关
  - **CAN**: XT30 (2+2) 接口, 带 120Ω 终端电阻开关
  - **I2C**: 2 x HY2.0-4P (Grove Port A/C)
- **USB**:
  - 1 x USB Type-A (Output)
  - 2 x USB Type-C (1x Only Input/Download, 1x Output)
  - 支持 USB 接口切换 (USB_CON_LV1/LV2 开关)
- **扩展总线**: 2.54mm 2x8P (16 Pin) Header

## 4. I2C 地址映射 (I2C Address Map)
Power Manager Bus (PM_SDA/PM_SCL):
- **0x40**: INA226 (USB Monitor)
- **0x42**: INA226 (PORT.A Monitor)
- **0x43**: INA226 (PORT.C Monitor)
- **0x44**: INA226 (PWRCAN Monitor)
- **0x45**: INA226 (PWR485 Monitor)
- **0x46**: INA226 (Battery Monitor)
- **0x32**: RX8130CE (RTC)
- **(Addr TBD)**: SC8721 (DC-DC Converter)

## 5. 物理规格 (Physical Specs)
- **尺寸 (Dimensions)**: 88.0 x 56.0 x 38.5mm
- **重量 (Weight)**:
  - 本体 (Body): 71.0g
  - 套件含电池 (Kit with Battery): 174.8g
- **工作温度 (Operating Temp)**: 0 ~ 40°C
- **安装方式**: 磁吸、背部挂孔、乐高孔、M2 螺丝孔

## 6. 其他板载资源 (Onboard Resources)
- **RGB LED**: 8 x WS2812C
- **RTC**: RX8130CE (含 70000μF 超级电容)
- **Buttons**:
  - 1 x Reset / Download (BtnPWR)
  - 2 x User Buttons (USR_SW2, PMU_SW1)

## 7. 引脚映射 (Pin Mapping)

| Interface/Function | Net Name | ESP32 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **I2C Bus** | | | |
| I2C SDA | PM_SDA | GPIO 45 | Internal & Grove I2C SDA |
| I2C SCL | PM_SCL | GPIO 48 | Internal & Grove I2C SCL |
| **User Interface** | | | |
| User Button | USR_SW2 | GPIO 11 | User Button (Shared with BOOT?) |
| RGB LED | WS2812 | *N/A (I2C)* | Controlled via STM32 (Addr 0x50) |
| **Communication** | | | |
| RS485 TX | MCU_485_TXD | *TBD* | RS485 Transmit (Requires UART config) |
| RS485 RX | MCU_485_RXD | *TBD* | RS485 Receive (Requires UART config) |
| CAN TX | MCU_CAN_TXD | *TBD* | CAN Transmit |
| CAN RX | MCU_CAN_RXD | *TBD* | CAN Receive |
| **Power Control** | | | |
| USB Connection | USB_CON | *N/A (I2C)* | Controlled via STM32 (Addr 0x50) |

> **Note**: Most peripherals (LEDs, Power Switching, USB Mode) are managed by the STM32 Co-processor (I2C Address `0x50`).

