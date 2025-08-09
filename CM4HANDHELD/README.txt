# Raspberry Pi CM4 Handheld Console with Custom Carrier Board

## 📌 Overview
This project is a **custom handheld gaming console** built around the **Raspberry Pi Compute Module 4 (CM4)**, featuring a fully designed carrier board with integrated power management, display interfaces, audio, input controls, and expansion capabilities.  
The system combines **embedded hardware design**, **Linux-based system integration**, and **custom firmware configuration** to create a portable gaming device capable of running retro and modern emulators.  

## 🛠 Features
- **Custom CM4 Carrier Board**
  - **Displays:**
    - Primary 5” DSI display (Waveshare) for gameplay
    - Secondary 2.4” SPI touchscreen for menus & status
  - **Audio:**
    - I²S audio DAC
    - Speaker + headphone jack with automatic switching
  - **Controller Input:**
    - Raspberry Pi Pico module for analog joysticks and button inputs via ADC and GPIO
  - **Storage:**
    - microSD slot for game/media storage (external if using eMMC CM4)
  - **Power:**
    - USB-C charging with battery management
    - Integrated battery fuel gauge
  - **Debug:**
    - Dedicated debug header with UART, SWD, and system reset
  - **Expansion:**
    - Breakout headers for unused GPIO

- **Software/Firmware**
  - RetroPie/EmulationStation setup for game management
  - Pico firmware for controller input over USB or serial
  - Touchscreen integration for system menu navigation

## 🖥 Technologies & Tools Used
- **Main Processor:** Raspberry Pi Compute Module 4
- **Microcontroller:** Raspberry Pi Pico
- **Displays:** Waveshare 5” DSI display (capacitive touch), 2.4” SPI TFT LCD (ILI9341)
- **Audio:** I²S audio DAC, headphone jack with auto switch
- **PCB Design:** KiCad
- **Firmware:** C/C++ for Pico, Linux configuration for CM4
- **Connectivity:** I²C, SPI, UART, I²S, USB
- **Power Management:** USB-C PD charging, fuel gauge IC