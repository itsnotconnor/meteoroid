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

#define SYNC_BYTE_UPPER 0xFE
#define SYNC_BYTE_LOWER 0xCA

#define PACKET_LEN 13U


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

        int index = 0;
        uint8_t packet[PACKET_LEN];

        /* Place Sync Bytes */
        packet[index] = (char) SYNC_BYTE_LOWER;  index++;
        packet[index] = (char) SYNC_BYTE_UPPER;  index++;
        /* Place counter */
        packet[index] = (char)((counter & 0x000000FF) >> 0);  index++;
        packet[index] = (char)((counter & 0x0000FF00) >> 8);  index++;
        packet[index] = (char)((counter & 0x00FF0000) >> 16); index++;
        packet[index] = (char)((counter & 0xFF000000) >> 24); index++;

        uint16_t temp_onboard_raw = read_adc(TEMP_ONBOARD_ADC_CHAN);
        //>>> 27 - (0.619556 - 0.706)/0.001721 == ~77.2289 degrees (double check this - slightly off)
        float temp_onboard_f = 27 - (convert_adc(temp_onboard_raw) - 0.706)/0.001721 - 10;// Added an additional -10 offset?? ;
        //printf("Temp_onboard: Celsius= %f : Fahrenheit= %f\n", (temp_onboard_f-32)*5/9, temp_onboard_f);
        float temp_onboard_c = (temp_onboard_f-32)*5/9;
        // Read Max31856 in C and F (F is just converted C)
        float temp_thermocouple = readCelsius();
        //printf("Thermocouple: Celsius= %f : ", temp_thermocouple);
        temp_thermocouple = readFahrenheit();

        /*
        Get raw bytes from onboard temp and send to device
        */
        packet[index] = (char)((temp_onboard_raw & 0x00FF) >> 0); index++;
        packet[index] = (char)((temp_onboard_raw & 0xFF00) >> 8); index++;
        
        /*
        Get f32 Fahrenheit and send bytes to device TODO
        */
        unsigned long d = *(unsigned long *)&temp_thermocouple;
        packet[index] = (char)(d & 0x000000FF) >> 0;  index++;
        packet[index] = (char)(d & 0x0000FF00) >> 8;  index++;
        packet[index] = (char)(d & 0x00FF0000) >> 16; index++;
        packet[index] = (char)(d & 0xFF000000) >> 24; index++;
        // end of line
        packet[index] = (char)0x0a; index++; // \n

        // Send data byte by byte
        for (int j=0 ; j<PACKET_LEN; j++){
            char byte_send = (char)packet[j];
            //Send over Pico USB  serial 
            printf("%c", byte_send);
            //Send over Pico GPIO serial UART pins
            uart_putc_raw(UART_ID,byte_send);
        }

        sleep_ms(2000);
        /*
        Toggle LED
        */
#ifndef PICO_DEFAULT_LED_PIN
#warning blink requires a board with a regular LED
#else
        gpio_put(LED_PIN, 1);
        sleep_ms(750);
        gpio_put(LED_PIN, 0);
        sleep_ms(750);
#endif
        // Increment counter
        counter++;
    }

}

/// \end::pico_main[]