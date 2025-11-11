#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

//////////////////////////////////////////////////////////////////////////////

const char* username = "raghav17";

//////////////////////////////////////////////////////////////////////////////

uint32_t adc_fifo_out = 0;
void display_init_pins();
void display_init_timer();
void display_char_print(const char* buffer);
void autotest();

//////////////////////////////////////////////////////////////////////////////

// Only uncomment ONE step at a time.

// When testing manual ADC single-shot conversion
#define STEP2
// When testing manual ADC free-running conversion
//#define STEP3
// When testing automated ADC sampling with DMA
//#define STEP4

//////////////////////////////////////////////////////////////////////////////

void init_adc() {
    adc_init();            
    adc_gpio_init(45);      
    adc_select_input(5);    
}

uint16_t read_adc() {
    return adc_read();
}

void init_adc_freerun() {
    adc_init();             
    adc_gpio_init(45);      
    adc_select_input(5);    
    adc_run(true);
}

void init_dma() {
    dma_channel_set_read_addr(0, &adc_hw->fifo, false);
    dma_channel_set_write_addr(0, &adc_fifo_out, false);
    dma_hw->ch[0].transfer_count = (DMA_CH0_TRANS_COUNT_MODE_VALUE_TRIGGER_SELF << 28) | (1 << 0);
    uint32_t temp = 0;
    temp |= 1 << 2;     
    temp |= DREQ_ADC << 17;    
    temp |= 1;            
    dma_hw->ch[0].ctrl_trig = temp;
}

void init_adc_dma() {
    init_dma();
    init_adc_freerun();
    adc_fifo_setup(true, true, 1, false, false);
}

//////////////////////////////////////////////////////////////////////////////

int main()
{
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Uncomment when you need to run autotest.
    // Keep this commented out until you need it
    // since it adds a lot of time to the upload process.
    autotest();

    // Step 2 - singleshot
    #ifdef STEP2
    init_adc();
    for(;;) {
        printf("ADC Result: %d     \r", read_adc());
        // We've found that when we do NOT send a newline character,
        // the output is not flushed immediately, which can cause
        // the output to be delayed or not appear at all in some cases.
        // The fflush function forces a flush.
        fflush(stdout);
        sleep_ms(250);
    }
    #endif

    // Step 3 - freerun
    #ifdef STEP3
    init_adc_freerun();

    int i = 0;
    for(;;) {
        printf("ADC Result: %d     \r", adc_hw->result);
        fflush(stdout);
        sleep_ms(250);
    }
    #endif

    // Step 4 - adc_dma
    #ifdef STEP4

    // Don't forget to copy in display.c from lab 3.
    display_init_pins();
    display_init_timer();

    init_adc_dma();
    char buffer[10];
    for(;;) {
        float f = (adc_fifo_out * 3.3) / 4095.0;
        snprintf(buffer, sizeof(buffer), "%1.7f", f);
        display_char_print(buffer);

        // If you need to debug without the display, 
        // you can uncomment the following lines.

        // printf("ADC Result: %s     \r", buffer);
        // fflush(stdout);
        
        sleep_ms(250);
    }
    #endif

    for(;;);
    return 0;
}
