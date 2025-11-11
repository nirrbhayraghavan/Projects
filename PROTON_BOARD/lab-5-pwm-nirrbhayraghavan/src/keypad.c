#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"

// Global column variable
int col = -1;

// Global key state
static bool state[16]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[17] = "DCBA#9630852*741";

// Defined here to avoid circular dependency issues with autotest
// You can see the struct definition in queue.h
void keypad_drive_column();
void keypad_isr();

/********************************************************* */
// Implement the functions below.

void keypad_init_pins() {
    int row_pins[4] = {2, 3, 4, 5};
    for (int i = 0; i < 4; i++) {
        int p = row_pins[i];
        pads_bank0_hw->io[p] |= PADS_BANK0_GPIO0_IE_BITS;
        pads_bank0_hw->io[p] &= ~PADS_BANK0_GPIO0_ISO_BITS;
        io_bank0_hw->io[p].ctrl = (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);
        sio_hw->gpio_oe_clr = (1u << p);
        sio_hw->gpio_clr    = (1u << p);
        gpio_pull_down(p);
    }
    for (int p = 6; p <= 9; p++) {
        pads_bank0_hw->io[p] |= PADS_BANK0_GPIO0_IE_BITS;
        pads_bank0_hw->io[p] &= ~PADS_BANK0_GPIO0_ISO_BITS;
        io_bank0_hw->io[p].ctrl = (5u << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);
        sio_hw->gpio_oe_set = (1u << p);
        sio_hw->gpio_clr    = (1u << p);
    }
}

void keypad_init_timer() {
    const uint32_t alarm0_mask = (1u << 0);
    const uint32_t alarm1_mask = (1u << 1);
    timer_hw->intr = alarm0_mask | alarm1_mask;
    uint64_t tick = timer_hw->timerawl;
    timer_hw->alarm[0] = tick + 1000000;  
    timer_hw->alarm[1] = tick + 1100000;   
    timer_hw->inte |= alarm0_mask;
    timer_hw->inte |= alarm1_mask;
    int irq_ids[2];
    irq_ids[0] = timer_hardware_alarm_get_irq_num(timer_hw, 0);
    irq_ids[1] = timer_hardware_alarm_get_irq_num(timer_hw, 1);
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            irq_set_exclusive_handler(irq_ids[i], keypad_drive_column);
        } else {
            irq_set_exclusive_handler(irq_ids[i], keypad_isr);
        }
        irq_set_enabled(irq_ids[i], true);
    }
}

void keypad_drive_column() {
    timer_hw->intr = (1u << 0);
    col++;
    if (col > 3) {
        col = 0;
    }
    uint32_t col_mask = (0xFu << 6);
    sio_hw->gpio_clr = col_mask;
    sio_hw->gpio_set = (1u << (6 + col));
    timer_hw->alarm[0] = timer_hw->alarm[0] + 25000;
}

uint8_t keypad_read_rows() {
    uint32_t input_snapshot = sio_hw->gpio_in;
    uint8_t rows = (input_snapshot & 0x3C) >> 2;
    return rows;
}

void keypad_isr() {
    timer_hw->intr = (1u << 1);
    for (int row = 0; row < 4; row++) {
        int idx = ((col & 3) << 2) | row;
        bool pressed_now = ((keypad_read_rows() >> row) & 1) == 1;
        if (pressed_now != state[idx]) {
            state[idx] = pressed_now;
            key_push(pressed_now ? (0x100 | (uint8_t)keymap[idx]) : (uint8_t)keymap[idx]);
        }
    }
    timer_hw->alarm[1] = timer_hw->timerawl + 25000;
}

