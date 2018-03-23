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

#ifndef LIB_SSD1306_SSD1306_H_
#define LIB_SSD1306_SSD1306_H_



#include <ssd1306/ssd1306_hal.h>
#include <ssd1306/fonts.h>

/*
 * This library uses 2 extra files (fonts.c/h).
 * In this files are 3 different fonts you can use:
 * 		- font_7x10
 * 		- font_11x18
 * 		- font_16x26
 */

#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /* pixel not lit -> black */
	SSD1306_COLOR_WHITE = 0x01  /* pixel lit -> white */
} ssd1306_color_t;

typedef struct {
	ssd1306_color_t background;   /* background color */
	ssd1306_color_t foreground;	  /* foreground color (text etc) */
	uint16_t current_x;
	uint16_t current_y;
} ssd1306_t;



void ssd1306_init(void);
void ssd1306_set_foreground(ssd1306_color_t color);
void ssd1306_set_background(ssd1306_color_t color);
void ssd1306_fill(ssd1306_color_t color);
void ssd1306_update(void);
void ssd1306_draw_pixel(uint8_t x, uint8_t y, ssd1306_color_t color);
void ssd1306_put_char(char ch, ssd1306_font_t font);
void ssd1306_put_str(char* str, ssd1306_font_t font);
void ssd1306_set_cursor(uint8_t x, uint8_t y);

void ssd1306_clear(void);
void ssd1306_invert(uint8_t invert);
#endif /* LIB_SSD1306_SSD1306_H__ */
