#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define IRQ_PIN  20   

#define NUM_METRICS 3
#define NUM_POINTS  10

volatile bool data_ready_flag = false;

float sensor_data[NUM_METRICS][NUM_POINTS] = {
    {45.3, 46.1, 45.8, 46.4, 45.6, 46.2, 45.9, 46.3, 46.0, 45.7},
    {22.9, 23.1, 23.0, 23.2, 23.3, 23.1, 23.0, 23.3, 23.4, 23.2},
    {1011.5, 1011.8, 1012.0, 1011.9, 1012.1, 1011.7, 1011.6, 1011.9, 1012.2, 1011.8}
};


void spi_write_byte(uint8_t data) {
    spi_write_blocking(SPI_PORT, &data, 1);
}

void spi_write_buffer(const uint8_t *buffer, size_t len) {
    spi_write_blocking(SPI_PORT, buffer, len);
}

void spi_send_command(uint8_t cmd, uint32_t arg) {
    uint8_t packet[6];
    packet[0] = 0x40 | cmd;
    packet[1] = (arg >> 24) & 0xFF;
    packet[2] = (arg >> 16) & 0xFF;
    packet[3] = (arg >> 8) & 0xFF;
    packet[4] = arg & 0xFF;
    packet[5] = 0x95; 
    spi_write_buffer(packet, 6);
}


void sd_write_data() {
    printf("\n=== Logging new data snapshot ===\n");

    for (int i = 0; i < NUM_METRICS; i++) {
        switch (i) {
            case 0: printf("Humidity: "); break;
            case 1: printf("Temperature: "); break;
            case 2: printf("Pressure: "); break;
        }

        for (int j = 0; j < NUM_POINTS; j++) {
            printf("%.2f ", sensor_data[i][j]);
        }
        printf("\n");
    }
    printf("Data stored successfully\n");
}


void irq_callback(uint gpio, uint32_t events) {
    data_ready_flag = true;
}


void init_spi_sd() {
    spi_init(SPI_PORT, 1 * 1000 * 1000);  
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}

void init_interrupt_pin() {
    gpio_init(IRQ_PIN);
    gpio_set_dir(IRQ_PIN, GPIO_IN);
    gpio_pull_down(IRQ_PIN);
    gpio_set_irq_enabled_with_callback(IRQ_PIN, GPIO_IRQ_EDGE_RISE, true, &irq_callback);
}

int main() {
    stdio_init_all();
    printf("Proton Board Data Logger with SPI SD Interface\n");

    init_spi_sd();
    init_interrupt_pin();

    printf("Waiting GPIO interrupt %d...\n", IRQ_PIN);

    while (true) {
        if (data_ready_flag) {
            data_ready_flag = false;

            for (int i = 0; i < NUM_METRICS; i++) {
                for (int j = 0; j < NUM_POINTS; j++) {
                    sensor_data[i][j] += ((rand() % 5) - 2) * 0.1f;
                }
            }

            gpio_put(PIN_CS, 0);
            sd_write_data();
            gpio_put(PIN_CS, 1);
        }

        sleep_ms(10);
    }
}
