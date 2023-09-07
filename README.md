# meteoroid
Pi Pico ADC Reader


# build
cd into meteoroid project dir

rm -rf build

mkdir build

cd build

cmake -DPICO_BOARD=pico -DPICO_SDK_PATH=/home/cnelson/Development/meateor/pico-sdk/ ..

make

# program
Program the pico by holding down the BOOT_SEL button while plugging in the device

Release BOOT_SEL once plugged in, and drag and drop .uf2 file over the mounted storage 'RPI-RP2'

# notes
If USB is failing to appear in PC devices, ensure that CMAKE is set to use USB and UART, and the tinyUSB lib has been checked out in the pico-sdk.

If not, run: 'git submodule update --init' in pico-sdk dir

Running from vscode terminal/tasks does not work (cant find PICO_TOOLCHAIN_PATH) must do from outside Terminal 