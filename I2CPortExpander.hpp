#pragma once

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <stdio.h>


class I2CPortExpander 
{
    private:
        i2c_inst * I2C_instance {nullptr};
        uint8_t _address;
        uint16_t _buf = 0;
        static const int DefaultTimeout = 100000;
    public:
        I2CPortExpander(i2c_inst* i2c, uint8_t address) {
            _address = address;
            I2C_instance = i2c;
        }

        void Write() {
            int wr = i2c_write_timeout_us(this->I2C_instance, this->_address, (uint8_t *) &_buf, 2, false, DefaultTimeout);
            if (wr != 2) {
                printf("IOExp write fail %d\n", wr);
            }
        }

        void Read() {
            int rd = i2c_read_timeout_us(this->I2C_instance, this->_address, (uint8_t *) &_buf, 2, false, DefaultTimeout);
            if (rd != 2) {
                printf("IOExp read fail %d\n", rd);
            }
        }

        void SetPin(uint pin, bool high) {
            _buf |= 1 << pin;
        }

        bool GetPin(uint pin) {
            return _buf & (1 << pin);
        }

        uint16_t GetAllPins() {
            return _buf;
        }

        void SetAllPins(uint16_t v) {
            _buf = v;
        }
};