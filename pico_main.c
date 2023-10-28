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

#define LOW  0
#define HIGH 1

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define DATA_REQ_PIN 9 //GPIO9

#define TEMP_ONBOARD_ADC_CHAN 4U

#define SYNC_BYTE_UPPER 0xFE
#define SYNC_BYTE_LOWER 0xCA

/*
Packet Len = 13
Composition = [ SYNC_BYTE_LOWER, SYNC_BYTE_UPPER, 
            COUNTER_BYTE_0, COUNTER_BYTE_1, COUNTER_BYTE_2, COUNTER_BYTE_3, 
            TEMP_ONBOARD_BYTE_LOWER, TEMP_ONBOARD_BYTE_UPPER, 
            THERMO_BYTE_0, THERMO_BYTE_1, THERMO_BYTE_2, THERMO_BYTE_3, 
            END_OF_LINE_0A
            ]
*/
#define PACKET_LEN 13U


/*
Expected output::
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 20.250000 :                                                                                                
[ 0xca  0xfe  
0x30  0x00  0x00  0x00  
0x01  0x03  
0x66  0xe6  0x88  0x42  
0x0a ]                                                  
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 20.250000 :                                                                                                
[ 0xca  0xfe  0x31  0x00  0x00  0x00  0x01  0x03  0x66  0xe6  0x88  0x42  0x0a ]                                                  
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 20.500000 :                                                                                                
[ 0xca  0xfe  0x32  0x00  0x00  0x00  0x01  0x03  0xcd  0xcc  0x89  0x42  0x0a ]                                                  
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 19.750000 :                                                                                                
[ 0xca  0xfe  0x33  0x00  0x00  0x00  0x01  0x03  0x9a  0x19  0x87  0x42  0x0a ]                                                  
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 20.500000 :                                                                                                
[ 0xca  0xfe  0x34  0x00  0x00  0x00  0x01  0x03  0xcd  0xcc  0x89  0x42  0x0a ]                                                  
Temp_onboard: Celsius= 19.571745 : Fahrenheit= 67.229141                                                                          
Thermocouple: Celsius= 20.500000 :                                                                                                
[ 0xca  0xfe  0x35  0x00  0x00  0x00  0x01  0x03  0xcd  0xcc  0x89  0x42  0x0a ]                                                  

*/



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

    const uint data_req = DATA_REQ_PIN;
    gpio_init(data_req);
    gpio_set_dir(data_req, GPIO_IN);

    uint32_t counter = 0;
    // Read temp forever
    while(1){

        sleep_ms(10);
        //Active Low Data Ready
        bool is_master_ready = gpio_get(data_req);
        if(!is_master_ready){
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
            //printf("Thermocouple: Celsius= %f : \n", temp_thermocouple);
            temp_thermocouple = readFahrenheit();

            /*
            Get raw bytes from onboard temp and send to device
            */
            packet[index] = (char)((temp_onboard_raw & 0x00FF) >> 0); index++;
            packet[index] = (char)((temp_onboard_raw & 0xFF00) >> 8); index++;
            
            /*
            Get f32 Fahrenheit and send bytes to device
            */
            char *dest = packet; dest+=index;
            memcpy(dest, &temp_thermocouple, sizeof(float));
            index = index + sizeof(float);

            // end of line
            packet[index] = (char)0x0a; // \n

            // Send data byte by byte
            for (int j=0 ; j<PACKET_LEN; j++){
                char byte_send = (char)packet[j];
                //Send over Pico USB  serial to Pi ZERO
                printf("%c", byte_send);

                //Send over Pico GPIO serial UART pins
                uart_putc_raw(UART_ID,byte_send);
            }

            /*
            Toggle LED
            */
    #ifndef PICO_DEFAULT_LED_PIN
    #warning blink requires a board with a regular LED
    #else
            gpio_put(LED_PIN, 1);
            sleep_ms(200);
            gpio_put(LED_PIN, 0);
            sleep_ms(200);
    #endif
            // Increment counter
            counter++;

            
            //Active High Data Not Ready
            while( !gpio_get(data_req) ){
                sleep_ms(100);
            }

        }
    }

}

/// \end::pico_main[]