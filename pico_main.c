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

// #include "pico/binary_info.h"
// #include "hardware/spi.h"
#include "max31856.h"

/// \tag::pico_main[]

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define TEMP_ONBOARD_ADC_CHAN 4U

static inline float convert_adc(uint16_t raw){
    return ((float)raw * 3.3f / (1 << 12));
}

uint16_t read_adc(uint8_t channel){
    /*
    Read the Pico ADC

    Select ADC input 0 (GPIO26) //TODO
    * Select an ADC input. 0...3 are GPIOs 26...29 respectively.
    * Input 4 is the onboard temperature sensor.
    */
    adc_select_input(channel);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    uint16_t result = adc_read();

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

    /* ADC setup */
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);

    // Init max31856 sensor
    max31856_init();

    /* LED setup */
#ifndef PICO_DEFAULT_LED_PIN
#warning blink requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

    uint32_t counter = 0;
    // Read temp forever
    while(1){

        uint16_t temp_onboard = read_adc(TEMP_ONBOARD_ADC_CHAN);
        printf("Temp onboard\n");
        //>>> 27 - (0.619556 - 0.706)/0.001721 == ~77.2289 degrees (double check this - slightly off)
        float temperature = 27 - (convert_adc(temp_onboard) - 0.706)/0.001721;
        printf("Raw value: 0x%03x, voltage: %f V\n", temp_onboard, temperature);


        printf("%x  ", (char)((temp_onboard & 0xFF00) >> 8));
        printf("%x\n", (char)((temp_onboard & 0x00FF) >> 0));

        // Read Max31856
        float temp_thermocouple = readCelsius();
        printf("Thermocouple: %f C\n", temp_thermocouple);


        // Send out a string, with CR/LF conversions
        uart_puts(UART_ID, " Hello, worlds!\n");
        /*
        Get raw bytes from onboard temp and send to device
        */
        uart_putc_raw(UART_ID, (char)((temp_onboard & 0xFF00) >> 8)); // first byte 
        uart_putc_raw(UART_ID, (char)((temp_onboard & 0x00FF) >> 0)); // second byte
        uart_puts(UART_ID, "\n");

        sleep_ms(1000);

#ifndef PICO_DEFAULT_LED_PIN
#warning blink requires a board with a regular LED
#else
        gpio_put(LED_PIN, 1);
        sleep_ms(850);
        gpio_put(LED_PIN, 0);
        sleep_ms(850);
#endif
        // Increment counter
        counter++;

    }

}

/// \end::pico_main[]