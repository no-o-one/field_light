#include "neopixel.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2815gen.pio.h"

static PIO pio = pio0;
static uint sm = 0;  //state machine
static uint32_t pixel_buf[NUM_LEDS];

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

void neopixel_init(void) {
    uint offset = pio_add_program(pio, &ws2815gen_program);
    ws2815gen_program_init(pio, sm, offset, NEOPIXEL_PIN, 800000.0f);
}

void neopixel_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < NUM_LEDS) pixel_buf[index] = urgb(r, g, b);
}

void neopixel_set_all(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t c = urgb(r, g, b);
    for (int i = 0; i < NUM_LEDS; i++) pixel_buf[i] = c;
}

void neopixel_show(void) {
    for (int i = 0; i < NUM_LEDS; i++) put_pixel(pixel_buf[i]);
}