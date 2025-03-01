#include <iostream>
#include <cstdint>
#include <cstddef>
#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "hwdefs.h"
#include "lcd2/LCD_I2C.hpp"
#include <memory>
#include "screen.hpp"
#include "Rotary.hpp"
#include "CircularBuffer.hpp"
#include "pico/binary_info.h"
#include "FatFsSd.h"

extern "C" {
#include "include/events.h"
#include "max6675/max6675.h"
}
#include "TempSensors.hpp"


using namespace std;

#ifndef PICO_DEFAULT_I2C_INSTANCE
#error "NO PICO_DEFAULT_I2C_INSTANCE"
#endif

mp_event _event_buffer[100];
CircularBuffer<mp_event> g_EVENTQ(_event_buffer, sizeof(_event_buffer)/sizeof(mp_event));

TempSensors g_tempSensors;

#define LED_DELAY_MS 200

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

Rotary rot;

void rotary_callback(uint gpio, uint32_t events) {
    bool v1 = gpio_get(MP_INP_ENCODER_A);
    bool v2 = gpio_get(MP_INP_ENCODER_B);
    if (gpio == MP_INP_ENCODER_A) {
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
            mp_event e;
            e.code = 1;
            e.i_arg = g_pos;
            g_EVENTQ.Enqueue(e);
            break;
        case DIR_CCW:
            g_pos--;
            e.code = 2;
            e.i_arg = g_pos;
            g_EVENTQ.Enqueue(e);
            break;
        case DIR_NONE:
            break;
    }
    
}
    


int main() {

    constexpr auto I2C = PICO_DEFAULT_I2C_INSTANCE();
    constexpr auto I2C_ADDRESS = 0x27;
    constexpr auto LCD_COLUMNS = 20;
    constexpr auto LCD_ROWS = 4;

    gpio_init(MP_INP_ENCODER_A);
    gpio_set_dir(MP_INP_ENCODER_A, false);
    gpio_init(MP_INP_ENCODER_B);
    gpio_set_dir(MP_INP_ENCODER_B, false);
    gpio_set_irq_enabled_with_callback(MP_INP_ENCODER_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &rotary_callback);
    gpio_set_irq_enabled_with_callback(MP_INP_ENCODER_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &rotary_callback);


    auto lcd = std::make_unique<LCD_I2C>(I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS, I2C, MP_DISPLAY_SDA, MP_DISPLAY_SCL);

    stdio_init_all();

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
    int cnt = 0;
    int d1 = 0, d2 = 0;
    
    max6675_config_t mcf = max6675_get_default_config();
    mcf.cs = MP_MAX6675_CS;
    mcf.sck = MP_SPI_SCK;
    mcf.so = MP_SPI_RX;
    max6675_init(mcf);

    int n = g_tempSensors.Initialize(MP_ONEWIRE_DATA, pio0);
    if (n != 0) {
        printf("temp sensors failed to init %d\n", n);
    }
    else {
        printf("temp sensors inited.. sensors found:\n", g_tempSensors.GetSensorCount());
    }

    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
        
        int evs = g_EVENTQ.GetCount();
        while(g_EVENTQ.GetCount() > 0) 
        {
            const mp_event& ev = g_EVENTQ.Dequeue();
            printf("event %d, %d\r\n", ev.code, ev.i_arg);
        }
        
        std::string s = "dal:" + std::to_string(g_tempSensors.GetSensorCount()) + " evs:" + std::to_string(evs) + " ";
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