#include <iostream>
#include <cstdint>
#include <cstddef>
#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "lcd2/LCD_I2C.hpp"
#include <memory>
#include "screen.hpp"
#include "rotary/Rotary.hpp"
#include "pico/binary_info.h"
extern "C" {
#include "max6675/max6675.h"
}


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

int g_r1 = 0, g_r2 = 0, g_pos=0;

#define ROT_P1 6
#define ROT_P2 7
Rotary rot;

void rotary_callback(uint gpio, uint32_t events) {
    bool v1 = gpio_get(ROT_P1);
    bool v2 = gpio_get(ROT_P2);
    if (gpio == ROT_P1) {
        g_r1++;
    }
    else {
        g_r2++;
    }

    unsigned char d = rot.process(v1, v2);
    printf("%d,%d:%d\r\n", v1, v2, d);
    switch(d) {
        case DIR_CW:
            g_pos++;
            break;
        case DIR_CCW:
            g_pos--;
            break;
        case DIR_NONE:
            break;
    }
}
    
int main() {

    constexpr auto I2C = PICO_DEFAULT_I2C_INSTANCE();
    constexpr auto SDA = 0;// PICO_DEFAULT_I2C_SDA_PIN;
    constexpr auto SCL = 1;//PICO_DEFAULT_I2C_SCL_PIN;
    constexpr auto I2C_ADDRESS = 0x27;
    constexpr auto LCD_COLUMNS = 20;
    constexpr auto LCD_ROWS = 4;

    gpio_init(ROT_P1);
    gpio_set_dir(ROT_P1, false);
    gpio_init(ROT_P2);
    gpio_set_dir(ROT_P2, false);
    gpio_set_irq_enabled_with_callback(ROT_P1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &rotary_callback);
    gpio_set_irq_enabled_with_callback(ROT_P2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &rotary_callback);

    auto lcd = std::make_unique<LCD_I2C>(I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS, I2C, SDA, SCL);

    stdio_init_all();

    cout << "masterp!!\n" << endl;
    cout << "sda " << SDA << ", SCL " << SCL << endl;

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
    int cnt = 0;
    int d1 = 0, d2 = 0;
    
    max6675_config_t mcf = max6675_get_default_config();
    max6675_init(mcf);


    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
        cout << "sda " << SDA << ", SCL " << SCL << endl;
        std::string s = "cnt:" + std::to_string(cnt++) + " ";
        lcd->PrintString(0, s);
        s = "r1:" + std::to_string(g_r1) + ", r2:" + std::to_string(g_r2) + " ";
        lcd->PrintString(1, s);
        s = "pos:" + std::to_string(g_pos) + " ";
        lcd -> PrintString(2, s);
        float ft = max6675_get_temp_c(&mcf);
        s = "temp:" + std::to_string(ft);
        lcd -> PrintString(3, s);
    }
}