cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(meteoroid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Main
add_executable(main
    pico_main.c
    max31856.c
    max31856.h
)

## Use USB or  UART default
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_adc hardware_spi hardware_gpio)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
