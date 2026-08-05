// src/wizchip_spi.c
#include "wizchip_spi.h"
#include "wizchip_conf.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_RST  20

static void cs_select(void)   { gpio_put(PIN_CS, 0); }
static void cs_deselect(void) { gpio_put(PIN_CS, 1); }

static uint8_t spi_read_byte(void) {
    uint8_t rx = 0, tx = 0xFF;
    spi_write_read_blocking(spi0, &tx, &rx, 1);
    return rx;
}
static void spi_write_byte(uint8_t tx) {
    uint8_t rx;
    spi_write_read_blocking(spi0, &tx, &rx, 1);
}
static void spi_read_burst(uint8_t *buf, uint16_t len) {
    uint8_t tx = 0xFF;
    for (uint16_t i = 0; i < len; i++) spi_write_read_blocking(spi0, &tx, &buf[i], 1);
}
static void spi_write_burst(uint8_t *buf, uint16_t len) {
    uint8_t rx;
    for (uint16_t i = 0; i < len; i++) spi_write_read_blocking(spi0, &buf[i], &rx, 1);
}

void wizchip_reset(void) {
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_RST, 1);
    sleep_ms(200);
}

void wizchip_spi_initialize(void) {
    spi_init(spi0, 8 * 1000 * 1000);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    wizchip_reset();

    reg_wizchip_cs_cbfunc(cs_select, cs_deselect);
    reg_wizchip_spi_cbfunc(spi_read_byte, spi_write_byte);
    reg_wizchip_spiburst_cbfunc(spi_read_burst, spi_write_burst);

    uint8_t tx_sizes[8] = {2,2,2,2,2,2,2,2};
    uint8_t rx_sizes[8] = {2,2,2,2,2,2,2,2};
    ctlwizchip(CW_INIT_WIZCHIP, (void*)tx_sizes);
    // note: ioLibrary's CW_INIT_WIZCHIP actually wants both sizes passed via
    // wizchip_setnetsize or a driver-specific call depending on version — see w5500.h
}