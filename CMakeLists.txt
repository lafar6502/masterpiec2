#PATH system env must contain path to arm toolchain and cmake
#set PICO_SDK_PATH env var
#install C++ Extensions in VSCode
message("PICO_SDK_PATH IS $ENV{PICO_SDK_PATH}")
set(PROJECT masterp2)
cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(${PROJECT} C CXX ASM)
pico_sdk_init()
add_executable(${PROJECT} main.cpp screen.cpp)
add_subdirectory(lcd_1602_i2c)
add_subdirectory(lcd2)
add_subdirectory(rotary)
add_subdirectory(max6675)
target_link_libraries(${PROJECT} lcd_1602 rotary max6675 pico_stdlib hardware_i2c hardware_spi)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)