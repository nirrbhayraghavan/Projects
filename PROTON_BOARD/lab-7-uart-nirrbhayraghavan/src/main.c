#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

//////////////////////////////////////////////////////////////////////////////

// No autotest, but you might want to set this anyway.
const char* username = "raghav17";

// When testing basic TX/RX
//#define STEP2
// When connecting UART to printf(), fgets()
//#define STEP3
// When testing UART IRQ for buffering
//#define STEP4
// When testing PCS
#define STEP5

//////////////////////////////////////////////////////////////////////////////

void init_uart() {
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));

    uart_init (uart0, 115200);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
}

#ifdef STEP2
int main() {
    init_uart();
    for (;;) {
        char buf[2];
        uart_read_blocking(uart0, (uint8_t*)buf, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0'; // Ensure null-termination
        uart_puts(uart0, "You said: ");
        uart_puts(uart0, buf);
        uart_puts(uart0, "\n");
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP3

// 3.3
int _read(__unused int handle, char *buffer, int length) {
    // Your code here to read from the UART and fill the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we read from the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to read, read 1 
    // character at a time from the UART until the buffer is 
    // filled or the length is reached. 
    int i;

i    for (i = 0; i < length; i++)
    {
        char c = uart_getc(uart0);
        
        if (c == 8)
        {
            if (i > 0)
                i--;
            uart_putc(uart0, 8);
            uart_putc(uart0, 32);
            uart_putc(uart0, 8);
            continue;

        }

        buffer[i] = c;
        uart_putc(uart0, c);
    }

    return i;
}

int _write(__unused int handle, char *buffer, int length) {
    // Your code here to write to the UART from the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we write to the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to write, write 1
    // character at a time to the UART until the length is reached. 
    int i;

    for (i = 0; i < length; i++)
    {
        uart_putc(uart0, buffer[i]);
    }

    return i;
}

int main()
{
    init_uart();

    setbuf(stdout, NULL);  // Disable buffering for stdout
    setbuf(stdin, NULL);   // Disable buffering for stdin
    char name[8];
    int age = 0;
    for(;;) {
        printf("Enter your name and age: ");
        scanf("%s %d", name, &age);
        printf("Hello, %s! You are %d years old.\n", name, age);
        sleep_ms(100);  // in case the output loops and is too fast
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP4

#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// add this here so that compiler does not complain about implicit function
void uart_rx_handler();

void init_uart_irq() {
    uart_set_fifo_enabled(uart0, false);
    uart_set_irqs_enabled(uart0, true, false);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
}

void uart_rx_handler() {
    uart0_hw->icr = 1 << 4;
    if (seridx == BUFSIZE)
        return;
    char c = uart0_hw->dr; 
    if (c == 0x0A)
        newline_seen = 1;
    if (c == 8 && seridx > 0)
    {
        uart_putc(uart0, 8);
        uart_putc(uart0, 32);
        uart_putc(uart0, 8);
        seridx--;
        serbuf[seridx] = '\0';
        return;
    }
    else 
    {
        uart_putc(uart0, c);
        serbuf[seridx] = c;
        seridx++;
    }
}

int _read(__unused int handle, char *buffer, int length) {
    while (newline_seen == 0)
    {
        sleep_ms(5);
    }
    newline_seen = 0;
    if (length >= seridx)
    {
        for (int i = 0; i < seridx; i++)
            buffer[i] = serbuf[i];
    }
    else
    {
        for (int i = 0; i < length; i++)
            buffer[i] = serbuf[i];
    }

    seridx = 0;
    return length;
}

int _write(__unused int handle, char *buffer, int length) {
    int i;

    for (i = 0; i < length; i++)
    {
        uart_putc(uart0, buffer[i]);
    }

    return i;
}

int main() {
    init_uart();
    init_uart_irq();

    setbuf(stdout, NULL); // Disable buffering for stdout

    char name[8];
    int age = 0;
    for(;;) {
        printf("Enter your name and age: ");
        scanf("%s %d", name, &age);
        // THIS IS IMPORTANT.
        fflush(stdin);
        printf("Hello, %s! You are %d years old.\r\n", name, age);
        sleep_ms(100);  // in case the output loops and is too fast
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP5

// Copy global variables, init_uart_irq, uart_rx_handler, _read, and _write from STEP4.
#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// add this here so that compiler does not complain about implicit function
void uart_rx_handler();

void init_uart_irq() {
    uart_set_fifo_enabled(uart0, false);
    uart_set_irqs_enabled(uart0, true, false);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
}

void uart_rx_handler() {
    uart0_hw->icr = 1 << 4;
    if (seridx == BUFSIZE)
        return;
    char c = uart0_hw->dr; 
    if (c == 0x0A)
        newline_seen = 1;
    if (c == 8 && seridx > 0)
    {
        uart_putc(uart0, 8);
        uart_putc(uart0, 32);
        uart_putc(uart0, 8);
        seridx--;
        serbuf[seridx] = '\0';
        return;
    }
    else 
    {
        uart_putc(uart0, c);
        serbuf[seridx] = c;
        seridx++;
    }
}

int _read(__unused int handle, char *buffer, int length) {
    while (newline_seen == 0)
    {
        sleep_ms(5);
    }
    newline_seen = 0;
    if (length >= seridx)
    {
        for (int i = 0; i < seridx; i++)
            buffer[i] = serbuf[i];
    }
    else
    {
        for (int i = 0; i < length; i++)
            buffer[i] = serbuf[i];
    }

    seridx = 0;
    return length;
}

int _write(__unused int handle, char *buffer, int length) {
    int i;

    for (i = 0; i < length; i++)
    {
        uart_putc(uart0, buffer[i]);
    }

    return i;
}

void cmd_gpio(int argc, char **argv) {
    // This is the main command handler for the "gpio" command.
    // It will call either cmd_gpio_out or cmd_gpio_set based on the arguments.
    
    // Ensure that argc is at least 2, otherwise print an example use case and return.

    // If the second argument is "out":
    //      Ensure that argc is exactly 3, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Set the pin to output using gpio_init and gpio_set_dir.
    //      Print a success message.
    
    // If the second argument is "set":
    //      Ensure that argc is exactly 4, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Check if the pin has been initialized as a GPIO output, if not, return.
    //      Convert the fourth argument to an integer value (0 or 1) using atoi.
    //      Check if the value is valid (0 or 1), otherwise print an error and return.
    //      Set the pin to the specified value using gpio_put.
    //      Print a success message.
    
    // Else, print an unknown command error.
    if (argc < 2)
    {
        printf("Example Use Case\n");
        return;
    }
    if (strcmp(argv[1], "out") == 0)
    {
        if (argc == 3)
        {
            int pin = atoi(argv[2]);
            if (pin >= 0 && pin <= 47)
            {
                gpio_init(pin);
                gpio_set_dir(pin, GPIO_OUT);
                printf("Success\n");
                return;
            }
            else
            {
                printf("Fail\n");
                return;
            }
        }
        else
        {
            printf("Example Use Case\n");
            return;
        }
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        if (argc == 4)
        {
            int pin = atoi(argv[2]);
            if (pin >= 0 && pin <= 47)
            {
                if (gpio_get_dir(pin) == GPIO_OUT)
                {
                    int value = atoi(argv[3]);
                    if (value == 0 || value == 1)
                    {
                        gpio_put(pin, value);
                        printf("Success\n");
                        return;
                    }
                    else
                    {
                        printf("Fail\n");
                        return;
                    }
                }
                else
                {
                    printf("Not Output Mode");
                    return;
                }
            }
            else
            {
                printf("Fail\n");
                return;
            }
        }
        else
        {
            printf("Example Use Case\n");
            return;
        }
    }
    else
    {
        printf("Unknown\n");
    }

}

int main() 
{

    init_uart();
    init_uart_irq();

    setbuf(stdout, NULL);
    printf("%s's Peripheral Command Shell (PCS)\n", username);
    printf("Enter a command below.\n");
    int argc;
    char *argv[10];
    char input[100];

    for (;;)
    {
        printf("\r\n> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            continue;

        fflush(stdin);
        input[strcspn(input, "\r\n")] = '\0';
        argc = 0;
        char *token = strtok(input, " ");

        while (token != NULL && argc < 10) 
        {
            argv[argc] = token;
            argc++;
            token = strtok(NULL, " ");
        }
        
        cmd_gpio(argc, argv);
    }

    return 0;
}

#endif

//////////////////////////////////////////////////////////////////////////////
