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

  /*!    @brief  For compatibility with older versions
         @returns Temperature in F or 0x on failure! 
    */

// class MAX6675 {
// public:
//   MAX6675(int8_t SCLK, int8_t CS, int8_t MISO);

float readCelsius(void);
float readFahrenheit(void);

// private:
//   int8_t sclk, miso, cs;
//   uint8_t spiread(void);
// };
