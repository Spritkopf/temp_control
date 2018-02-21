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

#include <onewire/onewire.h>
#include <onewire/onewire_hal_usart.h>

void onewire_write_bit(uint8_t tx_bit);
uint8_t onewire_read_bit(void);


/*!
 * \brief Init 1-Wire bus master 
 */
void onewire_init(void)
{
    onewire_hal_usart_init();
}

/*!
 * \brief Send 1-Wire reset pulse
 * \returns: presence - 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_reset(void)
{
    uint8_t presence = 0;

    presence = onewire_hal_usart_reset_line();

    return (presence);
    
}

/*!
 * \brief transfer one byte
 * \param[in] tx_byte: data to send
 */
void onewire_send_byte(uint8_t tx_byte)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        /* write bit to 1-Wire bus */
        onewire_write_bit((tx_byte >> i) & 0x01);
    }
}

/*!
 * \brief receive one byte
 * \returns answer byte
 */
uint8_t onewire_receive_byte(void)
{
    uint8_t i;
    uint8_t rx_byte = 0;
    uint8_t rx_bit = 0;

     for(i = 0; i < 8; i++)
    {
        rx_bit = onewire_read_bit();
        
        rx_byte |= rx_bit << i; 
    }

    return (rx_byte);
}





/******************************************************************
* BEGIN OF STATIC FUNCTIONS
******************************************************************/

/*!
 * \brief Issue a 1-Wire Write slot (1 | 0) on the bus
 * \param[in] tx_bit: The bit to send (1 or 0)
 */
void onewire_write_bit(uint8_t tx_bit)
{
    onewire_hal_usart_send_slot(tx_bit);
}

/*!
 * \brief Issue a 1-Wire Read slot on the bus and return the answer bit
 * \returns answer bit (1 or 0)
 */
uint8_t onewire_read_bit(void)
{
    return (onewire_hal_usart_read_slot());
}
