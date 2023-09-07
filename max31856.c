// /*

// Custom implementation of Adafruit MAX31856

// TODO
// */

#include "max31856.h"




// // this library is public domain. enjoy!
// // https://learn.adafruit.com/thermocouple/




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

static size_t read_register(const uint8_t *buf, uint8_t len) {
    cs_select();
    //delay 10 micro seconds
    sleep_us(10);

    size_t bytes_read = spi_read_blocking(spi_default, 0x0, buf, len);

    cs_deselect();
    return bytes_read;
}


void max31856_init(){
    // This example will use SPI0 at 10MHz.
    spi_init(spi_default, 1 * 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);

    // Make the all 3 SPI pins available to picotool (rx, tx, sck) **CS is manually triggered**  
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));


    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));
    }
#else
  #error ERROR: spi_default && PICO_DEFAULT_SPI_CSN_PIN not defined
#endif

float readCelsius(void) {
    uint16_t v;

    const size_t buffer_length = 2;
    uint8_t buf[buffer_length];
    read_register( buf, buffer_length);

    if(read_register( buf, buffer_length) > 0){
        v =  (uint16_t)buf[0] << 8;
        v |= (uint16_t)buf[1];
        if (v & 0x4) {
          // uh oh, no thermocouple attached!
          return (float)0xBADBADFF;
          // return -100;
        }
        v >>= 3;
        return (float)v * 0.25;
    }
    else{
      return (float)0xBEEFBEEF;
    }

}

float readFahrenheit(void) { return readCelsius() * 9.0 / 5.0 + 32; }
