

#target_sources(${PROJECT} PRIVATE LC2_I2C.cpp)

add_library(max6675 max6675.c max6675.h)



# pull in common dependencies and additional i2c hardware support
target_link_libraries(max6675 pico_stdlib hardware_spi)


# create map/bin/hex file etc.
#pico_add_extra_outputs(lcd_1602)

