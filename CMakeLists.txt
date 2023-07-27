cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(snowball C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Main
add_executable(read_adc
    adc_reader.c
)

## Use USB instead of UART default
pico_enable_stdio_usb(read_adc 1)
pico_enable_stdio_uart(read_adc 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(read_adc pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(read_adc)
