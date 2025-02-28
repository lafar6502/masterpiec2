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
    
    Screen(uint8_t address, uint8_t columns, uint8_t rows, i2c_inst * I2C, uint SDA = PICO_DEFAULT_I2C_SDA_PIN, uint SCL = PICO_DEFAULT_I2C_SCL_PIN)  {

    }

    void Initialize() {
        
    }
};










#endif