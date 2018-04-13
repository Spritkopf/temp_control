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

#define DEV_NUM_BROADCAST               255     /* device number for a broadcast to all devices */

/*!
 * \brief Type for setting resolution in config register 
 */
typedef enum _e_resolution
{
    DS18B20_RES_9B   = 0x10,
    DS18B20_RES_10B  = 0x3F,
    DS18B20_RES_11B  = 0x5F,
    DS18B20_RES_12B  = 0x7F,
} ds18b20_resolution_t;

/* 
 * \brief Type holding the DS18B20 scratchpad data 
 */
typedef struct _s_scratchpad
{
    uint8_t temp_lo;        /* temperature data low byte */
    uint8_t temp_hi;        /* temperature data high byte */
    uint8_t tth_lo;         /* temperature threshold low byte */
    uint8_t tth_hi;         /* temperature threshold high byte */
    uint8_t config;         /* configuration register */
    uint8_t crc;             /* CRC of scratchpad */
} ds18b20_scratchpad_t;



/*
 * \brief Type definition for a DS18B20 device
 */
typedef struct _s_ds18b20
{
    uint64_t rom;
    ds18b20_scratchpad_t scratchpad;
} ds18b20_t;



/*! 
 * \brief InitializeDS18B20 temperature sensor
 * \returns the number of found sensors, or 0 if none are present
 */
uint8_t ds18b20_init(void);

/*! 
 * \brief Set resolution of the temperature sensor
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DEV_NUM_BROADCAST for broadcast to all devices)
 * \param[in] resolution: 2-bit value of configuration register setting the resolution, must be of type \ref ds18b20_resolution_t
 * \retval 0  - OK
 * \retval -1 - Data corrupted
 * \retval -2 - illegal device number
 * \details Resolution of the temperature sensor is determined by bits 5 and 6 of configuration register:
 *          0x00 (0000 0000) -> 9 bits   - 93.75 ms  (0.5 degree precision)
 *          0x20 (0010 0000) -> 10 bits  - 187.5 ms  (0.25 degree precision)
 *          0x40 (0100 0000) -> 11 bits  - 375 ms    (0.125 degree precision)
 *          0x60 (0110 0000) -> 12 bits  - 750ms     (0.0625 degree precision)(default)
 *
 * \note The higher the resolution, the longer the conversion time
 */
int8_t ds18b20_set_resolution(uint8_t dev_num, ds18b20_resolution_t resolution);

/*! 
 * \brief Save configuration in EEPROM
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DEV_NUM_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_save_config(uint8_t dev_num);

/*! 
 * \brief Reload configuration from EEPROM
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DEV_NUM_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_load_config(uint8_t dev_num);

/*!
 * \brief Start temperature conversion
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DEV_NUM_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_start_conversion(uint8_t dev_num);

/*! 
 * \brief Read the temperature register
 * \param[in] dev_num: Number of device on bus (counting starts at 0)
 * \param[out] temperature - pointer to buffer for temperature vaue in °C
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_get_temperature(uint8_t dev_num, float* temperature);





#endif
