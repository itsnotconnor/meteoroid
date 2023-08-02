// /*

// Custom implementation of Adafruit MAX31856

// TODO
// */

#include "max31856.h"

#include <stdio.h>
#include "pico/binary_info.h"
#include "hardware/spi.h"


// // this library is public domain. enjoy!
// // https://learn.adafruit.com/thermocouple/


// /**************************************************************************/
// /*!
//     @brief  Initialize a MAX6675 sensor
//     @param   SCLK The Arduino pin connected to Clock
//     @param   CS The Arduino pin connected to Chip Select
//     @param   MISO The Arduino pin connected to Data Out
// */
// /**************************************************************************/
// MAX6675::MAX6675(int8_t SCLK, int8_t CS, int8_t MISO) {
//   sclk = SCLK;
//   cs = CS;
//   miso = MISO;

//   // define pin modes
//   pinMode(cs, OUTPUT);
//   pinMode(sclk, OUTPUT);
//   pinMode(miso, INPUT);

//   digitalWrite(cs, HIGH);
// }

// /**************************************************************************/
// /*!
//     @brief  Read the Celsius temperature
//     @returns Temperature in C or NAN on failure!
// */
// /**************************************************************************/
#ifdef PICO_DEFAULT_SPI_CSN_PIN
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}
#endif


#if defined(spi_default) && defined(PICO_DEFAULT_SPI_CSN_PIN)
static void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    cs_select();
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect();
    sleep_ms(1);
}

static void read_register(const uint8_t *buf, uint8_t len) {
    // uint8_t buf[2];
    // buf[0] = reg;
    // buf[1] = data;
    cs_select();

    spi_read_blocking(spi_default, 0x0, buf, len);

    cs_deselect();
}
#endif


float readCelsius(void) {

  uint16_t v;

  //digitalWrite(cs, LOW);
  //delayMicroseconds(10);

    //   v = spiread();
    //   v <<= 8;
    //   v |= spiread();

  const size_t buffer_length = 2;
  uint8_t buf[buffer_length];
  read_register( buf, buffer_length);

  //digitalWrite(cs, HIGH);

  if (v & 0x4) {
    // uh oh, no thermocouple attached!
    return 0xBADBADFF;
    // return -100;
  }

  v >>= 3;

  return v * 0.25;
}

// /**************************************************************************/
// /*!
//     @brief  Read the Fahenheit temperature
//     @returns Temperature in F or NAN on failure!
// */
// /**************************************************************************/
float readFahrenheit(void) { return readCelsius() * 9.0 / 5.0 + 32; }

// byte MAX6675::spiread(void) {
//   int i;
//   byte d = 0;

//   for (i = 7; i >= 0; i--) {
//     digitalWrite(sclk, LOW);
//     delayMicroseconds(10);
//     if (digitalRead(miso)) {
//       // set the bit to 0 no matter what
//       d |= (1 << i);
//     }

//     digitalWrite(sclk, HIGH);
//     delayMicroseconds(10);
//   }

//   return d;
// }

// void max_init(){
//     // This example will use SPI0 at 10MHz.
//     spi_init(spi_default, 10 * 1000 * 1000);
//     gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
//     gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

//     // Make the SPI pins available to picotool
//     bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

//     // Chip select is active-low, so we'll initialise it to a driven-high state
//     gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
//     gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
//     gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

//     // Make the CS pin available to picotool
//     bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));
//     }