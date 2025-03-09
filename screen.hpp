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
        char* _buf = NULL;
        uint8_t _rows;
        uint8_t _cols;
        uint64_t _modified[2] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    public:
    
    Screen(uint8_t address = 0x27, uint8_t columns = 16, uint8_t rows = 2, i2c_inst * I2C = PICO_DEFAULT_I2C_INSTANCE(), uint SDA = PICO_DEFAULT_I2C_SDA_PIN, uint SCL = PICO_DEFAULT_I2C_SCL_PIN)  {
        if (rows * columns > 128) {
            //printf("this will not work dude");
            return;
        }
        _rows = rows;
        _cols = columns;
        _buf = new char[rows*columns + 1];
        //memset(_buf, 0, rows*columns + 1);
        //_display = std::unique_ptr<LCD_I2C>(new LCD_I2C(address, columns, rows, I2C, SDA, SCL));

    }

    static void PrintLine(uint8_t line, const char* txt);
    static void PrintAt(uint8_t line, uint8_t pos, const char* txt);
    static void PrintfLine(uint8_t line, const char* fmt, ...);

    static int NumberOfSetBits(uint64_t i)
    {
        i = i - ((i >> 1) & 0x5555555555555555UL);
        i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
        return (int) ((((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL) >> 56);
    }

    void Sync();
    void Clear();
};










#endif