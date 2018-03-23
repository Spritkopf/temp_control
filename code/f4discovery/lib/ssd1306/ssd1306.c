/*
 * Copyright (c) 2018 Ricardo Beck.
 *
 * This file is part of temp_control
 * (see https://github.com/Spritkopf/temp_control).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <ssd1306/ssd1306.h>
#include <ssd1306/ssd1306_hal.h>


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;

static void init_seq()
{

}



//
//	Initialize the oled screen
//
void ssd1306_init(void)
{
	ssd1306_hal_init();
	/* Init LCD */

	ssd1306_hal_send_command(0xAE); //display off
	ssd1306_hal_send_command(0x20); //Set Memory Addressing Mode
	ssd1306_hal_send_command(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	ssd1306_hal_send_command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	ssd1306_hal_send_command(0xC8); //Set COM Output Scan Direction
	ssd1306_hal_send_command(0x00); //---set low column address
	ssd1306_hal_send_command(0x10); //---set high column address
	ssd1306_hal_send_command(0x40); //--set start line address
	ssd1306_hal_send_command(0x81); //--set contrast control register
	ssd1306_hal_send_command(0xFF);
	ssd1306_hal_send_command(0xA1); //--set segment re-map 0 to 127
	ssd1306_hal_send_command(0xA6); //--set normal display
	ssd1306_hal_send_command(0xA8); //--set multiplex ratio(1 to 64)
	ssd1306_hal_send_command(0x3F); //
	ssd1306_hal_send_command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	ssd1306_hal_send_command(0xD3); //-set display offset
	ssd1306_hal_send_command(0x00); //-not offset
	ssd1306_hal_send_command(0xD5); //--set display clock divide ratio/oscillator frequency
	ssd1306_hal_send_command(0xF0); //--set divide ratio
	ssd1306_hal_send_command(0xD9); //--set pre-charge period
	ssd1306_hal_send_command(0x22); //
	ssd1306_hal_send_command(0xDA); //--set com pins hardware configuration
	ssd1306_hal_send_command(0x12);
	ssd1306_hal_send_command(0xDB); //--set vcomh
	ssd1306_hal_send_command(0x20); //0x20,0.77xVcc
	ssd1306_hal_send_command(0x8D); //--set DC-DC enable
	ssd1306_hal_send_command(0x14); //
	ssd1306_hal_send_command(0xAF); //--turn on SSD1306 panel
#if 0
	ssd1306_hal_send_command(0xAE);
	ssd1306_hal_send_command(0xD5);
	ssd1306_hal_send_command(0xA8);
	ssd1306_hal_send_command(SSD1306_HEIGHT - 1);
	ssd1306_hal_send_command(0xD3);
	ssd1306_hal_send_command(0x0);
	ssd1306_hal_send_command(0x40 | 0x0);
	ssd1306_hal_send_command(0x8D);  									// chargepump????
	ssd1306_hal_send_command(0x10);										// externalvcc
	ssd1306_hal_send_command(0x20);
	ssd1306_hal_send_command(0x00);
	ssd1306_hal_send_command(0xA0 | 0x1);
	ssd1306_hal_send_command(0xC8);
	ssd1306_hal_send_command(0xDA);
	ssd1306_hal_send_command(0x12);
	ssd1306_hal_send_command(0x81);
	ssd1306_hal_send_command(0x9F);				// externalvcc
	ssd1306_hal_send_command(0xd9);
	ssd1306_hal_send_command(0x22);
	ssd1306_hal_send_command(0xDB);
	ssd1306_hal_send_command(0x40);
	ssd1306_hal_send_command(0xA4);
	ssd1306_hal_send_command(0xA6);
	ssd1306_hal_send_command(0x2E);
	ssd1306_hal_send_command(0xAF);
#endif
	// Clear screen
	ssd1306_Fill(SSD1306_COLOR_BLACK);

	// Flush buffer to screen
	ssd1306_UpdateScreen();

	// Set default values for screen object
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;

	SSD1306.Initialized = 1;

}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(SSD1306_Buffer); i++)
	{
		SSD1306_Buffer[i] = (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF;
	}
}

//
//  Write the screenbuffer with changed to the screen
//
void ssd1306_UpdateScreen(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		ssd1306_hal_send_command(0xB0 + i);
		ssd1306_hal_send_command(0x00);
		ssd1306_hal_send_command(0x10);

		ssd1306_hal_send_data(&SSD1306_Buffer[SSD1306_WIDTH * i],SSD1306_WIDTH);
	}
}

//
//	Draw one pixel in the screenbuffer
//	X => X Coordinate
//	Y => Y Coordinate
//	color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
	{
		// Don't write outside the buffer
		return;
	}

	// Check if pixel should be inverted
	if (SSD1306.Inverted)
	{
		color = (SSD1306_COLOR)!color;
	}

	// Draw in the right color
	if (color == SSD1306_COLOR_WHITE)
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	}
	else
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

//
//  Draw 1 char to the screen buffer
//	ch 		=> char om weg te schrijven
//	Font 	=> Font waarmee we gaan schrijven
//	color 	=> Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
	uint32_t i, b, j;

	// Check remaining space on current line
	if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
	{
		// Not enough space on current line
		return 0;
	}

	// Use the font to write
	for (i = 0; i < Font.FontHeight; i++)
	{
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for (j = 0; j < Font.FontWidth; j++)
		{
			if ((b << j) & 0x8000)
			{
				ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
			}
			else
			{
				ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
			}
		}
	}

	// The current space is now taken
	SSD1306.CurrentX += Font.FontWidth;

	// Return written char for validation
	return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color)
{
	// Write until null-byte
	while (*str)
	{
		if (ssd1306_WriteChar(*str, Font, color) != *str)
		{
			// Char could not be written
			return *str;
		}

		// Next char
		str++;
	}

	// Everything ok
	return *str;
}

//
//	Position the cursor
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}
