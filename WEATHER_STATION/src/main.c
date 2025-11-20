#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************/

#define SD_MISO -1
#define SD_CS -1
#define SD_SCK -1
#define SD_MOSI -1

#define NUM_METRICS 3
#define NUM_DATA 10

#define IRQ_PIN -1

volatile bool data_ready_flag = false;

float sensor_data[NUM_METRICS][NUM_DATA] = {0};

uint8_t write_index[NUM_METRICS] = {0, 0, 0};

volatile uint8_t latest_uart_byte = 0;



/*******************************************************************/

void init_spi_sdcard() {
    spi_init(spi0, 400000);   

    gpio_set_function(SD_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_SCK,  GPIO_FUNC_SPI);
    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, true);
    gpio_put(SD_CS, 1);
}

void disable_sdcard() {
    gpio_put(SD_CS, 1);
}

void enable_sdcard() {
    gpio_put(SD_CS, 1);
}

void sdcard_io_high_speed() {
    spi_set_baudrate(spi0, 10000000);
}

void init_sdcard_io() {
    init_spi_sdcard();

    uint8_t temp = 0xFF;
    for (int i = 0; i < 10; i++) {
        spi_write_blocking(spi0, &temp, 1);
    }
}

void irq_callback(uint gpio, uint32_t events) {
    data_ready_flag = true;
}

void init_irq_pin() {
    gpio_init(IRQ_PIN);
    gpio_set_dir(IRQ_PIN, GPIO_IN);
    gpio_pull_down(IRQ_PIN);
    gpio_set_irq_enabled_with_callback(IRQ_PIN, GPIO_IRQ_EDGE_RISE, true, &irq_callback);
}

uint8_t spi_read_byte() {
    uint8_t tx = 0xFF;
    uint8_t rx = 0;
    spi_write_read_blocking(spi0, &tx, &rx, 1);
    return rx;
}

void store_spi_sample(uint8_t data) {
    uint8_t metric = (data >> 6) & 0x03;
    uint8_t value  = data & 0x3F;

    if (metric >= NUM_METRICS) return;

    uint8_t idx = write_index[metric];
    sensor_data[metric][idx] = (float)value;

    write_index[metric] = (idx + 1) % NUM_DATA;
}

void write_dataset_to_sd() {
    FIL fp;
    f_open(&fp, "log.txt", FA_WRITE | FA_OPEN_APPEND);

    char line[128];
    for (int m = 0; m < NUM_METRICS; m++) {
        for (int i = 0; i < NUM_DATA; i++) {
            snprintf(line, sizeof(line), "%d,%d,%.2f\n", m, i, sensor_data[m][i]);
            f_puts(line, &fp);
        }
    }

    f_close(&fp);
}




/*******************************************************************/

void init_uart();
void init_uart_irq();
void date(int argc, char *argv[]);
void command_shell();

int main() {
    // Initialize the standard input/output library
    init_uart();
    init_uart_irq();
    
    init_sdcard_io();
    init_irq_pin();
    // SD card functions will initialize everything.
    command_shell();
    while (true) {
        if (data_ready_flag) {
            data_ready_flag = false;

            enable_sdcard();
            uint8_t data = spi_read_byte();
            disable_sdcard();

            store_spi_sample(data);
            write_dataset_to_sd();

            printf("Stored: addr=%d value=%d\n", data>>7, data & 0x7F);
        }

        sleep_ms(5);
    }
}