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


/* display buffer */
static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* display object */
static ssd1306_t display;


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

	/* default color: black background, white foreground */
	display.background = SSD1306_COLOR_BLACK;
	display.foreground = SSD1306_COLOR_WHITE;

	/* Clear screen */
	ssd1306_clear();

	/* send framebuffer to screen */
	ssd1306_update();

	// Set default values for screen object
	display.current_x = 0;
	display.current_y = 0;
}

/*!
 * \brief Set foreground color
 */
void ssd1306_set_foreground(ssd1306_color_t color)
{
	display.foreground = color;
}

/*!
 * \brief Set background color
 */
void ssd1306_set_background(ssd1306_color_t color)
{
	display.background = color;
}

/*!
 * \brief Fill screen with specific color
 * \param[in] color  - color to fill screen with
 */
void ssd1306_fill(ssd1306_color_t color)
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(framebuffer); i++)
	{
		if(color == SSD1306_COLOR_BLACK)
		{
			framebuffer[i] = 0x00;
		}
		else
		{
			framebuffer[i] = 0xFF;
		}
	}
}

/*!
 * \brief Write screenbuffer to device
 */
void ssd1306_update(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		ssd1306_hal_send_command(0xB0 + i);
		ssd1306_hal_send_command(0x00);
		ssd1306_hal_send_command(0x10);

		ssd1306_hal_send_data(&framebuffer[SSD1306_WIDTH * i],SSD1306_WIDTH);
	}
}

/*!
 * \brief Draw one single pixel
 * \param[in] x - x coordinate
 * \param[in] y - y coordinate
 */
void ssd1306_draw_pixel(uint8_t x, uint8_t y, ssd1306_color_t color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
	{
		/* out of range */
		return;
	}

	if (color == SSD1306_COLOR_WHITE)
	{
		framebuffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	}
	else
	{
		framebuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

/*!
 * \brief Draw one character at the current cursor position
 * \param[in] ch - character
 * \param[in] font - font type to use
 * \details The character will be drawn in the foreground color
 */
void ssd1306_put_char(char ch, ssd1306_font_t font)
{
	uint32_t i, b, j;
	ssd1306_color_t color;

	// Check remaining space on current line
	if (SSD1306_WIDTH <= (display.current_x + font.width) ||
		SSD1306_HEIGHT <= (display.current_y + font.height))
	{
		/* Not enough space on current line */
		return;
	}

	// Use the font to write
	for (i = 0; i < font.height; i++)
	{
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++)
		{
			if ((b << j) & 0x8000)
			{
				color = display.foreground;
			}
			else
			{
				color = display.background;
			}
			ssd1306_draw_pixel(display.current_x + j, (display.current_y + i), color);
		}
	}

	display.current_x += font.width;

}

/*!
 * \brief Draw a string at the current cursor position
 * \param[in] str - character array
 * \param[in] font - font type to use
 * \details The character will be drawn in the foreground color
 */
void ssd1306_put_str(char* str, ssd1306_font_t font)
{
	// Write until null-byte
	while (*str)
	{
		ssd1306_put_char(*str, font);

		str++;
	}

}

//
//	Position the cursor
//
void ssd1306_set_cursor(uint8_t x, uint8_t y)
{
	display.current_x = x;
	display.current_y = y;
}

/*!
 * \brief clears the sdreen by filling with the background color
 */
void ssd1306_clear(void)
{
	ssd1306_fill(display.background);
}

/*!
 * \brief Invert display
 * \param[in] invert - 1=inverted ; 0=normal
 */
void ssd1306_invert(uint8_t invert)
{
  if (invert == 1) {
	  ssd1306_hal_send_command(0xA7);	/* inverted mode */
  } else {
	  ssd1306_hal_send_command(0xA6);	/* normal mode */
  }
}



