// #include <stdio.h>
// #include "pico/stdlib.h"

// int main() {
//     stdio_init_all();

//     printf("Hello, worlds!\n");
//     while(1){
// 	printf("...\n");
// 	sleep_ms(2500);
//     }
//     //unreachable
//     return 0;
// }


/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */



#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

/// \tag::adc_reader[]

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1



uint16_t read_adc(uint8_t channel){
    /*
    Read the Pico ADC

    Select ADC input 0 (GPIO26) //TODO
    * Select an ADC input. 0...3 are GPIOs 26...29 respectively.
    * Input 4 is the onboard temperature sensor.
    */
    adc_select_input(channel);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);

    //Return 12bit adc value from channel
    return result;
}



int main() {
    stdio_init_all();
    /* UART setup */
    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    // // Send out a character without any conversions
    // uart_putc_raw(UART_ID, 'A');

    // // Send out a character but do CR/LF conversions
    // uart_putc(UART_ID, 'B');

    /* ADC setup */
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);

    /* LED setup */
#ifndef PICO_DEFAULT_LED_PIN
#warning blink requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
#endif


    // Create packet
    while(1){
        printf("GPIO26 AIN\n");
        uint16_t value = read_adc(0);

        printf("GPIO27 AIN\n");
        value = read_adc(1);

        printf("GPIO28 AIN\n");
        value = read_adc(2);

        printf("GPIO29 AIN\n");
        value = read_adc(3);

        printf("Temp onboard\n");
        value = read_adc(4);

        // Send out a string, with CR/LF conversions
        uart_puts(UART_ID, " Hello, worlds!\n");

        sleep_ms(5000);

#ifndef PICO_DEFAULT_LED_PIN
#warning blink requires a board with a regular LED
#else
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
#endif
    }

}

/// \end::adc_reader[]