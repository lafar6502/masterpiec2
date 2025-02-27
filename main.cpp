#include <iostream>
#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "lcd2/LCD_I2C.hpp"
#include <memory>

using namespace std;

#ifndef PICO_DEFAULT_I2C_INSTANCE
#error "NO PICO_DEFAULT_I2C_INSTANCE"
#endif

#define LED_DELAY_MS 500

// Perform initialisation
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

    
int main() {

    constexpr auto I2C = PICO_DEFAULT_I2C_INSTANCE();
    constexpr auto SDA = PICO_DEFAULT_I2C_SDA_PIN;
    constexpr auto SCL = PICO_DEFAULT_I2C_SCL_PIN;
    constexpr auto I2C_ADDRESS = 0x27;
    constexpr auto LCD_COLUMNS = 20;
    constexpr auto LCD_ROWS = 4;

    auto lcd = std::make_unique<LCD_I2C>(I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS, I2C, SDA, SCL);


    cout << "masterp!!\n" << endl;


    constexpr LCD_I2C::array HEART = {0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00};
    constexpr auto HEART_LOC = 0;
    lcd->CreateCustomChar(HEART_LOC, HEART);

    lcd->BacklightOn();
    lcd->SetCursor(0, 1);
    lcd->PrintString("Raspberry Pi Pico");
    lcd->SetCursor(1, 2);
    lcd->PrintString("I2C LCD Library");
    lcd->SetCursor(2, 2);
    lcd->PrintString("Made with love ");
    lcd->PrintCustomChar(HEART_LOC);
    lcd->SetCursor(3, 0);
    lcd->PrintString("by Cristian Cristea");

    int rc = pico_led_init();

    hard_assert(rc == PICO_OK);
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
    }
}