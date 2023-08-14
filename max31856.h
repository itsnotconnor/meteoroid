/*
Pulled from adafruit

// this library is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

*/

/**************************************************************************/
/*!
    @brief  Class for communicating with thermocouple sensor
*/
/**************************************************************************/

#include <stdio.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

/**************************************************************************/
/*!
    @brief  <init>
    @returns void
*/
/**************************************************************************/
void  max31856_init(void);

/**************************************************************************/
/*!
    @brief  Read the Celsius temperature
    @returns Temperature in C or NAN on failure!
*/
/**************************************************************************/
float readCelsius(void);

/**************************************************************************/
/*!
    @brief  Read the Fahenheit temperature
    @returns Temperature in F or NAN on failure!
*/
/**************************************************************************/
float readFahrenheit(void);


