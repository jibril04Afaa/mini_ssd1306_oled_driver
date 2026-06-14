#include "oled.h"

// screen dimensions
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// function implementations
void OLED_Init(I2C_HandleTypeDef* hi2c);
void OLED_Fill(uint8_t color);
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_UpdateScreen(I2C_HandleTypeDef* hi2c);
