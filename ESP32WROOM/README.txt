# ESP32 Photo Server with Custom Carrier Board

## 📌 Overview
This project is a **custom ESP32-based photo server** with a fully designed carrier board, capable of capturing images via a camera module, storing them locally, and serving them over Wi-Fi through an HTTP interface.  
The system integrates **hardware design, firmware development, and IoT connectivity**, and was later used as the foundation for a **hands-on workshop** where participants built and programmed the system from scratch.

## 🛠 Features
- **Custom ESP32 Carrier Board**
  - USB-C power input with voltage regulation
  - MicroSD card slot for image storage
  - SPI TFT display header for local image preview
  - Camera module interface (OV2640 or similar)
  - GPIO breakout for future expansion
- **Photo Server Firmware**
  - Captures images from the camera module
  - Stores images on MicroSD
  - Serves images over Wi-Fi via HTTP
  - Supports real-time streaming in browser
- **Workshop-Ready Design**
  - Step-by-step assembly and programming guide
  - Firmware and hardware files available for replication

## 🖥 Technologies & Tools Used
- **Microcontroller:** ESP32-WROOM-32
- **Firmware Language:** C++ (Arduino Framework / ESP-IDF compatible)
- **PCB Design:** KiCad
- **Camera Module:** OV2640 (ESP32-CAM or separate module)
- **Display:** SPI-based TFT LCD (e.g., ILI9341)
- **Storage:** MicroSD card
- **Power Regulation:** AP2112K-3.3 or equivalent
- **Debugging:** USB-to-UART via CH340C, optional SWD/JTAG headers

