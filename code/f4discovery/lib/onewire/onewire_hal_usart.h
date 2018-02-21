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

#ifndef ONWIRE_HAL_USART_H_
#define ONWIRE_HAL_USART_H_


#include <stdint.h>


/*!
 * \brief Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_init(void);

/*!
 * \brief De-Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_deinit(void);

/*!
 * \brief Reset the line
 * \returns 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_hal_usart_reset_line(void);

/*!
 * \brief Send one bit
 * \param[in] tx_onewire_bit: data to send (1 | 0)
 */
void onewire_hal_usart_send_slot(uint8_t tx_onewire_bit);

/*!
 * \brief Receive one bit
 * \returns the received bit
 */
uint8_t onewire_hal_usart_read_slot(void);

#endif