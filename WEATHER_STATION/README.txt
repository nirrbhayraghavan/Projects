# 🌤️ Proton Board Weather Station

### Overview
This project implements a **fully embedded weather-station data logger** running on the **Proton RP2350 board (Raspberry Pi Pico architecture)**.  
The firmware continuously measures **humidity**, **temperature**, and **pressure**, storing readings in a 2-D array and saving snapshots to an **SD card via SPI** whenever an **interrupt signal** is received.

It demonstrates low-level peripheral control, interrupt-driven data acquisition, and SPI-based non-volatile storage — all written in **C** using the **Pico SDK**.

---

## 🧠 Key Features
- **SPI SD card interface** for logging measurement data  
- **Interrupt-triggered data capture** from sensors or external GPIO events  
- **2D data structure**:  
  - 3 metrics × 10 samples each (`sensor_data[3][10]`)  
- **Dynamic array updates** to simulate continuous readings  
- **Portable C implementation** using `hardware/spi.h` and `hardware/irq.h`  
- Modular design ready to integrate real I²C or ADC sensors  

---

## ⚙️ System Architecture

```text
                +----------------------+
                |   Proton RP2350 MCU  |
                +----------------------+
                           │
                   ┌───────┴────────┐
                   │     SPI0 Bus    │
                   └───────┬────────┘
          MOSI ───────────▶│
          MISO ◀───────────│
           SCK ───────────▶│
            CS ───────────▶│
                           ▼
                 +----------------+
                 |   SD Card Slot  |
                 |  /data_log.txt  |
                 +----------------+
                           ▲
                     Interrupt (GPIO 20)
                           │
                    +--------------+
                    |  Sensor/Timer |
                    +--------------+
