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

#ifndef ONWIRE_H_
#define ONWIRE_H_


#include <stdint.h>


/*!
 * \brief Init 1-Wire bus master 
 */
void onewire_init(void);

/*!
 * \brief Send 1-Wire reset pulse
 * \returns presence: 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_reset(void);

/*!
 * \brief transmit one byte
 * \param[in] tx_byte: data to send
 */
void onewire_send_byte(uint8_t tx_byte);


/*!
 * \brief receive one byte
 * \returns answer byte
 */
uint8_t onewire_receive_byte(void);



#endif