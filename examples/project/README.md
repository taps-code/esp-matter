# Multi-Endpoint Smart Light

This project is based on the esp-matter Light example in release/v1.2 and creates an Extended Color Light and Light Sensor device on a single node using the ESP
Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## 1. Additional Environment Setup

In order to successfully build the project, make sure of the following.
- The project is based on the esp-matter release/v1.2 version of the project. Either checkout to this project branch or if you're developing from the esp-matter project on github, checkout to the release/v1.2 branch (https://github.com/espressif/esp-matter/tree/release/v1.2)
- In the connectedhomeip submodule, checkout to the v1.2-branch-esp branch (https://github.com/espressif/connectedhomeip/tree/v1.2-branch-esp))

## 2. Post Commissioning Setup

No additional setup is required.

## 3. Device and Performance
The Device was tested on a network with the Google Nest Hub (2. Gen) 

### 3.1 Hardware and Pin Configurations

The following is some information on hardware

-   device used: esp32_devkit_c
-   power supply:
    *   breadboard 5V power supply for peripherals + usb for microcontroller
-   12 LED neopixel ring (WS2812)
    * DI - pin 5
    * VCC - 5V
    * GND - GND
-   ssd1306 oled (I2C address: 0X3C)
    * SDA - pin 21
    * SCL - pin 23
    * VCC - 3.3 V
    * GND - GND
-   bh1750 light intensity sensor (I2C address: 0X23)
    * SDA - pin 21
    * SCL - pin 23
    * VCC - 3.3 V
    * GND - GND

### 3.2 Additional Device info 

-   `Bootup` == Device just finished booting up. Device is not
    commissionined or connected to wifi yet.
-   `After Commissioning` == Device is conneted to wifi and is also
    commissioned and is rebooted.
