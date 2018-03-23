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

#ifndef LIB_SSD1306_SSD1306_HAL_H_
#define LIB_SSD1306_SSD1306_HAL_H_

#include <stdint.h>

/*!
 * \file ssd1306_hal.h
 * \brief Hardware abstraction layer for the SSD1306 display driver
 */


/*!
 * \brief Initialize display interface
 * \returns 0 if OK, -1 on initialization error
 */
int8_t ssd1306_hal_init(void);

/*!
 * \brief Send a command to the display controller
 * \param[in] cmd	byte to send
 * \returns 0 if OK, -1 on IO error
 */
int8_t ssd1306_hal_send_command(uint8_t cmd);

/*!
 * \brief Send data to the display controller
 * \param[in] data	pointer to buffer
 * \param[in] len amount of bytes to send
 * \returns 0 if OK, -1 on IO error
 */
int8_t ssd1306_hal_send_data(uint8_t* data, uint32_t len);

/*!
 * \brief Blocking millisecond delay
 * \param[in] delay_ms	amount of milliseconds to wait
 * \returns 0 if OK, -1 on IO error
 */
void ssd1306_hal_delay_ms(uint32_t delay_ms);


#endif /* LIB_SSD1306_SSD1306_HAL_H_ */
