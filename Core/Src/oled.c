#include "oled.h"
#include "main.h"
#include <string.h>


// screen dimensions
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// creates a copy of the OLED screen, essentially
static uint8_t OLED_Buffer[OLED_WIDTH * OLED_HEIGHT / 8];

// helper function to send commands
static void OLED_WriteCmd(I2C_HandleTypeDef* hi2c, uint8_t cmd)
{
	/*format - HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
	 	 	 	 	 	 	 	 	 	 	 	 * uint16_t DevAddress,
	 	 	 	 	 	 	 	 	 	 	 	 * uint16_t MemAddress,
	 	 	 	 	 	 	 	 	 	 	 	 * uint16_t MemAddSize,
	 	 	 	 	 	 	 	 	 	 	 	 * uint8_t *pData,
	 	 	 	 	 	 	 	 	 	 	 	 * uint16_t Size,
	 	 	 	 	 	 	 	 	 	 	 	 * uint32_t Timeout);
	 	 	 	 	 	 	 	 	 	 	 	 * */
	HAL_I2C_Mem_Write(hi2c,
					  OLED_I2C_ADDR,
					  0x00,
					  I2C_MEMADD_SIZE_8BIT,
					  &cmd,
					  1,
					  HAL_MAX_DELAY);
}

// function implementations
void OLED_Init(I2C_HandleTypeDef* hi2c)
{
	HAL_Delay(100); // ad some delay for the screen

	OLED_WriteCmd(hi2c, 0xAE); // display OFF
	OLED_WriteCmd(hi2c, 0x20); // set memory addressing mode
	OLED_WriteCmd(hi2c, 0x00); // set horizontal addressing mode
	OLED_WriteCmd(hi2c, 0x8D); // charge pump setting - boost voltage
	OLED_WriteCmd(hi2c, 0x14); // enable charge pump
	OLED_WriteCmd(hi2c, 0xAF); // display ON
}


void OLED_Fill(uint8_t color)
{
	// sets every bit to white (0xFF) or black (0x00)
	uint8_t fill_val = (color == 0) ? 0x00 : 0xFF;

	// update all 1024 bytes
	memset(OLED_Buffer, fill_val, sizeof(OLED_Buffer));
}


void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	// annoying pixel math..
	if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return; // bounds control

	if (color)
	{
		OLED_Buffer[x + (y/8) * OLED_WIDTH] |= (1 << (y%8)); // turn pixel on
	}
	else
	{
		OLED_Buffer[x + (y/8) * OLED_WIDTH] &= ~(1 << (y%8)); // turn pixel off
	}

}


void OLED_UpdateScreen(I2C_HandleTypeDef* hi2c)
{
	// display - 128 x 64
	// loop 8 rows
	for (uint8_t j=0; j<8; j++)
	{
		OLED_WriteCmd(hi2c, 0xB0 + j); // getting ready to draw on row j
		OLED_WriteCmd(hi2c, 0x00);
		OLED_WriteCmd(hi2c, 0x10);

		/* put 128 bytes from the RAM buffer and transmit over I2C */
		HAL_I2C_Mem_Write(hi2c,
						  OLED_I2C_ADDR,
						  0x40,
						  I2C_MEMADD_SIZE_8BIT,
						  &OLED_Buffer[OLED_WIDTH * j],
						  OLED_WIDTH,
						  HAL_MAX_DELAY);
	}
}
