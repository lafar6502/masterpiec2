

#target_sources(${PROJECT} PRIVATE LC2_I2C.cpp)

add_library(lcd_1602 LCD_I2C.cpp LCD_I2C.hpp)



# pull in common dependencies and additional i2c hardware support
target_link_libraries(lcd_1602 pico_stdlib hardware_i2c)


# create map/bin/hex file etc.
#pico_add_extra_outputs(lcd_1602)

