URL: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-7B
Content-Type: text/html
Method: native

---

---
meta-ResourceLoaderDynamicStyles:
meta-generator: MediaWiki 1.35.5
meta-og:image: https://www.waveshare.com/w/upload/1/1d/540px-Esp32-s3-touch-lcd-7b-001.jpg
meta-viewport: width=device-width, initial-scale=1
title: ESP32-S3-Touch-LCD-7B - Waveshare Wiki
---

# ESP32-S3-Touch-LCD-7B

From Waveshare Wiki

Jump to: [navigation](#mw-navigation), [search](#p-search)

- | **ESP32-S3-Touch-LCD-7B With Touch Version** |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [![ESP32-S3-Touch-LCD-7](/w/upload/thumb/1/1d/540px-Esp32-s3-touch-lcd-7b-001.jpg/360px-540px-Esp32-s3-touch-lcd-7b-001.jpg)](https://www.waveshare.com/esp32-s3-lcd-7b.htm?sku=31726 "ESP32-S3-Touch-LCD-7") ESP32-S3-N16R8, Type-C USB |
- | **ESP32-S3-LCD-7B Without Touch Version** |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [![ESP32-S3-LCD-7](/w/upload/thumb/d/da/720px-ESP32-S3-LCD-7B-1.jpg/360px-720px-ESP32-S3-LCD-7B-1.jpg)](https://www.waveshare.com/esp32-s3-lcd-7b.htm?sku=31727 "ESP32-S3-LCD-7") ESP32-S3-N16R8, Type-C USB |

# Introduction

## Introduction

This is a low-cost, high-performance MCU development board designed by Waveshare. It supports 2.4GHz WiFi and BLE 5, integrates high-capacity Flash and PSRAM, and has a 7inch wide capacitive touch LCD screen on board to smoothly run GUI interface programs such as LVGL. It combines a variety of peripheral interfaces (e.g., CAN, I2C, RS485, etc.) to quickly develop applications such as HMIs for ESP32-S3. With a wide range of functions and interfaces, it can meet power consumption requirements in Internet of Things (IoT), mobile devices, smart home and other applications.

## Features

- Equipped with high-performance Xtensa 32-bit LX7 dual-core processor, up to 240MHz main frequency
- Supports 2.4GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (BLE), with onboard antenna
- Built-in 512KB SRAM and 384KB ROM, with onboard 16MB Flash and 8MB PSRAM
- Onboard 7inch LCD display, 1024×600 resolution, 65K color
- Supports capacitive touch control via I2C interface (optional), with 5-point touch, and supports interrupts
- Onboard CAN, RS485, I2C interface and TF card slot, integrates full-speed USB
- Onboard LED indicators for indicating the power and battery charging status
- Supports adjusting the backlight brightness and reading the current battery voltage
- Supports flexible clock, precise control with individual power settings for modules, enabling low-power modes for multiple scenarios

## Onboard Resources

[![ESP32-S3-Touch-LCD-7B-details-intro.jpg](/w/upload/b/b1/ESP32-S3-Touch-LCD-7B-details-intro.jpg)](/wiki/File:ESP32-S3-Touch-LCD-7B-details-intro.jpg)

| **1. ESP32-S3-WROOM-1-N16R8 module**  WiFi Bluetooth SoC module with 240MHz operating frequency  Package 8MB PSRAM and 16MB Flash **2. Display terminal** **3. Touchscreen terminal** **4. TF card slot** **5. USB Type-C port** **6. USB TO UART Type-C port** **7. UART terminal**  6 and 7 are the same UART, selected by switch 15 **8. I2C terminal** **9. Sensor terminal** **10. CAN terminal** **11. RS485 terminal**  Screen backlight boost chip **12. 5V output** |  | **13. 3.7V single lithium battery PH2.0 socket** **14. CAN termination resistor selection** **15. RS485 termination resistor selection** **16. UART selection**  UART1 or UART2 **17. BOOT button**  Press and hold to power on for program flashing **18. RESET button**  Press to restart controller **19. DONE**  Lithium battery charging completed indicator **20. CHARGE**  Lithium battery charging indicator **21. POWER**  Power indicator |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |

# Pinouts

- **LCD port**: The port to connect the LCD cable

| ESP32-S3     | LCD          | Description                       |
| ------------ | ------------ | --------------------------------- |
| GPIO0        | G3           | Green data 3rd place              |
| GPIO1        | R3           | Red data 3rd place                |
| GPIO2        | R4           | Red data 4th place                |
| GPIO3        | VSYNC        | Vertical synchronization signal   |
| GPIO5        | DE           | Data enable signal                |
| GPIO7        | PCLK         | Clock signal                      |
| GPIO10       | B7           | Blue 7th place                    |
| GPIO14       | B3           | Blue 3rd place                    |
| GPIO17       | B6           | Blue 6th place                    |
| GPIO18       | B5           | Blue 5th place                    |
| GPIO21       | G7           | Green 7th place                   |
| GPIO38       | B4           | Blue 4th place                    |
| GPIO39       | G2           | Green 2nd place                   |
| GPIO40       | R7           | Red 7th place                     |
| GPIO41       | R6           | Red 6th place                     |
| GPIO42       | R5           | Red 5th place                     |
| GPIO45       | G4           | Green 4th place                   |
| GPIO46       | HSYNC        | Horizontal synchronization signal |
| GPIO47       | G6           | Green 6th place                   |
| GPIO48       | G5           | Green 5th place                   |
| IO EXTENSION | LCD          | -                                 |
| EXIO2        | DISP         | Backlight enable pin              |
| EXIO6        | LCD\_VDD\_EN | VCOM voltage enable pin           |

- **Touchscreen port**: The port to connect the touch cable

| ESP32-S3     | Touch   | Description         |
| ------------ | ------- | ------------------- |
| GPIO4        | TP\_IRQ | Touch interrupt pin |
| GPIO8        | TP\_SDA | Touch data pin      |
| GPIO9        | TP\_SCL | Touch clock pin     |
| IO EXTENSION | Touch   | -                   |
| EXIO1        | TP\_RST | Touch reset pin     |

- **USB port**: For power supply and flashing

| ESP32-S3     | USB      | Description                               |
| ------------ | -------- | ----------------------------------------- |
| GPIO19       | USB\_DN  | Data cable D-                             |
| GPIO20       | USB\_DP  | Data cable D+                             |
| IO EXTENSION | USB      | -                                         |
| EXIO5        | USB\_SEL | Pull down to USB mode, otherwise CAN mode |

- **TF card slot**: It is used to insert the TF card. The pin connections are shown in the table below.

| ESP32-S3     | TF     | Description                    |
| ------------ | ------ | ------------------------------ |
| GPIO11       | MOSI   | TF card input pin              |
| GPIO12       | SCK    | TF card clock pin              |
| GPIO13       | MISO   | TF card output pin             |
| IO EXTENSION | TF     | -                              |
| EXIO4        | SD\_CS | TF card enable pin, active low |

- **RS485 port**: The development board is equipped with an RS485 port, allowing direct connection for device communication, with automatic switching of the circuit's transmit and receive modes

| ESP32-S3 | RS485      | Description |
| -------- | ---------- | ----------- |
| GPIO16   | RS485\_RXD | Data input  |
| GPIO15   | RS485\_TXD | Data output |

- **CAN port**: Implements the transmission and reception control, data analysis, acquisition and monitoring of the CAN bus network

| ESP32-S3     | CAN      | Description                             |
| ------------ | -------- | --------------------------------------- |
| GPIO20       | CANTX    | Data output                             |
| GPIO19       | CANRX    | Data input                              |
| IO EXTENSION | CAN      | -                                       |
| EXIO5        | CAN\_SEL | Pull up to CAN mode, otherwise USB mode |

- **I2C port**: ESP32-S3 provides multi-channel hardware I2C, currently using GPIO8 (SDA) and GPIO9 (SCL) pins for I2C bus  
  It connects peripherals such as IO expansion chip and touch screen via the I2C port.

| ESP32-S3 | I2C | Description   |
| -------- | --- | ------------- |
| GPIO8    | SDA | I2C data pin  |
| GPIO9    | SCL | I2C clock pin |

- **PH2.0 battery socket**: The development board uses a high-efficiency charge/discharge management chip CS8501, which can boost a single lithium battery to 5V, the current charging current is 580mA, and the user can change the charging current by replacing the R45 resistor, please refer to [ESP32-S3-Touch-LCD-7B Schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-7B/ESP32-S3-Touch-LCD-7B-Schematic.pdf) for details.

## Dimensions

### Without Touch Version

[![ESP32-S3-Touch-LCD-7B-details-size1.jpg](/w/upload/9/99/ESP32-S3-Touch-LCD-7B-details-size1.jpg)](/wiki/File:ESP32-S3-Touch-LCD-7B-details-size1.jpg)

### With Touch Version

[![ESP32-S3-Touch-LCD-7B-details-size2.jpg](/w/upload/e/e2/ESP32-S3-Touch-LCD-7B-details-size2.jpg)](/wiki/File:ESP32-S3-Touch-LCD-7B-details-size2.jpg)

## Specifications

| Basic parameters        | |
| ----------------------- | ------------------------------------------------------------------------------------ |
| Processor               | High-performance 32-bit Xtensa LX7 dual-core processor with a frequency up to 240MHz |
| Wifi/Bluetooth          | Supports 2.4 GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (LE) with onboard antennas     |
| Flash                   | 16MB Flash                                                                           |
| PSRAM                   | 8MB PSRAM                                                                            |
| Power supply range      | TypeC 5V                                                                             |
| Screen parameters       | |
| Resolution              | 1024 x 600                                                                           |
| Display interface       | RGB                                                                                  |
| Display panel           | IPS                                                                                  |
| Viewing angle           | 170°                                                                                 |
| Screen brightness       | 235 cd/m²                                                                            |
| Touch type              | Capacitive                                                                           |
| Touch panel             | toughened glass                                                                      |
| Peripheral interface    | |
| Communication interface | CAN, RS485, I2C, USB                                                                 |
| Other                   | |
| Power consumption       | 5V 350mA                                                                             |
| Operating temperature   | 0℃ ~ 65℃                                                                             |
| Product size (L×W)      | Non-touch version: 164×97mm Touch version: 192.96×110.76mm                           |

# Usage Instructions

Currently there are three development tools and frameworks, **Arduino IDE**, **ESP-IDF** and **PlatformIO**, providing flexible development options, you can choose the right development tool according to your project needs and personal habits.

## Development Tool

|                                                                                                                                                                                                          |                                                                                                                                                                                                          |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [![270px-Arduino-IDE-logo.jpg](/w/upload/thumb/a/a4/270px-Arduino-IDE-logo.jpg/180px-270px-Arduino-IDE-logo.jpg)](/wiki/File:270px-Arduino-IDE-logo.jpg)                                                 | Arduino IDE Arduino IDE is an open source electronic prototyping platform, convenient and flexible, easy to get started. After a simple learning, you can start to develop quickly. At the same time, Arduino has a large global user community, providing an abundance of open source code, project examples and tutorials, as well as rich library resources, encapsulating complex functions, allowing developers to quickly implement various functions. |
| [![270px-ESP-IDF-logo.jpg](/w/upload/thumb/a/a2/270px-ESP-IDF-logo.jpg/180px-270px-ESP-IDF-logo.jpg)](/wiki/File:270px-ESP-IDF-logo.jpg)                                                                 | ESP-IDF ESP-IDF, or full name Espressif IDE, is a professional development framework introduced by Espressif Technology for the ESP series chips. It is developed using the C language, including a compiler, debugger, and flashing tool, etc., and can be developed via the command lines or through an integrated development environment (such as Visual Studio Code with the Espressif IDF plugin). The plugin offers features such as code navigation, project management, and debugging, etc. |
| [![ESP32-S3-Touch-LCD-7B-PlatformIO-07.png](/w/upload/thumb/1/1a/ESP32-S3-Touch-LCD-7B-PlatformIO-07.png/180px-ESP32-S3-Touch-LCD-7B-PlatformIO-07.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-07.png) | PlatformIO PlatformIO is a cross-platform, cross-architecture, multi-framework professional tool for embedded system engineers and software developers who write applications for embedded products.     |

Each of these three development approaches has its own advantages, and developers can choose according to their needs and skill levels. Arduino and PlatformIO (faster compilation speed) are suitable for beginners and non-professionals because they are easy to learn and quick to get started. ESP-IDF is a better choice for developers with a professional background or high performance requirements, as it provides more advanced development tools and greater control capabilities for the development of complex projects.

## Components Preparation

- ESP32-S3-Touch-LCD-7B x1
- TF card x 1 **(Not required, example required)**
- USB cable (Type-A male to Type-C male) x1
- USB TO RS485 bidirectional converter x1 **(Not required, example required)**
- USB to CAN adapter analyzer x1 **(Not required, example required)**

[![ESP32-S3-LCD-7-03.png](/w/upload/a/ab/ESP32-S3-LCD-7-03.png)](/wiki/File:ESP32-S3-LCD-7-03.png)

## Precautions

- The development board has an onboard automatic download circuit, the Type C port at the UART silk screen is used for program download and log printing. After downloading the program, press the RESET button to run the program
- Please pay attention to the PCB antenna area when using, and avoid attaching other metals or plastic parts to the PCB antenna
- The TF card uses SPI/MMC to communicate, note that the SD_CS pin needs to be driven by the EXIO4 of the IO EXTENSION
- The development board uses PH2.0 header to lead out ADC, CAN, I2C, RS485, 5V peripheral pins, and connects sensor components using PH2.0 to 2.54mm male connector
- The CAN and RS485 peripherals use use a dip switch to connect the 120 ohm resistor by default, and NC can optionally cancel the termination resistor connection
- The 7inch screen occupies the vast majority of the GPIO, and the development board uses the chip to expand the IO for reset, backlight on/off, brightness adjustment, and battery voltage reading, etc.
- **The development board uses USB to download the demo. If the port cannot be recognized, please enter boot mode (press and hold the boot button, then connect to the computer, and then release the boot button). After downloading the demo, press the RESET button to run the demo.**
- Currently, under ESP-IDF v5.3, the average frame rate limit for running the LVGL benchmark demo with a single core is 17 frames per second, corresponding to an interface PCLK rate of 30 MHz). Before compilation, ESP32 and LVGL need to be configured through menuconfig:

```
CONFIG_FREERTOS_HZ=1000
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y
CONFIG_ESPTOOLPY_FLASHMODE_QIO=y
CONFIG_ESPTOOLPY_FLASHFREQ_120M=y [Need to be consistent with PSRAM]
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_IDF_EXPERIMENTAL_FEATURES=y and CONFIG_SPIRAM_SPEED_120M=y [Need to be consistent with FLASH]
CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y
CONFIG_SPIRAM_RODATA=y
CONFIG_ESP32S3_DATA_CACHE_LINE_64B=y
CONFIG_COMPILER_OPTIMIZATION_PERF=y
#The following LVGL configuration items are helpful for frame rate improvement (LVGL v8.3):
#define LV_MEM_CUSTOM 1 or CONFIG_LV_MEM_CUSTOM=y
#define LV_MEMCPY_MEMSET_STD 1 or CONFIG_LV_MEMCPY_MEMSET_STD=y
#define LV_ATTRIBUTE_FAST_MEM IRAM_ATTR or CONFIG_LV_ATTRIBUTE_FAST_MEM=y
```

- For detailed LCD and LVGL performance descriptions, please refer to [Documentation](https://files.waveshare.com/wiki/common/Performance.pdf)
- PH2.0 lithium battery holder only supports a single 3.7V lithium battery, do not use multiple sets of battery packs to connect to charge and discharge at the same time, and it is recommended that the capacity of a single battery is less than 2000mAH
- ESP32-S3-Touch-LCD-7B occupies the following slave address, please do not use I2C devices with the same address:

```
    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
10: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
20: -  -  -  -  24 -  -  - -  -  -  -  -  -  -  -
30: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
40: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
50: -  -  -  -  -  -  -  -  -  -  -  -  -  -  5d -
60: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
70: -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
```

Before operating, it is recommended to browse the table of contents to quickly understand the document structure. For smooth operation, please read the [FAQ](#FAQ) carefully to understand possible problems in advance. All resources in the document are provided with hyperlinks for easy download.

# Working with Arduino

This chapter introduces setting up the Arduino environment, including the Arduino IDE, management of ESP32 boards, installation of related libraries, program compilation and downloading, as well as testing demos. It aims to help users master the development board and facilitate secondary development.

## Environment Setup

### Download and Install Arduino IDE

- Click to visit the [Arduino official website](https://www.arduino.cc/en/software), select the corresponding system and system bit to download The version of the Arduino IDE needs to be ≥ 1.8, and the path of installation must not be Chinese, otherwise there will be an error when compiling.  
[![ESP32-S3-AMOLED-1.91-Ar-software-01.png](/w/upload/7/73/ESP32-S3-AMOLED-1.91-Ar-software-01.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-software-01.png)
- Run the installer and install all by default

The environment setup is carried out on the Windows 10 system, Linux and Mac users can access [Arduino-esp32 environment setup](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) for reference

### Install Arduino-ESP32

- Must first install the development board **esp32-XIP-3.1.1**.
- Only supports **Install Offline**.
- For the installation tutorial, please refer to [**esp32-XIP-3.1.1** Development board installation](https://www.waveshare.com/wiki/Esp32-XIP-3.1.1_Development_Board_Installation).

*\- ESP32\-S3\-Touch\-LCD\-7B required development board installation description*

| Board name      | Board installation requirement | Instruction                                                                              |
| --------------- | ------------------------------ | ---------------------------------------------------------------------------------------- |
| esp32-XIP-3.1.1 | "Install Offline"              | **The "esp32-XIP-3.1.1" board must be installed according to the installation tutorial** |

### Install Libraries

- When installing Arduino libraries, there are usually two ways to choose from: **Install online** and **Install offline**. **If the library installation requires offline installation, you must use the provided library file**  
  For most libraries, users can easily search and install them through the online library manager of the Arduino software. However, some open-source libraries or custom libraries are not synchronized to the Arduino Library Manager, so they cannot be acquired through online searches. In this case, users can only manually install these libraries offline.
- For library installation tutorial, please refer to [Arduino library manager tutorial](https://www.waveshare.com/wiki/Arduino_Library_Manager_Tutorial)
- ESP32-S3-Touch-LCD-7B library file is stored in the demo, click here to jump: [ESP32-S3-Touch-LCD-7B Demo](#Resources)

*\- ESP32\-S3\-Touch\-LCD\-7 Library file installation instructions*

| Library Name | Description             | Version | Library Installation Requirement |
| ------------ | ----------------------- | ------- | -------------------------------- |
| lvgl         | LVGL graphical library  | v8.4.0  | "Install Offline"                |
| lv\_conf.h   | LVGL configuration file | ——      | "Install Offline"                |

## Run the First Arduino Demo

If you are just getting started with ESP32 and Arduino, and you don't know how to create, compile, flash, and run Arduino ESP32 programs, then please expand and take a look. Hope it can help you!

### New Project

- Run the Arduino IDE and select `File` -> `New Sketch`  
[![ESP32-S3-AMOLED-1.91-Ar-study-01.png](/w/upload/c/c6/ESP32-S3-AMOLED-1.91-Ar-study-01.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-01.png)
- Enter the code:

```c
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello, World!");
  delay(2000);
}
```

- Save the project and select `File` -> `Save As...`. In the pop-up menu, select the path to save the project, and enter a project name, such as **Hello_World**, click `Save`

```
[![ESP32-S3-AMOLED-1.91-Ar-study-02.png](/w/upload/4/4c/ESP32-S3-AMOLED-1.91-Ar-study-02.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-02.png)
```

### Compile and Flash Demos

- Select the corresponding development board, take the ESP32S3 motherboard as an example:  

①. Click to select the dropdown menu option **Select Other Board and Port**;  
②. Search for the required development board model **esp32s3 dev module** and select;  
③. Select **COM Port**;  
④. **Save** the selection.  
[![ESP32-S3-AMOLED-1.91-Ar-study-03.png](/w/upload/d/d9/ESP32-S3-AMOLED-1.91-Ar-study-03.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-03.png)

- If the ESP32S3 mainboard only has a USB port, you need to enable **USB CDC**, as shown in the following diagram:  

[![ESP32-S3-AMOLED-1.91-Ar-study-04.png](/w/upload/3/33/ESP32-S3-AMOLED-1.91-Ar-study-04.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-04.png)

- Compile and upload the program:  

①. Compile the program; ②. Compile and download the program; ③. Download successful.  
[![ESP32-S3-AMOLED-1.91-Ar-study-05.png](/w/upload/d/dc/ESP32-S3-AMOLED-1.91-Ar-study-05.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-05.png)

- Open the **Serial Monitor** window, and the demo will print "Hello World!" every 2 seconds, and the operation is as follows:  

[![ESP32-S3-AMOLED-1.91-Ar-study-06.png](/w/upload/6/66/ESP32-S3-AMOLED-1.91-Ar-study-06.png)](/wiki/File:ESP32-S3-AMOLED-1.91-Ar-study-06.png)

## Demo

*\- ESP32\-S3\-Touch\-LCD\-7B Demo*

| Demo                 | Basic Description                                                                              | Dependency Library |
| -------------------- | ---------------------------------------------------------------------------------------------- | ------------------ |
| 01\_GPIO             | Test GPIO                                                                                      | -                  |
| 02\_UART             | Test UART                                                                                      | -                  |
| 03\_I2C              | Test I2C                                                                                       | -                  |
| 04\_CAN              | Test CAN                                                                                       | -                  |
| 05\_RS485            | Test I2C                                                                                       | -                  |
| 06\_LCD              | Test display                                                                                   | -                  |
| 07\_SD               | Test TF card                                                                                   | -                  |
| 08\_TOUCH            | Test touchscreen                                                                               | -                  |
| 09\_DISPLAY\_BMP     | Display BMP images from the TF card on the screen                                              | -                  |
| 11\_WIFI\_STA        | Connect to the AP and display the IP                                                           | -                  |
| 12\_WIFI\_AP         | Start AP and display the MAC address of connected devices                                      | -                  |
| 13\_LVGL\_TRANSPLANT | Transplant LVGL                                                                                | LVGL               |
| 14\_LVGL\_BTN        | Draw a button to control GPIO operation                                                        | LVGL               |
| 15\_LVGL\_SLIDER     | Draw a slider to control backlight and GPIO output, and simultaneously display battery voltage | LVGL               |

- **Demos 08, 09, 13, 14, and 15 can only be applied to boards with touch screens.**

- **Select the Waveshare ESP32S3 XIP model and port**  

[![ESP32-S3-Touch-LCD-2.8C example 1.png](/w/upload/9/9b/ESP32-S3-Touch-LCD-2.8C_example_1.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C_example_1.png)

### 01_GPIO

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes GPIO 6 and sets it to output mode.  

- **`loop()`**:

`loop()` function is the main loop of the program, with the core functionality of repeatedly pulling the GPIO pin high or low to control the LED's on and off.  

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- The screen shows nothing, and the connected LED lights will turn on and off at a frequency of 1 Hz  

### 02_UART

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup ` function is primarily used for initializing serial communication  
Use the` UART.begin `function to initialize serial port` Serial `, set the baud rate, data format, and specify the receive and transmit pins. Then, through a loop, ensure the serial port initialization is successful.

- **`loop()`** :

` loop `function is the main loop part of the program, and its main function is to implement simple UART communication data return  
By checking whether there is data available at the serial port, if there is data, it reads a byte and sends it back immediately, so that the received UART data can be sent back intact

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B device, and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-45.png](/w/upload/e/e2/ESP32-Arduino-45.png)](/wiki/File:ESP32-Arduino-45.png)

### 03_I2C

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes I2C and IO expansion.  

- **`DEV_I2C_Init()`** :

`DEV_I2C_Init()` function initializes the I2C device  

- **`IO_EXTENSION_Init()`** :

`IO_EXTENSION_Init()` function initializes IO extension.  

- **`loop()`**:

`loop()` function is the main loop of the program, with the core functionality of controlling the IO expansion chip to repeatedly pull the DISP pin high or low, thereby controlling the screen backlight on and off.  

- **`IO_EXTENSION_Output(uint8_t pin, uint8_t value)`** :

`IO_EXTENSION_Output(uint8_t pin, uint8_t value)` function controls the IO expansion chip to output a specified level.  

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- The screen backlight will turn on and off at a frequency of 1Hz  

### 04_CAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB-CAN-A](https://www.waveshare.com/USB-CAN-A.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-04.png](/w/upload/5/5f/ESP32-S3-Touch-LCD-7B-Arduino-04.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-04.png)

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes I2C, IO expansion, and CAN interface. Before initializing the CAN interface, it needs to control IO_EXTENSION_IO_5 to a high level; otherwise, the CAN interface will not work.  

- **`can_init(twai_timing_config_t t_config, twai_filter_config_t f_config, twai_general_config_t g_config)`** :
- ` can_init` function is mainly responsible for handling the initialization of TWAI (an interface similar to the CAN bus)  

- **`loop()`**:

`loop()` function checks if any alarms occur. Read the triggered alerts by calling` can_read_alerts `and obtain the TWAI status information into` alerts_triggered `. Then, according to the different alarms triggered, perform corresponding processing. For example, if an error passive alarm, a bus error alarm, a transmission failure alarm, or a transmission success alarm is triggered, print the corresponding message and output some status information such as the bus error count, the number of messages to be sent, the transmission error counter, and the transmission failure counter.

- * If the correct data is read, it will read the information and then send the data back unchanged.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- The screen will not display
- After configuring [USB-CAN-A_TOOL](#Resources), start and send a CAN message to ESP32-S3-Touch-LCD-7B. You can see that ESP32-S3-Touch-LCD-7B returns the same message.

[![ESP32-S3-Touch-LCD-7B-Arduino-01.png](/w/upload/e/e0/ESP32-S3-Touch-LCD-7B-Arduino-01.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-01.png) [![ESP32-S3-Touch-LCD-7B-Arduino-02.png](/w/upload/b/b8/ESP32-S3-Touch-LCD-7B-Arduino-02.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-02.png)

### 05_RS485

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB to RS485 converter](https://www.waveshare.com/USB-TO-RS485-B.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-05.png](/w/upload/b/b4/ESP32-S3-Touch-LCD-7B-Arduino-05.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-05.png)

**Code analysis**

---

- **`setup()`**:

` setup ` function is primarily used for initializing serial communication  
Use the` RS485.begin `function to initialize serial port` Serial1`, set the baud rate, data format, and specify the receive and transmit pins. Then, through a loop, ensure the serial port initialization is successful.

- **`loop()`** :

` loop `function is the main loop part of the program, and its main function is to implement simple 485 communication data return  
By checking whether there is data available at the serial port, if there is data, it reads a byte and sends it back immediately, so that the received 485 data can be sent back intact

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B device (the message must contain a newline, otherwise no data will be returned), and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-32.png](/w/upload/b/bd/ESP32-Arduino-32.png)](/wiki/File:ESP32-Arduino-32.png)

### 06_LCD

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:
- ` setup `function mainly performs a series of initialization operations and tests on the display
  * First, it initializes I2C, IO expansion, and the RGB LCD, and then turns on the backlight. Then create a buffer to store the image, initialize the GUI image data, set the current image format and rotation angle, and finally start controlling the screen to display the corresponding interface.  
  * During the test, you can set the rotation angle yourself (in line 13) to realize the display of different angles.  

- **`waveshare_esp32_s3_rgb_lcd_init()`** :
- ` waveshare_esp32_s3_rgb_lcd_init` function mainly initializes the RGB LCD.

- **`wavesahre_rgb_lcd_bl_on()`** :
- ` wavesahre_rgb_lcd_bl_on` function mainly turns on the LCD backlight.
  * For more functions, they are also explained in the example, which can be viewed directly in the example.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- The ESP32-S3-Touch-LCD-7B will first display the RGB565 color box, then show the pattern and text, and finally display the image.

[https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4](https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-01.mp4>

### 07_SD

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the TF card to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and tests on the TF card, and displays relevant information on the screen.

- **`sd_mmc_init()`** :

` sd_mmc_init `function mainly initializes and mounts the TF card.

- **`read_sd_capacity(uint64_t *total_capacity, uint64_t *available_capacity)`** :

` read_sd_capacity `function primarily reads the memory information of the TF card.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Running Effect: ESP32-S3-Touch-LCD-7B will display TF card information, if not, it will display "SD Card Fail!”

[https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4](https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-02.mp4>

### 08_TOUCH

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and tests on the touch screen, and displays it on the screen.

- **`touch_gt911_init()`** :

` touch_gt911_init` function mainly initializes the touchscreen.

```
touch_gt911_read_point(uint8_t max_touch_cnt) :
```

` touch_gt911_read_point` function mainly reads the current coordinate point and can recognize up to 5 touch points.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- When touching the screen, points will be displayed on the screen, and touching five points at the same time will display five different colored points.

[https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4](https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-03.mp4>

### 09_DISPLAY_BMP

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Save the images in the directory to the TF card and connect it to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and reads image files, and switches display by clicking the arrow on the screen.

- **`list_files(const char *base_path)`** :

` list_files` function mainly reads BMP image file names and saves the names to BmpPath.

- **`GUI_ReadBmp(UWORD Xstart, UWORD Ystart, const char *path)`** :

` GUI_ReadBmp `function primarily reads BMP image data from the TF card and writes the data into the buffer, with the position determined by Xstart and Ystart.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- If the mounting is successful, an arrow will be displayed. Click the arrow to show the image, and clicking it again will switch the image left or right.

[https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4](https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-04.mp4>

### 10_WIFI_SCAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and scans nearby Wi-Fi, and displays the names of the scanned Wi-Fi networks on the screen (currently, it cannot display Chinese characters because the font library is not comprehensive enough).

- **`wifi_scan()`** :

` wifi_scan `function primarily scans for nearby Wi-Fi and writes the data to a cache buffer.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Start scanning, the current WiFi name will be displayed after scanning ends.

[https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4](https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-05.mp4>

### 11_WIFI_STA

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and connects to the specified WIFI (2.4GHz), and displays the IP address of the WIFI on the screen.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Connect to the set WIFI, and after the connection is successful, print the IP address on the screen

[https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4](https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-06.mp4>

### 12_WIFI_AP

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and creates a hotspot named ESP32-S3-Touch-LCD-7B, and displays the MAC address of the currently connected device on the screen.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Start the hotspot and display the MAC address of the connected device

[https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4](https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-07.mp4>

### 13_LVGL_TRANSPLANT

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

code> setup function mainly performs a series of initialization operations and runs the LVGL Demo.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- It will run lv_demo_widgets, allowing for interactive operations.

[https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4](https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-08.mp4>

**Other instructions**

---

- Screen drifting occurs during use, please refer to [ESP official FAQ](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen)
- When using your own UI program, there is a lack of memory, you can click Tools to select a larger partition table  
[![ESP32-Arduino-28.png](/w/upload/thumb/c/cb/ESP32-Arduino-28.png/800px-ESP32-Arduino-28.png)](/wiki/File:ESP32-Arduino-28.png)
- The version of lvgl used is 8.4, and you can query and use the LVGL API through the following documentation
  * [LVGL documentation](https://docs.lvgl.io/8.4/)

### 14_LVGL_BTN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and creates a button. Clicking the button can control the LED to turn on or off.

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Display a button, click the button to change the state of the GPIO to control the LED on and off

[https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4](https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-09.mp4>

### 15_LVGL_SLIDER

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and creates a slider and text (displaying battery voltage).

**Demo flashing**

---

- Select the Waveshare ESP32S3 XIP model and port
- Set development board parameters  

[![ESP32-S3-Touch-LCD-2.8C-demo-01.png](/w/upload/e/ee/ESP32-S3-Touch-LCD-2.8C-demo-01.png)](/wiki/File:ESP32-S3-Touch-LCD-2.8C-demo-01.png)

- Flash the demo

**Result demonstration**

---

- Display a slider showing the current battery voltage. By controlling the slider, you can simultaneously adjust the brightness of the LED and the screen.

[https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4](https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-10.mp4>

# Working with ESP-IDF

This chapter introduces setting up the ESP-IDF environment setup, including the installation of Visual Studio and the Espressif IDF plugin, program compilation, downloading, and testing of demos, to assist users in mastering the development board and facilitating secondary development. [![ESP-IDF-flow-01.png](/w/upload/thumb/d/d7/ESP-IDF-flow-01.png/800px-ESP-IDF-flow-01.png)](/wiki/File:ESP-IDF-flow-01.png)

## Environment Setup

### Download and Install Visual Studio

- Open the download page of [VScode official website](https://code.visualstudio.com/download), choose the corresponding system and system bit to download  
[![ESP32-S3-AMOLED-1.91-VScode-01.png](/w/upload/9/96/ESP32-S3-AMOLED-1.91-VScode-01.png)](/wiki/File:ESP32-S3-AMOLED-1.91-VScode-01.png)
- After running the installation package, the rest can be installed by default, but here for the subsequent experience, it is recommended to check boxes 1, 2, and 3  
[![ESP32-S3-AMOLED-1.91-VScode-02.png](/w/upload/1/19/ESP32-S3-AMOLED-1.91-VScode-02.png)](/wiki/File:ESP32-S3-AMOLED-1.91-VScode-02.png)
  * After the first two items are enabled, you can open VSCode directly by right-clicking files or directories, which can improve the subsequent user experience.
  * After the third item is enabled, you can select VSCode directly when you choose how to open it

The environment setup is carried out on the Windows 10 system, Linux and Mac users can access [ESP-IDF environment setup](https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32s3/get-started/windows-setup.html) for reference

### Install Espressif IDF Plugin

- It is generally recommended to use **Install Online**. If online installation fails due to network factor, use **Install Offline**.
- For more information about how to install the Espressif IDF plugin, see [Install Espressif IDF Plugin](https://www.waveshare.com/wiki/Install_Espressif_IDF_Plugin_Tutorial)

## Run the First ESP-IDF Demo

If you are just getting started with ESP32 and ESP-IDF, and you don't know how to create, compile, flash, and run ESP-IDF ESP32 programs, then please expand and take a look. Hope it can help you!

### New Project

[![ESP32-S3-AMOLED-1.91-study-01.png](/w/upload/9/95/ESP32-S3-AMOLED-1.91-study-01.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-01.png)  

[![ESP32-S3-AMOLED-1.91-study-02.png](/w/upload/2/2d/ESP32-S3-AMOLED-1.91-study-02.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-02.png)

### Create Demo

- Using the shortcut F1, enter esp-idf:show examples projects  

[![ESP32-S3-AMOLED-1.91-study-03.png](/w/upload/f/fa/ESP32-S3-AMOLED-1.91-study-03.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-03.png)

- Select your current IDF version  

[![ESP32-S3-AMOLED-1.91-study-04.png](/w/upload/d/da/ESP32-S3-AMOLED-1.91-study-04.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-04.png)

- Take the Hello world demo as an example

①Select the corresponding demo  
②Its readme will state what chip the demo applies to (how to use the demo and the file structure are described below, omitted here)  
③Click to create the demo  
[![ESP32-S3-AMOLED-1.91-study-05.png](/w/upload/b/bc/ESP32-S3-AMOLED-1.91-study-05.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-05.png)

- Select the path to save the demo, and require that the demos cannot use folders with the same name  

[![ESP32-S3-AMOLED-1.91-study-06.png](/w/upload/0/00/ESP32-S3-AMOLED-1.91-study-06.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-06.png)

### Modify COM Port

- The corresponding COM ports are shown here, click to modify them  
- Please select the COM ports according to your device (You can view it from the device manager)  
- In case of a download failure, please press the Reset button for more than 1 second or enter download mode, and wait for the PC to recognize the device again before downloading once more  

[![ESP32-S3-AMOLED-1.91-study-07.png](/w/upload/0/0d/ESP32-S3-AMOLED-1.91-study-07.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-07.png)

### Modify Driver Object

- Select the object we need to drive, which is our main chip ESP32S3  

[![ESP32-S3-AMOLED-1.91-study-08.png](/w/upload/b/b2/ESP32-S3-AMOLED-1.91-study-08.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-08.png)

- Choose the openocd path, it doesn't affect us here, so let's just choose one  

[![ESP32-S3-AMOLED-1.91-study-09.png](/w/upload/1/18/ESP32-S3-AMOLED-1.91-study-09.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-09.png)

### Other Status Bar Functions

①.ESP-IDF Development Environment Version Manager, when our project requires differentiation of development environment versions, it can be managed by installing different versions of ESP-IDF. When the project uses a specific version, it can be switched to by utilizing it  
②.Device flashing COM port, select to flash the compiled program into the chip  
③.Select set-target chip model, select the corresponding chip model, for example, ESP32-P4-NANO needs to choose esp32p4 as the target chip  
④.menuconfig, click it to modify sdkconfig configuration file, please refer to [project configuration details](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/kconfig.html)  
⑤.fullclean button, when the project compilation error or other operations pollute the compiled content, you can clean up all the compiled content by clicking it  
⑥.Build project, when a project satisfies the build, click this button to compile  
⑦.Current download mode, the default is UART  
⑧.flash button, when a project build is completed, select the COM port of the corresponding development board, and click this button to flash the compiled firmware to the chip  
⑨.monitor enable flashing port monitoring, when a project passes through Build --> Flash, click this button to view the log of output from flashing port and debugging port, so as to observe whether the application works normally  
⑩.Debug  
⑪.Build Flash Monitor one-click button, which is used to continuously execute Build --> Flash --> Monitor, often referred to as "little flame"  

[![ESP32-S3-AMOLED-1.91-study-10.png](/w/upload/e/ed/ESP32-S3-AMOLED-1.91-study-10.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-10.png)

### Compile, Flash and Serial Port Monitor

- Click on the all-in-one button we described before to compile, flash and open the serial port monitor  

[![ESP32-S3-AMOLED-1.91-study-11.png](/w/upload/2/27/ESP32-S3-AMOLED-1.91-study-11.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-11.png)

- It may take a long time to compile especially for the first time  

[![ESP32-S3-AMOLED-1.91-study-12.png](/w/upload/7/73/ESP32-S3-AMOLED-1.91-study-12.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-12.png)

- During this process, the ESP-IDF may take up a lot of CPU resources, so it may cause the system to lag  
- If it is the first time to flash the program for a new project, you will need to select the download method, and select UART  

[![ESP32-S3-AMOLED-1.91-study-13.png](/w/upload/4/47/ESP32-S3-AMOLED-1.91-study-13.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-13.png)

- This can also be changed later in the Download methods section (click on it to pop up the options)  

[![ESP32-S3-AMOLED-1.91-study-14.png](/w/upload/7/71/ESP32-S3-AMOLED-1.91-study-14.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-14.png)

- As it comes with the onboard automatic download circuit, it can be downloaded automatically without manual operation
- After successful download, it will automatically enter the serial monitor, you can see the chip output the corresponding information and be prompted to restart after 10S  

[![ESP32-S3-AMOLED-1.91-study-15.png](/w/upload/2/29/ESP32-S3-AMOLED-1.91-study-15.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-15.png)

### Use the IDF Demos

The following takes **ESP32-S3-LCD-1.47-Demo** as an example to introduce the two opening methods of the project and the general steps of use, and the detailed explanation of the ESP-IDF project. If you use other projects, the operation steps can be applied similarly.

#### Open In the Software

- Open VScode software and select the folder to open the demo  

[![ESP32-S3-AMOLED-1.91-study-16.png](/w/upload/6/6b/ESP32-S3-AMOLED-1.91-study-16.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-16.png)

- Select the provided ESP-IDF example and click to select the file (located in the /Demo/ESP-IDF path under demo)  

[![ESP32-S3-AMOLED-1.91-study-17.png](/w/upload/d/dc/ESP32-S3-AMOLED-1.91-study-17.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-17.png)

#### Open from Outside the Software

- Select the project directory correctly and open the project, otherwise it will affect the compilation and flashing of subsequent programs  

[![ESP32-S3-AMOLED-1.91-study-18.png](/w/upload/7/7c/ESP32-S3-AMOLED-1.91-study-18.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-18.png)

- After connecting the device, select the COM port and model, click below to compile and flash to achieve program control  

[![ESP32-S3-AMOLED-1.91-study-19.png](/w/upload/9/9e/ESP32-S3-AMOLED-1.91-study-19.png)](/wiki/File:ESP32-S3-AMOLED-1.91-study-19.png)

#### ESP-IDF Project Details

- Component: The components in ESP-IDF are the basic modules for building applications, each component is usually a relatively independent code base or library, which can implement specific functions or services, and can be reused by applications or other components, similar to the definition of libraries in Python development.
  * Component reference: The import of libraries in the Python development environment only requires to "import library name or path", while ESP-IDF is based on the C language, and the importing of libraries is configured and defined through `CMakeLists.txt`.
  * The purpose of CmakeLists.txt: When compiling ESP-IDF, the build tool `CMake` first reads the content of the top-level `CMakeLists.txt` in the project directory to read the build rules and identify the content to be compiled. When the required components and demos are imported into the `CMakeLists.txt`, the compilation tool `CMake` will import each content that needs to be compiled according to the index. The compilation process is as follows:

[![ESP32-P4 VSCode ESP-IDF GettingStart 240906 02.png](/w/upload/f/f7/ESP32-P4_VSCode_ESP-IDF_GettingStart_240906_02.png)](/wiki/File:ESP32-P4_VSCode_ESP-IDF_GettingStart_240906_02.png)

## Demo

*\- ESP32\-S3\-Touch\-LCD\-7B Demo*

| Demo                 | Basic Description                                                                              |
| -------------------- | ---------------------------------------------------------------------------------------------- |
| 01\_GPIO             | Test GPIO                                                                                      |
| 02\_UART             | Test UART                                                                                      |
| 03\_I2C              | Test I2C                                                                                       |
| 04\_CAN              | Test CAN                                                                                       |
| 05\_RS485            | Test I2C                                                                                       |
| 06\_LCD              | Test display                                                                                   |
| 07\_SD               | Test TF card                                                                                   |
| 08\_TOUCH            | Test touchscreen                                                                               |
| 09\_DISPLAY\_BMP     | Display BMP images from the TF card on the screen                                              |
| 11\_WIFI\_STA        | Connect to the AP and display the IP                                                           |
| 12\_WIFI\_AP         | Start AP and display the MAC address of connected devices                                      |
| 13\_LVGL\_TRANSPLANT | Transplant LVGL                                                                                |
| 14\_LVGL\_BTN        | Draw a button to control GPIO operation                                                        |
| 15\_LVGL\_SLIDER     | Draw a slider to control backlight and GPIO output, and simultaneously display battery voltage |
| 16\_LVGL\_UI         | Comprehensive demo                                                                             |

- **Demos 08, 09, 13, 14, 15 and 16 can only be applied to boards with touch screens.**

### 01_GPIO

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`app_main()`**:
  * Firstly, initialize GPIO 6 and set it to output mode.  

- * Finally, the core function of the loop section is to pull the GPIO pin high or low to control the LED to turn on and off.  

**Result demonstration**

---

- After successful flashing, the screen shows nothing, and the connected LED lights will turn on and off at a frequency of 1 Hz  

### 02_UART

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * First of all, it is mainly used to initialize serial communication and allocate a temporary buffer for receiving data  

- * Finally, the loop implements a simple UART communication data backhaul  

By checking whether there is data available on the serial port, if there is data, it is read and immediately sent back, achieving the function of returning the received UART data intact.

**Result demonstration**

---

- After successful flashing, open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B, and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-45.png](/w/upload/e/e2/ESP32-Arduino-45.png)](/wiki/File:ESP32-Arduino-45.png)

### 03_I2C

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Firstly, initialize I2C and IO extensions.  

- **`DEV_I2C_Init()`** :

`DEV_I2C_Init()` function initializes the I2C device  

- **`IO_EXTENSION_Init()`** :

`IO_EXTENSION_Init()` function initializes IO extension.  

- * Finally, the loop control IO expansion chip to repeatedly pull the DISP pin high or low, thereby controlling the screen backlight on and off.  

- **`IO_EXTENSION_Output(uint8_t pin, uint8_t value)`** :

`IO_EXTENSION_Output(uint8_t pin, uint8_t value)` function controls the IO expansion chip to output a specified level.  

**Result demonstration**

---

- After successful flashing, the screen backlight will turn on and off at a frequency of 1Hz  

### 04_CAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB-CAN-A](https://www.waveshare.com/USB-CAN-A.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-04.png](/w/upload/5/5f/ESP32-S3-Touch-LCD-7B-Arduino-04.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-04.png)

**Code analysis**

---

- **`app_main()`**:
  * First initialize the I2C, IO expansion, and CAN interface. Before initializing the CAN interface, it needs to control IO_EXTENSION_IO_5 to a high level; otherwise, the CAN interface will not work.  

- **`can_init(twai_timing_config_t t_config, twai_filter_config_t f_config, twai_general_config_t g_config)`** :
- ` can_init` function is mainly responsible for handling the initialization of TWAI (an interface similar to the CAN bus)  

- * Finally, loop through to check if any alarms have occurred. Read the triggered alerts by calling` can_read_alerts `and obtain the TWAI status information into` alerts_triggered `. Then, according to the different alarms triggered, perform corresponding processing. For example, if an error passive alarm, a bus error alarm, a transmission failure alarm, or a transmission success alarm is triggered, print the corresponding message and output some status information such as the bus error count, the number of messages to be sent, the transmission error counter, and the transmission failure counter.
  * If the correct data is read, it will read the information and then send the data back unchanged.

**Result demonstration**

---

- After successful flashing, the screen will not display
- After configuring [USB-CAN-A_TOOL](#Resources), start and send a CAN message to ESP32-S3-Touch-LCD-7B. You can see that ESP32-S3-Touch-LCD-7B returns the same message.

[![ESP32-S3-Touch-LCD-7B-Arduino-01.png](/w/upload/e/e0/ESP32-S3-Touch-LCD-7B-Arduino-01.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-01.png) [![ESP32-S3-Touch-LCD-7B-Arduino-02.png](/w/upload/b/b8/ESP32-S3-Touch-LCD-7B-Arduino-02.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-02.png)

### 05_RS485

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB to RS485 converter](https://www.waveshare.com/USB-TO-RS485-B.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-05.png](/w/upload/b/b4/ESP32-S3-Touch-LCD-7B-Arduino-05.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-05.png)

**Code analysis**

---

- **`app_main()`**:
  * First initialize serial communication  

Initialize the serial port using the `DEV_UART_Init` function, set the baud rate, configure the receive and transmit pins, and allocate a temporary buffer for receiving data.

- * Finally, the loop implements a simple RS485 communication data backhaul  

By checking whether there is data available on the serial port, if there is data, it is read and immediately sent back, achieving the function of returning the received RS485 data intact.

**Result demonstration**

---

- After successful flashing, open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B, and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-32.png](/w/upload/b/bd/ESP32-Arduino-32.png)](/wiki/File:ESP32-Arduino-32.png)

### 06_LCD

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly perform a series of initialization operations and tests on the display
  * First, it initializes I2C, IO expansion, and the RGB LCD, and then turns on the backlight. Then create a buffer to store the image, initialize the GUI image data, set the current image format and rotation angle, and finally start controlling the screen to display the corresponding interface.  
  * During the test, you can set the rotation angle yourself (in line 18) to realize the display of different angles.  

- **`waveshare_esp32_s3_rgb_lcd_init()`** :
- ` waveshare_esp32_s3_rgb_lcd_init` function mainly initializes the RGB LCD.

- **`wavesahre_rgb_lcd_bl_on()`** :
- ` wavesahre_rgb_lcd_bl_on` function mainly turns on the LCD backlight.
  * For more functions, they are also explained in the example, which can be viewed directly in the example.

**Result demonstration**

---

- After successful flashing, ESP32-S3-Touch-LCD-7B will first display the RGB565 color box, then show the pattern and text, and finally display the image.

[https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4](https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-01.mp4>

### 07_SD

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the TF card to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and tests on the TF card, and displays relevant information on the screen.

- **`sd_mmc_init()`** :

` sd_mmc_init `function mainly initializes and mounts the TF card.

- **`read_sd_capacity(uint64_t *total_capacity, uint64_t *available_capacity)`** :

` read_sd_capacity `function primarily reads the memory information of the TF card.

**Result demonstration**

---

- After successful flashing, ESP32-S3-Touch-LCD-7B will display TF card information, if not, it will display "SD Card Fail!"

[https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4](https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-02.mp4>

### 08_TOUCH

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and tests on the touch screen, and displays relevant information on the screen.

- **`touch_gt911_init()`** :

` touch_gt911_init` function mainly initializes the touchscreen.

**`touch_gt911_read_point(uint8_t max_touch_cnt)`** : ` touch_gt911_read_point` function mainly reads the current coordinate point and can recognize up to 5 touch points.

**Result demonstration**

---

- After successful flashing, points will be displayed on the screen when touching the screen, and touching five points at the same time will display five different colored points.

[https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4](https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-03.mp4>

### 09_DISPLAY_BMP

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Save the images in the directory to the TF card and connect it to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and reads image files, and switches display by clicking the arrow on the screen.

- **`list_files(const char *base_path)`** :

` list_files` function mainly reads BMP image file names and saves the names to BmpPath.

- **`GUI_ReadBmp(UWORD Xstart, UWORD Ystart, const char *path)`** :

` GUI_ReadBmp `function primarily reads BMP image data from the TF card and writes the data into the buffer, with the position determined by Xstart and Ystart.

**Result demonstration**

---

- After successful flashing, if the mounting is successful, an arrow will be displayed. Click the arrow to show the image, and clicking it again will switch the image left or right.

[https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4](https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-04.mp4>

### 10_WIFI_SCAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and scans nearby Wi-Fi, and displays the names of the scanned Wi-Fi networks on the screen (currently, it cannot display Chinese characters because the font library is not comprehensive enough).

- **`wifi_scan()`** :

` wifi_scan `function primarily scans for nearby Wi-Fi and writes the data to a cache buffer.

**Result demonstration**

---

- After successful flashing, start scanning, the current WiFi name will be displayed after scanning ends.

[https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4](https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-05.mp4>

### 11_WIFI_STA

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and connects to the specified WIFI (2.4GHz), and displays the IP address of the WIFI on the screen.

**Result demonstration**

---

- After successful flashing, connect to the set WIFI, and after the connection is successful, print the IP address on the screen

[https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4](https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-06.mp4>

### 12_WIFI_AP

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and creates a hotspot named ESP32-S3-Touch-LCD-7B, and displays the MAC address of the currently connected device on the screen.

**Result demonstration**

---

- After successful flashing, start the hotspot and display the MAC address of the connected device

[https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4](https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-07.mp4>

### 13_LVGL_TRANSPLANT

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and runs the LVGL Demo.

**Result demonstration**

---

- After successful flashing, it will run lv_demo_widgets, allowing for interactive operations.

[https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4](https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-08.mp4>

**Other instructions**

---

- Screen drifting occurs during use, please refer to [ESP official FAQ](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen)
- When using your own UI program, there is a lack of memory, you can click Tools to select a larger partition table  
[![ESP32-Arduino-28.png](/w/upload/thumb/c/cb/ESP32-Arduino-28.png/800px-ESP32-Arduino-28.png)](/wiki/File:ESP32-Arduino-28.png)
- The version of lvgl used is 8.4, and you can query and use the LVGL API through the following documentation
  * [LVGL documentation](https://docs.lvgl.io/8.4/)

### 14_LVGL_BTN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and creates a button. Clicking the button can control the LED to turn on or off.

**Result demonstration**

---

- After successful flashing, a button will be displayed. Click the button to change the state of the GPIO to control the LED on and off

[https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4](https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-09.mp4>

### 15_LVGL_SLIDER

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and creates a slider and text (displaying battery voltage).

**Result demonstration**

---

- After successful flashing, a slider showing the current battery voltage will be displayed. By controlling the slider, you can simultaneously adjust the brightness of the LED and the screen.

[https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4](https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-10.mp4>

### 16_LVGL_UI

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`app_main()`**:
  * Mainly performs a series of initialization operations and creates a custom GUI interface. The GUI is designed by SquareLine Studio, since it is only used to draw graphics, it does not provide SquareLine Studio projects.

**Result demonstration**

---

- After the flashing is successful, you can perform the following steps:
  * The first time you run it, you need to create a login and password
  * After the creation is completed, click the button to return to the login interface, and enter the username and password to log in
  * After logging in, you can see four boxes, corresponding to different functions
  * **`WIFI`**: It can test Scan, STA, and AP, and display related WiFi information
  * **`RS485`**: It can test the RS485 function, set the baud rate, send and receive data
  * **`CAN`**: It can test CAN functions, set the rate, and send and receive data
  * **`PWM`**: It can test backlight, battery voltage reading, and TF card

[https://www.waveshare.com/w/upload/e/e3/ESP32-S3-Touch-LCD-7B-12.mp4](https://www.waveshare.com/w/upload/e/e3/ESP32-S3-Touch-LCD-7B-12.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-12.mp4>

# PlatformIO Development

This chapter introduces setting up the platformio environment, including the program compilation and downloading, as well as testing demos. It aims to help users master the development board and facilitate secondary development

## Environment Setup

### Download and Install Visual Studio

- Open the download page of [VScode official website](https://code.visualstudio.com/download), choose the corresponding system and system bit to download  
[![ESP32-S3-AMOLED-1.91-VScode-01.png](/w/upload/9/96/ESP32-S3-AMOLED-1.91-VScode-01.png)](/wiki/File:ESP32-S3-AMOLED-1.91-VScode-01.png)
- After running the installation package, the rest can be installed by default, but here for the subsequent experience, it is recommended to check boxes 1, 2, and 3  
[![ESP32-S3-AMOLED-1.91-VScode-02.png](/w/upload/1/19/ESP32-S3-AMOLED-1.91-VScode-02.png)](/wiki/File:ESP32-S3-AMOLED-1.91-VScode-02.png)
  * After the first two items are enabled, you can open VSCode directly by right-clicking files or directories, which can improve the subsequent user experience.
  * After the third item is enabled, you can select VSCode directly when you choose how to open it

### Install PlatformIO IDE Plugin

- Open VSCode, click on Extensions on the left, search for and install PlatformIO IDE in the extensions.

[![ESP32-S3-Touch-LCD-7B-PlatformIO-01.png](/w/upload/8/88/ESP32-S3-Touch-LCD-7B-PlatformIO-01.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-01.png)

## Run the First PlatformIO Demo

If you are just getting started with ESP32 and PlatformIO, and you don't know how to create, compile, flash, and run PlatformIO ESP32 demos, then please expand and take a look. Hope it can help you!

### New Project

- Open VS Code, if the PlatformIO IDE plugin is already installed, click on the PlatformIO icon, select `Create New Project` -> `New Projject` -> Set the project name and board model, such as **Hello_World**  

```
[![ESP32-S3-Touch-LCD-7B-PlatformIO-02.png](/w/upload/6/6f/ESP32-S3-Touch-LCD-7B-PlatformIO-02.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-02.png)
[![ESP32-S3-Touch-LCD-7B-PlatformIO-03.png](/w/upload/e/ea/ESP32-S3-Touch-LCD-7B-PlatformIO-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-03.png)
```

- The first run requires waiting for PlatformIO to download the relevant packages, which may take a long time, so please be patient

```
[![ESP32-S3-Touch-LCD-7B-PlatformIO-04.png](/w/upload/d/de/ESP32-S3-Touch-LCD-7B-PlatformIO-04.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-04.png)
```

- Enter the code:

```c
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello, World!");
  delay(2000);
}
```

- Save the code file, connect the development board to the computer, choose `Port` -> `Compile and Flash`; When the fifth item is displayed, it indicates that the flashing is successful. Then click on the sixth item to observe the operation of the demo

```
[![ESP32-S3-Touch-LCD-7B-PlatformIO-05.png](/w/upload/8/8a/ESP32-S3-Touch-LCD-7B-PlatformIO-05.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-05.png)
```

### Compile and Flash Demos

- Compile and flash according to the following steps:

①. Click to open main.cpp file;  
②. Enter the code given above and save it;  
③. Select **COM Port**;  
④. **Compile and flash the demo**.  
⑤. If **SUCCESS** appears, it indicates successful flashing.  
⑥. Click on serial port assistant, and repeatedly output **Hello, World!**, then it is successful.  

```
[![ESP32-S3-Touch-LCD-7B-PlatformIO-05.png](/w/upload/8/8a/ESP32-S3-Touch-LCD-7B-PlatformIO-05.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-05.png)
[![ESP32-S3-Touch-LCD-7B-PlatformIO-06.png](/w/upload/3/33/ESP32-S3-Touch-LCD-7B-PlatformIO-06.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-PlatformIO-06.png)
```

## Demo

*\- ESP32\-S3\-Touch\-LCD\-7B Demo*

| Demo                 | Basic Description                                                                              | Dependency Library |
| -------------------- | ---------------------------------------------------------------------------------------------- | ------------------ |
| 01\_GPIO             | Test GPIO                                                                                      | -                  |
| 02\_UART             | Test UART                                                                                      | -                  |
| 03\_I2C              | Test I2C                                                                                       | -                  |
| 04\_CAN              | Test CAN                                                                                       | -                  |
| 05\_RS485            | Test I2C                                                                                       | -                  |
| 06\_LCD              | Test display                                                                                   | -                  |
| 07\_SD               | Test TF card                                                                                   | -                  |
| 08\_TOUCH            | Test touchscreen                                                                               | -                  |
| 09\_DISPLAY\_BMP     | Display BMP images from the TF card on the screen                                              | -                  |
| 11\_WIFI\_STA        | Connect to the AP and display the IP                                                           | -                  |
| 12\_WIFI\_AP         | Start AP and display the MAC address of connected devices                                      | -                  |
| 13\_LVGL\_TRANSPLANT | Transplant LVGL                                                                                | LVGL               |
| 14\_LVGL\_BTN        | Draw a button to control GPIO operation                                                        | LVGL               |
| 15\_LVGL\_SLIDER     | Draw a slider to control backlight and GPIO output, and simultaneously display battery voltage | LVGL               |

- **Demos 08, 09, 13, 14, and 15 can only be applied to boards with touch screens.**

### 01_GPIO

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes GPIO 6 and sets it to output mode.  

- **`loop()`**:

`loop()` function is the main loop of the program, with the core functionality of repeatedly pulling the GPIO pin high or low to control the LED's on and off.  

**Result demonstration**

---

- After successful flashing, the screen shows nothing, and the connected LED lights will turn on and off at a frequency of 1 Hz  

### 02_UART

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup ` function is primarily used for initializing serial communication  
Use the` UART.begin `function to initialize serial port` Serial `, set the baud rate, data format, and specify the receive and transmit pins. Then, through a loop, ensure the serial port initialization is successful.

- **`loop()`** :

` loop `function is the main loop part of the program, and its main function is to implement simple UART communication data return  
By checking whether there is data available at the serial port, if there is data, it reads a byte and sends it back immediately, so that the received UART data can be sent back intact

**Result demonstration**

---

- After successful flashing, open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B, and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-45.png](/w/upload/e/e2/ESP32-Arduino-45.png)](/wiki/File:ESP32-Arduino-45.png)

### 03_I2C

**Hardware connection**

---

- Connect the UART port of the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes I2C and IO expansion.  

- **`DEV_I2C_Init()`** :

`DEV_I2C_Init()` function initializes the I2C device  

- **`IO_EXTENSION_Init()`** :

`IO_EXTENSION_Init()` function initializes IO extension.  

- **`loop()`**:

`loop()` function is the main loop of the program, with the core functionality of controlling the IO expansion chip to repeatedly pull the DISP pin high or low, thereby controlling the screen backlight on and off.  

- **`IO_EXTENSION_Output(uint8_t pin, uint8_t value)`** :

`IO_EXTENSION_Output(uint8_t pin, uint8_t value)` function controls the IO expansion chip to output a specified level.  

**Result demonstration**

---

- After successful flashing, the screen backlight will turn on and off at a frequency of 1Hz  

### 04_CAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB-CAN-A](https://www.waveshare.com/USB-CAN-A.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-04.png](/w/upload/5/5f/ESP32-S3-Touch-LCD-7B-Arduino-04.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-04.png)

**Code analysis**

---

- **`setup()`**:

`setup()` function initializes I2C, IO expansion, and CAN interface. Before initializing the CAN interface, it needs to control IO_EXTENSION_IO_5 to a high level; otherwise, the CAN interface will not work.  

- **`can_init(twai_timing_config_t t_config, twai_filter_config_t f_config, twai_general_config_t g_config)`** :
- ` can_init` function is mainly responsible for handling the initialization of TWAI (an interface similar to the CAN bus)  

- **`loop()`**:

`loop()` function checks if any alarms occur. Read the triggered alerts by calling` can_read_alerts `and obtain the TWAI status information into` alerts_triggered `. Then, according to the different alarms triggered, perform corresponding processing. For example, if an error passive alarm, a bus error alarm, a transmission failure alarm, or a transmission success alarm is triggered, print the corresponding message and output some status information such as the bus error count, the number of messages to be sent, the transmission error counter, and the transmission failure counter.

- * If the correct data is read, it will read the information and then send the data back unchanged.

**Result demonstration**

---

- After successful flashing, the screen will not display
- After configuring [USB-CAN-A_TOOL](#Resources), start and send a CAN message to ESP32-S3-Touch-LCD-7B. You can see that ESP32-S3-Touch-LCD-7B returns the same message.

[![ESP32-S3-Touch-LCD-7B-Arduino-01.png](/w/upload/e/e0/ESP32-S3-Touch-LCD-7B-Arduino-01.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-01.png) [![ESP32-S3-Touch-LCD-7B-Arduino-02.png](/w/upload/b/b8/ESP32-S3-Touch-LCD-7B-Arduino-02.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-02.png)

### 05_RS485

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the development board to [USB to RS485 converter](https://www.waveshare.com/USB-TO-RS485-B.htm), as shown in the figure

[![ESP32-S3-Touch-LCD-7B-Arduino-05.png](/w/upload/b/b4/ESP32-S3-Touch-LCD-7B-Arduino-05.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-05.png)

**Code analysis**

---

- **`setup()`**:

` setup ` function is primarily used for initializing serial communication  
Use the` RS485.begin `function to initialize serial port` Serial1`, set the baud rate, data format, and specify the receive and transmit pins. Then, through a loop, ensure the serial port initialization is successful.

- **`loop()`** :

` loop `function is the main loop part of the program, and its main function is to implement simple 485 communication data return  
By checking whether there is data available at the serial port, if there is data, it reads a byte and sends it back immediately, so that the received 485 data can be sent back intact

**Result demonstration**

---

- After successful flashing, open the serial port debugging assistant to send a message to the ESP32-S3-Touch-LCD-7B device (the message must contain a newline, otherwise no data will be returned), and the device will return the received message to the serial port debugging assistant

[![ESP32-Arduino-32.png](/w/upload/b/bd/ESP32-Arduino-32.png)](/wiki/File:ESP32-Arduino-32.png)

### 06_LCD

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:
- ` setup `function mainly performs a series of initialization operations and tests on the display
  * First, it initializes I2C, IO expansion, and the RGB LCD, and then turns on the backlight. Then create a buffer to store the image, initialize the GUI image data, set the current image format and rotation angle, and finally start controlling the screen to display the corresponding interface.  
  * During the test, you can set the rotation angle yourself (in line 13) to realize the display of different angles.  

- **`waveshare_esp32_s3_rgb_lcd_init()`** :
- ` waveshare_esp32_s3_rgb_lcd_init` function mainly initializes the RGB LCD.

- **`wavesahre_rgb_lcd_bl_on()`** :
- ` wavesahre_rgb_lcd_bl_on` function mainly turns on the LCD backlight.
  * For more functions, they are also explained in the example, which can be viewed directly in the example.

**Result demonstration**

---

- After successful flashing, ESP32-S3-Touch-LCD-7B will first display the RGB565 color box, then show the pattern and text, and finally display the image.

[https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4](https://www.waveshare.com/w/upload/8/85/ESP32-S3-Touch-LCD-7B-01.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-01.mp4>

### 07_SD

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the TF card to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and tests on the TF card, and displays relevant information on the screen.

- **`sd_mmc_init()`** :

` sd_mmc_init `function mainly initializes and mounts the TF card.

- **`read_sd_capacity(uint64_t *total_capacity, uint64_t *available_capacity)`** :

` read_sd_capacity `function primarily reads the memory information of the TF card.

**Result demonstration**

---

- After successful flashing, the running effect: ESP32-S3-Touch-LCD-7B will display TF card information, if not, it will display "SD Card Fail!"

[https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4](https://www.waveshare.com/w/upload/4/4d/ESP32-S3-Touch-LCD-7B-02.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-02.mp4>

### 08_TOUCH

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and tests on the touch screen, and displays it on the screen.

- **`touch_gt911_init()`** :

` touch_gt911_init` function mainly initializes the touchscreen.

**`touch_gt911_read_point(uint8_t max_touch_cnt)`** : ` touch_gt911_read_point` function mainly reads the current coordinate point and can recognize up to 5 touch points.

**Result demonstration**

---

- After successful flashing, points will be displayed on the screen when touching the screen, and touching five points at the same time will display five different colored points.

[https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4](https://www.waveshare.com/w/upload/7/79/ESP32-S3-Touch-LCD-7B-03.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-03.mp4>

### 09_DISPLAY_BMP

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Save the images in the directory to the TF card and connect it to ESP32-S3-Touch-LCD-7B

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and reads image files, and switches display by clicking the arrow on the screen.

- **`list_files(const char *base_path)`** :

` list_files` function mainly reads BMP image file names and saves the names to BmpPath.

- **`GUI_ReadBmp(UWORD Xstart, UWORD Ystart, const char *path)`** :

` GUI_ReadBmp `function primarily reads BMP image data from the TF card and writes the data into the buffer, with the position determined by Xstart and Ystart.

**Result demonstration**

---

- After successful flashing, if the mounting is successful, an arrow will be displayed. Click the arrow to show the image, and clicking it again will switch the image left or right.

[https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4](https://www.waveshare.com/w/upload/f/f0/ESP32-S3-Touch-LCD-7B-04.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-04.mp4>

### 10_WIFI_SCAN

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and scans nearby Wi-Fi, and displays the names of the scanned Wi-Fi networks on the screen (currently, it cannot display Chinese characters because the font library is not comprehensive enough).

- **`wifi_scan()`** :

` wifi_scan `function primarily scans for nearby Wi-Fi and writes the data to a cache buffer.

**Result demonstration**

---

- After successful flashing, start scanning, the current WiFi name will be displayed after scanning ends.

[https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4](https://www.waveshare.com/w/upload/e/e4/ESP32-S3-Touch-LCD-7B-05.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-05.mp4>

### 11_WIFI_STA

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and connects to the specified WIFI (2.4GHz), and displays the IP address of the WIFI on the screen.

**Result demonstration**

---

- After successful flashing, connect to the set WIFI, and after the connection is successful, print the IP address on the screen

[https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4](https://www.waveshare.com/w/upload/1/1a/ESP32-S3-Touch-LCD-7B-06.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-06.mp4>

### 12_WIFI_AP

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

` setup `function mainly performs a series of initialization operations and creates a hotspot named ESP32-S3-Touch-LCD-7B, and displays the MAC address of the currently connected device on the screen.

**Result demonstration**

---

- After successful flashing, start the hotspot and display the MAC address of the connected device

[https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4](https://www.waveshare.com/w/upload/b/b3/ESP32-S3-Touch-LCD-7B-07.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-07.mp4>

### 13_LVGL_TRANSPLANT

**Hardware connection**

---

- Connect the board to the computer using a USB cable

**Code analysis**

---

- **`setup()`**:

code> setup function mainly performs a series of initialization operations and runs the LVGL Demo.

**Result demonstration**

---

- After successful flashing, it will run lv_demo_widgets, allowing for interactive operations.

[https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4](https://www.waveshare.com/w/upload/2/2f/ESP32-S3-Touch-LCD-7B-08.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-08.mp4>

**Other instructions**

---

- Screen drifting occurs during use, please refer to [ESP official FAQ](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen)
- When using your own UI program, there is a lack of memory, you can click Tools to select a larger partition table  
[![ESP32-Arduino-28.png](/w/upload/thumb/c/cb/ESP32-Arduino-28.png/800px-ESP32-Arduino-28.png)](/wiki/File:ESP32-Arduino-28.png)
- The version of lvgl used is 8.4, and you can query and use the LVGL API through the following documentation
  * [LVGL documentation](https://docs.lvgl.io/8.4/)

### 14_LVGL_BTN

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and creates a button. Clicking the button can control the LED to turn on or off.

**Result demonstration**

---

- After successful flashing, a button will be displayed. Click the button to change the state of the GPIO to control the LED on and off

[https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4](https://www.waveshare.com/w/upload/a/a4/ESP32-S3-Touch-LCD-7B-09.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-09.mp4>

### 15_LVGL_SLIDER

**Hardware connection**

---

- Connect the board to the computer using a USB cable
- Connect the LED module to the GPIO

[![ESP32-S3-Touch-LCD-7B-Arduino-03.png](/w/upload/0/04/ESP32-S3-Touch-LCD-7B-Arduino-03.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-03.png)

**Code analysis**

---

- **`setup()`**:

`setup` function mainly performs a series of initialization operations and creates a slider and text (displaying battery voltage).

**Result demonstration**

---

- After successful flashing, a slider showing the current battery voltage will be displayed. By controlling the slider, you can simultaneously adjust the brightness of the LED and the screen.

[https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4](https://www.waveshare.com/w/upload/5/5e/ESP32-S3-Touch-LCD-7B-10.mp4)<https://www.waveshare.com/wiki/File:ESP32-S3-Touch-LCD-7B-10.mp4>

# Resources

## Schematic Diagram

- [ESP32-S3-Touch-LCD-7B Schematic diagram](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-7B/ESP32-S3-Touch-LCD-7B-Schematic.pdf)

## 3D Diagram

- [ESP32-S3-Touch-LCD-7B 3D diagram](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-7B/ESP32-S3-Touch-LCD-7B_Drawing.zip)

## Demo

- [ESP32-S3-Touch-LCD-7B Demo](https://drive.google.com/file/d/1SlaHaUGaepOzLuIErm9lD_saKXQ0TIRK/view)

## Datasheets

### ESP32-S3

- [ESP32-S3 Datasheet](https://files.waveshare.com/wiki/common/Esp32-s3_datasheet_en.pdf)
- [ESP32-S3 Technical reference manual](https://files.waveshare.com/wiki/common/Esp32-s3_technical_reference_manual_en.pdf)
- [ESP32-S3-WROOM-1 datasheet](https://files.waveshare.com/wiki/common/Esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)

### Other Components

- [ST7262 Datasheet](https://files.waveshare.com/wiki/common/ST7262.pdf)
- [GT911 Datasheet](https://files.waveshare.com/wiki/common/GT911_EN_Datasheet.pdf)
- [CH343 Datasheet](https://files.waveshare.com/wiki/common/CH343DS1-en.pdf)
- [TJA1051 Datasheet](https://files.waveshare.com/wiki/common/TJA1051.pdf)

## Software Tools

### Arduino

- [Arduino IDE Official download link](https://www.arduino.cc/en/software/)

### VScode

- [VScode official website](https://code.visualstudio.com/download)

### Debugging Tools

- [SSCOM Serial port and network debugging assistant](https://files.waveshare.com/wiki/common/Sscom5.13.1.zip)
- [USB-CAN-A_TOOL](https://files.waveshare.com/wiki/USB-CAN-A/Tool/USBCANV2.10.zip)

## Other Resource Links

- [ESP32-Arduino official documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/index.html)
- [ESP32-Arduino official resources](https://github.com/espressif/arduino-esp32)
- [ESP-IDF official resources](https://github.com/espressif/esp-idf)
- [LVGL official documentation](https://docs.lvgl.io/master/intro/introduction/index.html)

## FAQ

#### [Question: ESP32-S3-Touch-LCD-7B shows no response after uploading an Arduino demo for RGB screen displaying?](#accordion1)

 **Answer:**

If there's no screen response after programming the code, check whether the correct configurations are set in Arduino IDE -> Tools, select the corresponding Flash (16MB) and enable PSRAM (8MB OPI))

#### [Question: Why are all libraries installed, and the flashing LVGL program reports an error missing lv_cong.h?](#accordion2)

 **Answer:**

The path to install the library contains Chinese characters, resulting in the inability to find the library file

#### [Question: Why does error "fatal error:esp_ memory_ utils.h:No such file or directory" occur when compiling demos using the Arduino IDE?](#accordion3)

 **Answer:**

Please install the Arduino ESP32 that we provided to solve this issue

#### [Question: Can CAN communication use debugging tools to send continuous frames?](#accordion4)

 **Answer:**

Yes, you can also customize the frequency of consecutive frames. When the frequency is too high and the computer freezes, it may cause bus errors

#### [Question: What should I do if ESP-IDF flashing fails?](#accordion5)

 **Answer:**

1. You can set the board to download mode to solve this issue. Power off completely, press and hold the Boot button and power it on again, then release it, enter the download mode, re-flash the program, reset and run.
2. Please try to press the fullclean button in the status bar and recompile the flashing, this function is to clean up all the compiled content by clicking when the project compilation error or other operations pollute the compiled content

#### [Question: What should I do if I can't find the AppData folder?](#accordion6)

 **Answer:**

Some AppData folders are hidden by default and can be set to show

- English system: Explorer->View->Check "Hidden items"
- Chinese system: File Explorer -> View -> Display -> Check "Hidden Items"

#### [Question: How to check the COM port?](#accordion7)

 **Answer:**

Press the Windows + R keys to open the "Run" dialog box, input devmgmt.msc and press Enter to open the Device Manager;
Expand the "Port (COM and LPT)" section, here it will list all the COM ports and their current status.

#### [Question: How to deal with the first compilation of the program being extremely slow?](#accordion8)

 **Answer:**

It's normal for the first compilation to be slow, just be patient

#### [Question: How to solve the problem that the program is flashed successfully but there is no display on LCD?](#accordion9)

 **Answer:**

If there is a reset button on the development board, press the reset button; if there is no reset button, please power it on again

#### [Question: Why does the program flashing fail when using a MAC device?](#accordion10)

 **Answer:**

Install [MAC Driver](https://files.waveshare.com/wiki/common/CH34XSER_MAC.7z) and flash again.

#### [Question: How to use SquareLine Studio to design interfaces?](#accordion11)

 **Answer:**

- Please refer to [SquareLine Studio tutorial](https://www.waveshare.com/wiki/Waveshare_SquareLine_Studio)

#### [Question: How to port the provided lib libraries? Or how to develop my own LCD screen? How to drive?](#accordion12)

 **Answer:**

The LCD screen display chip used in this product is ST7701, and the touch chip is GT911, we recommend that you directly refer to the demos of Arduino or ESP-IDF

#### [Question: What should I do if WiFi reconnecting causes drifting?](#accordion13)

 **Answer:**

Click this link to view the solution: [LCD FAQ](https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/lcd.html#why-do-i-get-drift-overall-drift-of-the-display-when-esp32-s3-is-driving-an-rgb-lcd-screen)

#### [Question: How to use LVGL for boards without touch?](#accordion14)

 **Answer:**

In the main function, comment out the code shown in the red box.  
[![ESP32-S3-Touch-LCD-7B-Arduino-15.png](/w/upload/6/69/ESP32-S3-Touch-LCD-7B-Arduino-15.png)](/wiki/File:ESP32-S3-Touch-LCD-7B-Arduino-15.png)

# Support

Technical Support

If you need technical support or have any feedback/review, please click the **Submit Now** button to submit a ticket, Our support team will check and reply to you within 1 to 2 working days. Please be patient as we make every effort to help you to resolve the issue.  
Working Time: 9 AM - 6 PM GMT+8 (Monday to Friday)

[Submit Now](https://service.waveshare.com/)

Retrieved from "<https://www.waveshare.com/w/index.php?title=ESP32-S3-Touch-LCD-7B&oldid=106787>"

[Category](/wiki/Special:Categories "Special:Categories"):

- [Type-C USB interface](/w/index.php?title=Category:Type-C_USB_interface&action=edit&redlink=1 "Category:Type-C USB interface (page does not exist)")

- [Login / Create Account](/w/index.php?title=Special:UserLogin&returnto=ESP32-S3-Touch-LCD-7B "You are encouraged to log in; however, it is not mandatory [o]")