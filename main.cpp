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
#include "f_util.h"
#include "ff.h"
#include "util.h"
}
#include "TempSensors.hpp"
#include "I2CPortExpander.hpp"


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

void gpio_extender_callback(uint gpio, uint32_t events) {
    bool v1 = gpio_get(gpio);
    printf("EXT %d %d ev%d\n", gpio, v1, events);
}

void gpio_callback(uint gpio, uint32_t events) {

    if (gpio == MP_INP_ENCODER_A || gpio == MP_INP_ENCODER_B) {
        rotary_callback(gpio, events);
        return;
    }
    else if (gpio == MP_EXTENDER_INT) {
        gpio_extender_callback(gpio, events);
        return;
    }
    else {
        printf("unexpected intr pin%d : %d\n", gpio, events);
    }
}

int n0 = 0;
void test_file_write() {
    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Open a file and write to it
    FIL fil;
    const char* const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }
    if (f_printf(&fil, "Hello, world! %d\n", n0++) < 0) {
        printf("f_printf failed\n");
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Unmount the SD card
    f_unmount("");
    
}

void test_simplefs() 
{
    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    // Open a file and write to it
    FIL fil;
    const char* const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        printf("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return;
    }
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Unmount the SD card
    fr = f_unmount("");
    if (FR_OK != fr) {
        printf("unmount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

}

void test_fatfs() {

    sd_card_t* pcard = sd_get_by_num(0);
    FatFsNs::SdCard *card = FatFsNs::FatFs::add_sd_card(pcard);
    if (card == NULL) {
        printf("init 1 fail null card\n");
        return;
    }
    printf("mount..\n");
    FRESULT fr = card->mount();
    if (fr != FR_OK) {
        printf("mount fail %d\n", fr);
        return;
    }
    FatFsNs::File file;
    char const* const filename = "filename.txt";
    fr = file.open(filename, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK) {
        printf("file open fail %d\n", fr);
        return;
    }
    char const* const str = "Hello, world!\n";
    if (file.printf(str) < strlen(str)) {
        printf("file write fail len\n");
    }
    else {
        printf("written\n");
    }
    fr = file.close();
    if (fr != FR_OK) {
        printf("file close fail %d\n", fr);
    }
    fr = card->unmount();
    if (fr != FR_OK) {
        printf("unmount fail %d\n", fr);
    }
    printf("test done\n");
}

int main0() 
{

    stdio_init_all();
    sleep_ms(5000);
    printf("start initing sd driver\n");

    //if (!sd_init_driver()) {
    //    panic("failed to init driver");
    //}
    printf("initing max6675\n");
    max6675_config_t mcf = max6675_get_default_config();
    mcf.cs = MP_MAX6675_CS;
    mcf.sck = MP_SPI_SCK;
    mcf.so = MP_SPI_RX;
    if (false) {
        max6675_init(mcf);
    }
    else {
        gpio_init(mcf.cs);
        gpio_set_dir(mcf.cs, GPIO_OUT);
        printf("max6675 spi already inited %d sck %d so %d cs %d\r\n", mcf.spi_bus, mcf.sck, mcf.so, mcf.cs);
    }
    
    bool bb = false;
    gpio_put(MP_MAX6675_CS, bb);
    while(true) {
        
        
        sleep_ms(5000);
        printf("write test %d\n", bb);
        //test_file_write();
        //test_fatfs();
        test_simplefs();
        printf("done\n");
        sleep_ms(1000);
        if (true) {
            printf("try read temp\n");
            float ft = max6675_get_temp_c(&mcf);
            printf("temp read %f\n", ft);
        }
        //gpio_put(MP_MAX6675_CS, bb);
        //bb=!bb;
    }

    return 0;
}

int main() {

    constexpr auto I2C = PICO_DEFAULT_I2C_INSTANCE();
    constexpr auto I2C_ADDRESS = 0x27;
    constexpr auto LCD_COLUMNS = 20;
    constexpr auto LCD_ROWS = 4;

    gpio_init(MP_INP_ENCODER_A);
    gpio_set_dir(MP_INP_ENCODER_A, false);
    gpio_pull_up(MP_INP_ENCODER_A);
    gpio_init(MP_INP_ENCODER_B);
    gpio_set_dir(MP_INP_ENCODER_B, false);
    gpio_pull_up(MP_INP_ENCODER_B);

    gpio_set_irq_callback(gpio_callback);

    gpio_set_irq_enabled(MP_INP_ENCODER_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(MP_INP_ENCODER_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    gpio_init(MP_INP_ENCODER_BTN);
    gpio_set_dir(MP_INP_ENCODER_BTN, false);
    gpio_pull_up(MP_INP_ENCODER_BTN);
    
    gpio_init(MP_EXTENDER_INT);
    gpio_set_dir(MP_EXTENDER_INT, false);
    gpio_pull_up(MP_EXTENDER_INT);
    gpio_set_irq_enabled(MP_EXTENDER_INT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    stdio_init_all();
    if (true) {
        printf("scan i2c\n");
        sleep_ms(5000);
        scan_i2c_bus(I2C, MP_DISPLAY_SDA, MP_DISPLAY_SCL);
        printf("i2c scan done\n");
    }
    

    auto lcd = std::make_unique<LCD_I2C>(I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS, I2C, MP_DISPLAY_SDA, MP_DISPLAY_SCL);

    

    I2CPortExpander expander(I2C, 0x24);
    

    cout << "masterp!!\n" << endl;
    sleep_ms(5000);

    if (!sd_init_driver()) {
        panic("failed to init sd card driver");
    }
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
    if (false) {
        max6675_init(mcf);
    }
    else if (true) {
        gpio_init(mcf.cs);
        gpio_set_dir(mcf.cs, GPIO_OUT);
        printf("max6675 spi already inited %d sck %d so %d cs %d\r\n", mcf.spi_bus, mcf.sck, mcf.so, mcf.cs);
    }
    

    int n = g_tempSensors.Initialize(MP_ONEWIRE_DATA, pio0);
    if (n != 0) {
        printf("temp sensors failed to init %d\n", n);
    }
    else {
        printf("temp sensors inited.. sensors found:\n", g_tempSensors.GetSensorCount());
    }

    bool bb = false;
    gpio_put(MP_MAX6675_CS, bb);
    printf("max 6675 cs %d\n", bb);
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
        expander.Read();
        int evs = g_EVENTQ.GetCount();
        while(g_EVENTQ.GetCount() > 0) 
        {
            const mp_event& ev = g_EVENTQ.Dequeue();
            printf("event %d, %d\r\n", ev.code, ev.i_arg);
        }
        
        std::string s = "dal:" + std::to_string(g_tempSensors.GetSensorCount()) + " evs:" + std::to_string(evs) + " ";
        lcd->PrintString(0, s);
        s = "exp:" + std::to_string(expander.GetAllPins()) + ", r2:" + std::to_string(g_r2) + " ";
        lcd->PrintString(1, s);
        s = "pos:" + std::to_string(g_pos) + " ";
        lcd -> PrintString(2, s);
        float ft = max6675_get_temp_c(&mcf);
        s = "temp:" + std::to_string(ft);
        lcd -> PrintString(3, s);
        if (g_pos == -8) {
            printf("testing file..\n");
            test_file_write();
        }
        if (evs > 0) {
            printf("updating expander pins %d\n", g_pos);
            expander.SetAllPins(g_pos);
            expander.Write();
        }
        printf(".\n");
    }
}