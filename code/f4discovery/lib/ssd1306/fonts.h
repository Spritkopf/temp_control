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

#ifndef LIB_SSD1306_FONTS_H_
#define LIB_SSD1306_FONTS_H_

#include <stdint.h>
/*!
 * \brief struct for font definition
 */
typedef struct {
	uint8_t width;    /*!< Font width in pixels */
	uint8_t height;   		/*!< Font height in pixels */
	const uint16_t *data; 		/*!< Pointer to data font data array */
} ssd1306_font_t;


extern ssd1306_font_t font_7x10;
extern ssd1306_font_t font_11x18;
extern ssd1306_font_t font_16x26;


#endif /* LIB_SSD1306_FONTS_H_ */
