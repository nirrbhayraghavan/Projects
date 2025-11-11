#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
// 7-segment display message buffer
// Declared as static to limit scope to this file only.
static char msg[8] = {
    0x3F, // seven-segment value of 0
    0x06, // seven-segment value of 1
    0x5B, // seven-segment value of 2
    0x4F, // seven-segment value of 3
    0x66, // seven-segment value of 4
    0x6D, // seven-segment value of 5
    0x7D, // seven-segment value of 6
    0x07, // seven-segment value of 7
};
extern char font[]; // Font mapping for 7-segment display
static int index = 0; // Current index in the message buffer

// We provide you with this function for directly displaying characters.
// However, it can't use the decimal point, which display_print does.
void display_char_print(const char message[]) {
    for (int i = 0; i < 8; i++) {
        msg[i] = font[message[i] & 0xFF];
    }
}

/********************************************************* */
// Implement the functions below.


void display_init_pins() {
    for (int g = 10; g <= 20; g++) {
        sio_hw->gpio_clr = 1u << g;              
        sio_hw->gpio_oe_set = 1u << g;           
        io_bank0_hw->io[g].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
        hw_clear_bits(&pads_bank0_hw->io[g], PADS_BANK0_GPIO0_ISO_BITS);
        hw_set_bits(&pads_bank0_hw->io[g], PADS_BANK0_GPIO0_IE_BITS);
    }   
}

void display_isr() {
    timer1_hw->intr = 1u << 0;
    sio_hw->gpio_clr = (0xFFu << 10) | (0x7u << 18);
    sio_hw->gpio_set = ((uint32_t)msg[index] << 10) | ((uint32_t)index << 18);
    index = (index + 1) & 7;
    timer1_hw->alarm[0] = timer1_hw->timerawl + 3000u;
}

void display_init_timer() {
    timer1_hw->intr = 1u << 0;
    uint32_t now = timer1_hw->timerawl;
    timer1_hw->alarm[0] = now + 3000u;
    timer1_hw->inte = 1u << 0;
    irq_set_exclusive_handler(TIMER1_IRQ_0, display_isr);
    irq_set_enabled(TIMER1_IRQ_0, true);
}

void display_print(const uint16_t message[]) {
    for (int i = 0; i < 8; i++) {
        uint16_t evt = message[i];
        uint8_t chr = evt & 0xFF;
        char segs = font[chr & 0x7F];
        if (evt & 0x100)   
            segs |= 0x80;
        else
            segs &= ~0x80;
        msg[i] = segs;
    } 
}