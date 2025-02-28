

#target_sources(${PROJECT} PRIVATE LC2_I2C.cpp)

add_library(rotary Rotary.cpp)



# pull in common dependencies and additional i2c hardware support
target_link_libraries(rotary pico_stdlib)


# create map/bin/hex file etc.
#pico_add_extra_outputs(lcd_1602)

