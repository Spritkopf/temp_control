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

#include <ds18b20/ds18b20.h>
#include <onewire/onewire.h>

#define DS18B20_CMD_ROM_SEARCH          0xF0    /* get ROM information about devices on the bus */
#define DS18B20_CMD_ROM_READ            0x33    /* read rom code of device (ONLY if one device is on bus) */
#define DS18B20_CMD_ROM_MATCH           0x55    /* access one specific device on the bus */
#define DS18B20_CMD_ROM_SKIP            0xCC    /* access all devices present on the bus simultaneously */
#define DS18B20_CMD_ALARM_SEARCH        0xEC    /* look for devces which are in alarm state */
#define DS18B20_CMD_CONVERT             0x44    /* start a temperature conversion */
#define DS18B20_CMD_SCRATCHPAD_WRITE    0x4E    /* write scratchpad RAM */
#define DS18B20_CMD_SCRATCHPAD_READ     0xBE    /* read scratchpad RAM */
#define DS18B20_CMD_SCRATCHPAD_COPY     0x48    /* copy data from scratchpad RAM to EEPROM */
#define DS18B20_CMD_EEPROM_RECALL       0xB8    /* reload data from EEPROM to scratchpad RAM */
#define DS18B20_CMD_READ_POWER_SUPPLY   0xB4    /* determine if device is in parasitic power mode */

#define DS18B20_SCRATCHPAD_IDX_TEMP_L   0x00    /* Temperature conversion result low byte */
#define DS18B20_SCRATCHPAD_IDX_TEMP_H   0x01    /* Temperature conversion result high byte */
#define DS18B20_SCRATCHPAD_IDX_ALERT_H  0x02    /* Alert register high byte */
#define DS18B20_SCRATCHPAD_IDX_ALERT_L  0x03    /* Alert register low byte */
#define DS18B20_SCRATCHPAD_IDX_CONFIG   0x04    /* Configuration register */
#define DS18B20_SCRATCHPAD_IDX_CRC      0x08    /* Scratchpad CRC */

/* static declarations */
static void ds18b20_send_command(uint8_t cmd);
static void ds18b20_scratchpad_write(uint8_t alert_l, uint8_t alert_h, uint8_t config);
static void ds18b20_scratchpad_read(uint8_t* buffer, uint8_t len);



/* =================================================================== */


/*! 
 * \brief InitializeDS18B20 temperature sensor
 * \retval 0  - OK
 * \retval -1 - No device found on bus
 */
int8_t ds18b20_init(void)
{
    uint8_t presence = 0;

    onewire_init();

    presence = onewire_reset();

    if(presence == 1)
    {
        return (0);
    }
    else
    {
        return(-1);
    }
}


/*! 
 * \brief Set resolution of the temperature sensor
 * \param[in] resolution: 2-bit value of configuration register setting the resolution, must be of type \ref ds18b20_resolution_t
 * \retval 0  - OK
 * \retval -1 - Data corrupted
 * \details Resolution of the temperature sensor is determined by bits 5 and 6 of configuration register:
 *          0x00 (0000 0000) -> 9 bits   - 93.75 ms  (0.5 degree precision)
 *          0x20 (0010 0000) -> 10 bits  - 187.5 ms  (0.25 degree precision)
 *          0x40 (0100 0000) -> 11 bits  - 375 ms    (0.125 degree precision)
 *          0x60 (0110 0000) -> 12 bits  - 750ms     (0.0625 degree precision)(default)
 *
 * \note The higher the resolution, the longer the conversion time
 */
int8_t ds18b20_set_resolution(ds18b20_resolution_t resolution)
{
    uint8_t scratchpad_buffer[9] = {0};

    /* write config register  to scratchpad */
    ds18b20_scratchpad_write(0xAA, 0x87, resolution);

    /* read back scratchpad to ensure data integrity */
    ds18b20_scratchpad_read(scratchpad_buffer, 9);    

    if(scratchpad_buffer[DS18B20_SCRATCHPAD_IDX_CONFIG] != resolution)
    {
        return (-1);
    } 

    return (0);

}

/*! 
 * \brief Save configuration in EEPROM
 */
void ds18b20_save_config(void)
{
    ds18b20_send_command(DS18B20_CMD_SCRATCHPAD_COPY);
}

/*! 
 * \brief Reload configuration from EEPROM
 */
void ds18b20_load_config(void)
{
    ds18b20_send_command(DS18B20_CMD_EEPROM_RECALL);
}

/*! 
 * \brief Start temperature conversion
 */
void ds18b20_start_conversion(void)
{
    ds18b20_send_command(DS18B20_CMD_CONVERT);   
}

/*! 
 * \brief Read the temperature register
 * \param[out] temperature: buffer for temperature value
 * \returns temperature
 */
float ds18b20_get_temperature(void)
{
    uint8_t scratchpad_buffer[DS18B20_SCRATCHPAD_IDX_CONFIG+1];
    float temperature = 0.0f;
    int16_t temp_raw_value;
    uint8_t i;
    uint8_t resolution_bits;
    
    /* read scratchpad until config register*/
    ds18b20_scratchpad_read(scratchpad_buffer, DS18B20_SCRATCHPAD_IDX_CONFIG+1);   

    temp_raw_value = (int16_t)((uint16_t)scratchpad_buffer[1] << 8) | scratchpad_buffer[0];

    /* clear least significant bits for lower resolutions, because they may be undefined (see datasheet) */
    resolution_bits = (scratchpad_buffer[DS18B20_SCRATCHPAD_IDX_CONFIG] & 0x60) >> 5;
    
    for(i = 0; i < 3-resolution_bits; i++)    
    {
        temp_raw_value &= ~(0x01 << i);
    }

    /* calculate temperature in deg C */
    temperature = ((float)temp_raw_value) / 16.0f;

    return (temperature);

}


/******************************************************************
* BEGIN OF STATIC FUNCTIONS
******************************************************************/

/*!
 * \brief Send command byte to device
 */
static void ds18b20_send_command(uint8_t cmd)
{
    onewire_reset();

    onewire_send_byte(DS18B20_CMD_ROM_SKIP);
    onewire_send_byte(cmd);
}

/*!
 * \brief Write data to scratchpad RAM
 * \param[in] alert_h Value of alert register T_H
 * \param[in] alert_l Value of alert register T_L
 * \param[in] config Value of config register
 */
static void ds18b20_scratchpad_write(uint8_t alert_h, uint8_t alert_l, uint8_t config)
{
    ds18b20_send_command(DS18B20_CMD_SCRATCHPAD_WRITE);
    onewire_send_byte(alert_h);
    onewire_send_byte(alert_l);
    onewire_send_byte(config);

    //onewire_reset();
}

/*!
 * \brief Read data from scratchpad RAM
 * \param[out] buffer pointer to data buffer to read the data into
 * \param[in] len Amount of bytes to read
 */
static void ds18b20_scratchpad_read(uint8_t* buffer, uint8_t len)
{
    uint8_t i;

    if(len > 0)
    {
        if(len > 9)
        {
            len = 9;
        }

        ds18b20_send_command(DS18B20_CMD_SCRATCHPAD_READ);

        for(i = 0; i < len; i++)
        {
            buffer[i] = onewire_receive_byte();
        }

        //onewire_reset();
    }  
}
