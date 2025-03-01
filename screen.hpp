#ifndef _MASTERP2_SCREEN_HPP_INCLUDED_
#define _MASTERP2_SCREEN_HPP_INCLUDED_
#include <cstdint>
#include <cstddef>
#include <memory>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "lcd2/LCD_I2C.hpp"

class Screen 
{
    private:
        std::unique_ptr<LCD_I2C> _display = NULL;
        
    public:
    
    Screen(uint8_t address = 0x27, uint8_t columns = 16, uint8_t rows = 2, i2c_inst * I2C = PICO_DEFAULT_I2C_INSTANCE(), uint SDA = PICO_DEFAULT_I2C_SDA_PIN, uint SCL = PICO_DEFAULT_I2C_SCL_PIN)  {

    }

    static void PrintLine(uint8_t line, const char* txt);
    static void PrintAt(uint8_t line, uint8_t pos, const char* txt);
    static void PrintfLine(uint8_t line, const char* fmt, ...);
};










#endif