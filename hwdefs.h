#ifndef _HWDEFS_H_INCLUDED_
#define _HWDEFS_H_INCLUDED_
//pin definitions

//GPIO

//0 - SDA wyswietlacz I2C0
#define MP_DISPLAY_SDA 0
//1 - SCL wyswietlacz I2C0
#define MP_DISPLAY_SCL 1
//2 - pompa co
#define MP_PUMP_CO   2
//3 - pompa cwu
#define MP_PUMP_CWU  3
//4 - pompa obieg
#define MP_PUMP_CIRC 4
//5 - podajnik
#define MP_STOKER    10
//6 - dmuchawa
#define MP_BLOWER    6
//7 - zapalarka
#define MP_IGNITER    7
//8 - we: zalacz pompe co
#define MP_INP_PUMP_CO  8
//9 - we: zalacz pompe cwu
#define MP_INP_PUMP_CWU 9
//10 - sd card cs
#define MP_SDCARD_CS 5
//11 - sd card spi1
//12 - sd card spi1
//13 - sd card spi1

//14 - onewire data pio0
#define MP_ONEWIRE_DATA   14
//15 - wejście termostat pok.
#define MP_INP_THERMOSTAT 15
//16 - max6675 rx spi0
#define MP_SPI_RX 16
//17 - max6675 cs spi0 
#define MP_MAX6675_CS 17
//18 - max6675 SCK spi0
#define MP_SPI_SCK 18
//19 - spi0 tx (nieuzyty)
#define MP_SPI_TX  19
//20 - enkoder przycisk
#define MP_INP_ENCODER_BTN 20
//21 - enkoder A
#define MP_INP_ENCODER_A 21
//22 - enkoder B
#define MP_INP_ENCODER_B 22
//23##brak
//24##brak
//25##brak
//26 - przycisk back
#define MP_INP_BACK_BTN 26
//27 - detektor zera
#define MP_INP_ZERO_DET 27
//28 ADC - input z przeplywomierza
#define MP_INP_FLOW_VOL 28

#endif