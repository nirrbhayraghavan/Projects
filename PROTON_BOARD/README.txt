# ⚡ Proton Board Embedded Systems Projects

### Overview
This repository documents a complete set of **embedded firmware modules** developed for the **Proton development board (RP2350-based)**.  
Each lab incrementally builds hardware functionality — from basic GPIO control to DMA-driven peripherals — culminating in a fully integrated embedded system capable of real-time sensing, display, and communication.

All code is written in **C**, compiled with the ARM toolchain, and deployed directly to the Proton board via USB-UART flashing.

---

## 🧩 Implemented Modules

| Module | Description | Key Features |
|---------|--------------|---------------|
| **1. GPIO & Timing** | Implemented LED blinking and button input handling. | Configured registers manually, implemented software delay loops, introduced timer-based event timing. |
| **2. PWM Control** | Generated PWM output for LEDs and speaker tone generation. | Adjusted frequency and duty cycle, implemented brightness fading and buzzer tone mapping. |
| **3. Keypad Interface** | Created a matrix keypad driver. | Scanned rows/columns using GPIO, implemented debouncing and interrupt-driven key detection. |
| **4. Display System** | Developed a 7-segment and LCD output driver. | Multiplexed display updates via timer interrupts, displayed ASCII-encoded messages and numeric values. |
| **5. UART Communication** | Built a full-duplex UART interface. | Sent/received characters to a serial terminal, parsed command input, enabled dynamic LED or display control via serial commands. |
| **6. SPI & ADC Integration** | Connected sensors over SPI and read analog inputs via ADC. | Implemented periodic sampling, data averaging, and SPI data transfer with clock synchronization. |
| **7. DMA Automation** | Offloaded repetitive data transfers from CPU using DMA. | Configured DMA channels for memory-to-peripheral and peripheral-to-memory streaming; verified throughput improvement. |
| **8. System Integration (Final)** | Combined all modules into a unified firmware. | Integrated UART commands, keypad input, display updates, and PWM control into one real-time main loop with interrupt coordination. |

---

## 🧠 Technical Highlights

- **Register-level configuration** for all peripherals — no HAL used.  
- **Interrupt-driven event handling** (TIMER, GPIO, UART RX).  
- **DMA-based parallelism** to free CPU during streaming tasks.  
- **Real-time display updates** via timer multiplexing.  
- **Custom modular architecture** (`gpio.c`, `uart.c`, `display.c`, `keypad.c`, `dma.c`, etc.).  
- **Cycle-accurate debugging** using logic analyzer and serial output logs.

---

## ⚙️ Project Structure

