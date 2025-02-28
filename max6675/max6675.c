#include "max6675.h"
#include "stdio.h"

// Configuracion de MAX6675
max6675_config_t max6675_config = {0};

/**
 * @brief Inicializa el SPI para el MAX6675
 * @param config configuracion del MAX6675
 */
void max6675_init(max6675_config_t config) {
    // Guarda los datos de configuracion
    max6675_config = config;
    // Inicializa SPI
    spi_init(max6675_config.spi_bus, max6675_config.frequency);
    // Inicializa GPIOs
    gpio_set_function(max6675_config.sck, GPIO_FUNC_SPI);
    gpio_set_function(max6675_config.so, GPIO_FUNC_SPI);

    gpio_init(max6675_config.cs);
    gpio_set_dir(max6675_config.cs, GPIO_OUT);
    printf("max6675 spi init %d sck %d so %d cs %d\r\n", max6675_config.spi_bus, max6675_config.sck, max6675_config.so, max6675_config.cs);
}

/**
 * @brief Devuelve configuracion por defecto para el MAX6675
 * @return estructura de configuracion con SPI0, 4MHz, 
 * SCK = 18, RX = 16, CS = 17
 */
max6675_config_t max6675_get_default_config(void) {

    return (max6675_config_t) {
        .spi_bus = spi0,
        .frequency = 4000000,
        .sck = PICO_DEFAULT_SPI_SCK_PIN,
        .so = PICO_DEFAULT_SPI_RX_PIN,
        .cs = PICO_DEFAULT_SPI_CSN_PIN
    };
}

/**
 * @brief Pone el CS en low
 */
static void spi_cs_low(max6675_config_t* cfg) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cfg->cs, false);
    asm volatile("nop \n nop \n nop");
}

/**
 * @brief Pone el CS en high
 */
static void spi_cs_high(max6675_config_t* cfg) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cfg->cs, true);
    asm volatile("nop \n nop \n nop");
}

/**
 * @brief Hace una lectura de temperatura
 * @return temperatura en C
 */
static float max6675_get_temp(max6675_config_t* cfg) {
    // Buffer para leer
	uint8_t buffer[2] = {0};
    // Tiro abajo CS
    spi_cs_low(cfg);
    // Hace una lectura del bus
    int n = spi_read_blocking(cfg->spi_bus, 0, buffer, 2);
    // Deshabilito CS
    spi_cs_high(cfg);
    //printf("max6675 read blocking %d spi init %d sck %d so %d cs %d\r\n", n, max6675_config.spi_bus, max6675_config.sck, max6675_config.so, max6675_config.cs);
    // Armo los 16 bits
	uint16_t reading = (buffer[0] << 8) + buffer[1];
    // Los primeros tres bits no sirven
	reading >>= 3;
    // Devuelvo temperatura
	return reading * 0.25;
}

/**
 * @brief Obtiene la temperatura de la termocupla
 * @return temperatura en C
 */
float max6675_get_temp_c(max6675_config_t* cfg) {
    return max6675_get_temp(cfg);
}

/**
 * @brief Obtiene la temperatura de la termocupla
 * @return temperatura en K
 */
float max6675_get_temp_k(max6675_config_t* cfg) {
	return max6675_get_temp(cfg) + 273.15;
}

/**
 * @brief Obtiene la temperatura de la termocupla
 * @return temperatura en F
 */
float max6675_get_temp_f(max6675_config_t* cfg) {
	return (max6675_get_temp(cfg) * 1.8) + 32.0;
}
