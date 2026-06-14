#ifndef OLED_H
#define OLED_H

// optimized for stm32f411xe boards
#include "stm32f411xe.h"

#define OLED_I2C_ADDR (0x3C << 1)

// define functions
void OLED_Init(I2C_HandleTypeDef* hi2c);
void OLED_Fill(uint8_t color);
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_UpdateScreen(I2C_HandleTypeDef* hi2c);

#endif
