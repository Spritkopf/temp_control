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

#ifndef DS18B20_H_
#define DS18B20_H_

#include <stdint.h>


/*!
 * \brief Type for setting resolution in config register 
 */
typedef enum
{
    DS18B20_RES_9B   = 0x10,
    DS18B20_RES_10B  = 0x3F,
    DS18B20_RES_11B  = 0x5F,
    DS18B20_RES_12B  = 0x7F,
} ds18b20_resolution_t;

/*! 
 * \brief InitializeDS18B20 temperature sensor
 * \retval 0  - OK
 * \retval -1 - No device found on bus
 */
int8_t ds18b20_init(void);

/*! 
 * \brief Set resolution of the temperature sensor
 * \param[in] resolution: 2-bit value of configuration register setting the resolution, must be of type \ref ds18b20_resolution_t
 * \retval 0  - OK
 * \retval -1 - No device found on bus
 * \details Resolution of the temperature sensor is determined by bits 0 and 1 of configuration register:
 *          0x00 (00) -> 9 bits   - 93.75 ms
 *          0x01 (01) -> 10 bits  - 187.5 ms
 *          0x02 (10) -> 11 bits  - 375 ms
 *          0x03 (11) -> 12 bits  - 750ms (default)
 *
 * \note The higher the resolution, the longer the conversion time
 */
int8_t ds18b20_set_resolution(ds18b20_resolution_t resolution);

/*! 
 * \brief Save configuration in EEPROM
 */
void ds18b20_save_config(void);

/*! 
 * \brief Reload configuration from EEPROM
 */
void ds18b20_load_config(void);

/*!
 * \brief Start temperature conversion
 */
void ds18b20_start_conversion(void);


/*! 
 * \brief Read the temperature register
 * \param[out] temperature: buffer for temperature value
 * \returns temperature
 */
float ds18b20_get_temperature(void);





#endif
