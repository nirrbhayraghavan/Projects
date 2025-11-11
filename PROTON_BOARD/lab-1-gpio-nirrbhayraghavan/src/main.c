#include <stdio.h>
#include "pico/stdlib.h"


//////////////////////////////////////////////////////////////////////////////

const char* username = "raghav17";

//////////////////////////////////////////////////////////////////////////////

void autotest();

void init_outputs() {
    for (int pin = 22; pin <= 25; ++pin) 
    {
        hw_write_masked(&pads_bank0_hw->io[pin],
            PADS_BANK0_GPIO0_IE_BITS,
            PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
        );
        io_bank0_hw->io[pin].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
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

int main() {
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();
    init_outputs();
    init_inputs();
    init_keypad();

    // Leave commented until you actually need it.
    // Can take significantly longer to upload code when uncommented.
    //autotest();
    
    ////////////////////////////////////
    // All your code goes below.

    int COLS[4] = {6, 7, 8, 9};   
    int ROWS[4] = {2, 3, 4, 5};  

    while (true) {
        for (int i = 0; i < 4; ++i) {
            int col_pin = COLS[i];
            int row_pin = ROWS[i];
            int led_pin = 25 - i;           

            sio_hw->gpio_set = (1u << col_pin);

            sleep_ms(10);

            if ((sio_hw->gpio_in & (1u << row_pin)) == 0) 
            {
                sio_hw->gpio_clr = (1u << led_pin);
            } 
            else 
            {
                sio_hw->gpio_set = (1u << led_pin);
            }


            sio_hw->gpio_clr = (1u << col_pin);
        }
    }

    
    
    
    
    
    
    // All your code goes above.
    ////////////////////////////////////

    // An infinite loop is necessary to 
    // ensure control flow remains with user.
    //for(;;);

    // Never reached.
    return 0;
}