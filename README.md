# snowball
Pi Pico ADC Reader


# build
cd into snowball project dir

rm -rf build

mkdir build

cd build

cmake -DPICO_SDK_PATH=pico -DPICO_SDK_PATH=/home/cnelson/Development/pico/pico-sdk/ ..

make

# program
Program the pico by holding down the BOOT_SEL button while plugging in the device

Release BOOT_SEL once plugged in, and drag and drop .uf2 file over the mounted storage 'RPI-RP2'
