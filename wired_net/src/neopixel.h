#ifndef NEOPIXEL_H
#define NEOPIXEL_H
#include <stdint.h>

#define NUM_LEDS 300          
#define NEOPIXEL_PIN 2       

void neopixel_init(void);
void neopixel_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void neopixel_set_all(uint8_t r, uint8_t g, uint8_t b);
void neopixel_show(void);

#endif