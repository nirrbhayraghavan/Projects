#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"

void autotest();
const char keymap[16] = "DCBA#9630852*741";

char key = '\0';
int col = 0;

//////////////////////////////////////////////////////////////////////////////

const char* username = "raghav17";

//////////////////////////////////////////////////////////////////////////////

// Only uncomment ONE step at a time.

// When testing init_gpio_irq
//#define STEP1
// When testing init_keypad_irq
//#define STEP2
// When testing init_fifo_irq
#define STEP3

/****************************************************************/
// Copy in the three functions from lab 1 below.

void init_outputs() {
    for (int pin = 22; pin <= 25; ++pin) {
        hw_write_masked(&pads_bank0_hw->io[pin],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
        io_bank0_hw->io[pin].ctrl =
            (io_bank0_hw->io[pin].ctrl & ~IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) |
            (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

    #ifdef PADS_BANK0_GPIO0_ISO_BITS
            hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
    #endif
    }

    sio_hw->gpio_oe_set = (1u << 22);
    sio_hw->gpio_oe_set = (1u << 23);
    sio_hw->gpio_oe_set = (1u << 24);
    sio_hw->gpio_oe_set = (1u << 25);
    
}

void init_inputs() {
    for (int pin = 21; pin <= 26; pin += 5) {
        hw_write_masked(&pads_bank0_hw->io[pin],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
        io_bank0_hw->io[pin].ctrl =
            (io_bank0_hw->io[pin].ctrl & ~IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) |
            (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

        #ifdef PADS_BANK0_GPIO0_ISO_BITS
            hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
        #endif
    }


    sio_hw->gpio_oe_clr = (1u << 21) | (1u << 26);
}

void init_keypad() {
    for (int pin = 2; pin <= 5; ++pin) {
        hw_write_masked(&pads_bank0_hw->io[pin],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);

        io_bank0_hw->io[pin].ctrl =
            (io_bank0_hw->io[pin].ctrl & ~IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) |
            (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

        #ifdef PADS_BANK0_GPIO0_ISO_BITS
            hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
        #endif
    }

    sio_hw->gpio_oe_clr = (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5);

    for (int pin = 6; pin <= 9; ++pin) {
        hw_write_masked(&pads_bank0_hw->io[pin],
                        PADS_BANK0_GPIO0_IE_BITS,
                        PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);

        io_bank0_hw->io[pin].ctrl =
            (io_bank0_hw->io[pin].ctrl & ~IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) |
            (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

        #ifdef PADS_BANK0_GPIO0_ISO_BITS
            hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
        #endif
        
        sio_hw->gpio_clr = (1u << pin);
    }

    sio_hw->gpio_oe_set = (1u << 6) | (1u << 7) | (1u << 8) | (1u << 9);
}

/****************************************************************/
// Lab 2 functions

void gpio_isr() {
    if (gpio_get_irq_event_mask(21) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
        sio_hw->gpio_clr = (1u << 22) | (1u << 23) | (1u << 24) | (1u << 25);
        xosc_dormant();
    }
    if (gpio_get_irq_event_mask(26) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(26, GPIO_IRQ_EDGE_RISE);
        sio_hw->gpio_set = (1u << 22) | (1u << 23) | (1u << 24) | (1u << 25);
    }
}

void init_gpio_irq() {
    sio_hw->gpio_set = (1u << 22) | (1u << 23) | (1u << 24) | (1u << 25);
    gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
    gpio_acknowledge_irq(26, GPIO_IRQ_EDGE_RISE);
    gpio_add_raw_irq_handler_masked((1u << 21) | (1u << 26), gpio_isr);
    irq_set_enabled(IO_IRQ_BANK0, true);
    gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(26, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_dormant_irq_enabled(26, GPIO_IRQ_EDGE_RISE, true);
}

void drive_column() {
    static int col_index = 0;   
    const int col_pins[4] = {6, 7, 8, 9};

    sio_hw->gpio_clr = (1u << col_pins[0]);
    sio_hw->gpio_clr = (1u << col_pins[1]);
    sio_hw->gpio_clr = (1u << col_pins[2]);
    sio_hw->gpio_clr = (1u << col_pins[3]);
    sio_hw->gpio_set = (1u << col_pins[col_index]);
    sleep_ms(25);
    col_index = (col_index + 1) & 3;

}

void keypad_isr() {
    const int row_pins[4] = {2, 3, 4, 5};
    const int col_pins[4] = {6, 7, 8, 9};
    int row = 0, act, pin, index;

    static uint64_t prev_time = 0;
    uint64_t now = time_us_64();
    uint64_t diff = now - prev_time;

    if (diff < 50000) return;
    prev_time = now;

    while (row < 4) {
        pin = row_pins[row];

        if (gpio_get_irq_event_mask(pin) & GPIO_IRQ_EDGE_RISE) {
            gpio_acknowledge_irq(pin, GPIO_IRQ_EDGE_RISE);

            act = col & 0x03;

            int col_idx = 0;
            while (col_idx < 4) {
                if (gpio_get(col_pins[col_idx])) {
                    act = col_idx;
                    break;
                }
                col_idx++;
            }

            index = (pin - 2) + (act * 4);
            key = keymap[index];

#ifdef STEP3
            multicore_fifo_push_blocking((uint32_t) keymap[index]);
#else
            printf("Pressed: %c\n", key);
#endif
            return;
        }
        row++;
    }
}


void init_keypad_irq() {
    gpio_add_raw_irq_handler_masked((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), keypad_isr);
    for (int i = 2; i <= 5; i++) {
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_RISE, true);
    }

    irq_set_enabled(IO_IRQ_BANK0, true);
}

void init_fifo_irq() {
    init_keypad_irq();
    for (;;) {
        drive_column();
    }
}

/****************************************************************/
// Main

int main() {
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Uncomment when you need to run autotest.
    // Keep this commented out until you need it
    // since it adds a lot of time to the upload process.
    autotest();

    ///////////////////////
    // From lab 1

    init_inputs();
    init_outputs();
    init_keypad();

    ///////////////////////
    // For lab 2
    
    #ifdef STEP1
        init_gpio_irq();
        for(;;) {
            printf("Hello world\n");
            sleep_ms(1000);
        }
    #endif
    #ifdef STEP2
        init_keypad_irq();
        for (;;) {
            drive_column();
        }
    #endif
    #ifdef STEP3
        multicore_launch_core1(init_fifo_irq);
        for(;;) {
            key = (char) multicore_fifo_pop_blocking();
            printf("Pressed: %c\n", key);
        }
    #endif
    
    // Never reached.
    for(;;);
    return 0;
}
