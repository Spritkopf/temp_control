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


#define DS18B20_ROM_FAMILY_CODE         0x28    /* family code of DS18B20 devices */

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

#define MAX_DEVICES                     64      /* allow a maximum of 64 devices on the bus */


/* static declarations */
static int8_t ds18b20_send_command(ds18b20_t* device, uint8_t cmd);
static int8_t ds18b20_scratchpad_write(ds18b20_t* device, uint8_t alert_l, uint8_t alert_h, uint8_t config);
static int8_t ds18b20_scratchpad_read(ds18b20_t* device, uint8_t* buffer, uint8_t len);
static uint8_t ds18b20_rom_search(void);

/* this list holds the objects for each found device. 64 sensors are supported by default */
ds18b20_t ds18b20_device_list[MAX_DEVICES] = {0};
uint8_t ds18b20_device_count = 0;

/* =================================================================== */


/*! 
 * \brief Initialize DS18B20 temperature sensor
 * \returns the number of found sensors, or 0 if none are present
 */
uint8_t ds18b20_init(void)
{
    uint8_t presence = 0;

    onewire_init();

    presence = onewire_reset();

    if(presence == 1)
    {
        /* perform ROM search */
        return (ds18b20_rom_search());
    }
    else
    {
        return(0);
    }
}

/*! 
 * \brief Set resolution of the temperature sensor
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DS18B20_BROADCAST for broadcast to all devices)
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
int8_t ds18b20_set_resolution(uint8_t dev_num, ds18b20_resolution_t resolution)
{
    uint8_t scratchpad_buffer[9] = {0};
    ds18b20_t* device;

    if(dev_num >= ds18b20_device_count)
    {
        return (-2);
    }

    device = &(ds18b20_device_list[dev_num]);

    /* write config register  to scratchpad */
    ds18b20_scratchpad_write(device, 0xAA, 0x87, resolution);

    /* read back scratchpad to ensure data integrity */
    ds18b20_scratchpad_read(device, scratchpad_buffer, 9);    

    if(scratchpad_buffer[DS18B20_SCRATCHPAD_IDX_CONFIG] != resolution)
    {
        return (-1);
    } 

    return (0);

}

/*! 
 * \brief Save configuration in EEPROM
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DS18B20_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_save_config(uint8_t dev_num)
{
    return (ds18b20_send_command(dev_num, DS18B20_CMD_SCRATCHPAD_COPY));
}

/*! 
 * \brief Reload configuration from EEPROM
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DS18B20_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_load_config(uint8_t dev_num)
{
    return (ds18b20_send_command(dev_num, DS18B20_CMD_EEPROM_RECALL));
}

/*!
 * \brief Start temperature conversion
 * \param[in] dev_num: Number of device on bus (counting starts at 0, use DS18B20_BROADCAST for broadcast to all devices)
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_start_conversion(uint8_t dev_num)
{
    return (ds18b20_send_command(dev_num, DS18B20_CMD_CONVERT));
}

/*! 
 * \brief Read the temperature register
 * \param[in] dev_num: Number of device on bus (counting starts at 0)
 * \param[out] temperature - temperature vaue in °C
 * \retval 0  - OK
 * \retval -1 - illegal device number
 */
int8_t ds18b20_get_temperature(uint8_t dev_num, float* temperature)
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
 * \returns 0 if OK, -1 if illegal device number
 */
static int8_t ds18b20_send_command(uint8_t dev_num, uint8_t cmd)
{
    uint64_t rom;

    if((dev_num >= ds18b20_device_count) && (dev_num != DEV_NUM_BROADCAST))
    {
        return (-1);
    }

    onewire_reset();
    
    /* only issue ROM commands if this is not a rom search */
    if (cmd != DS18B20_CMD_ROM_SEARCH)
    {
        if(dev_num == DEV_NUM_BROADCAST)
        {
            /* for broadcast commands, skip ROM */
            onewire_send_byte(DS18B20_CMD_ROM_SKIP);
        }
        else
        {
            uint8_t i = 0;
            rom = ds18b20_device_list[dev_num].rom;
            
            /* for broadcast commands, skip ROM */
            onewire_send_byte(DS18B20_CMD_ROM_MATCH);

            for(i = 0; i < 8; i++)
            {
                /* send rom code byte by byte */
                onewire_send_byte((rom >> i*8) & 0xFF);
            }

        }
    }

    /* send the actual command */
    onewire_send_byte(cmd);

    return (0);
}

/*!
 * \brief Write data to scratchpad RAM
 * \param[in] alert_h Value of alert register T_H
 * \param[in] alert_l Value of alert register T_L
 * \param[in] config Value of config register
 * \returns 0 if OK, -1 if illegal device number
 */
static int8_t ds18b20_scratchpad_write(uint8_t dev_num, uint8_t alert_h, uint8_t alert_l, uint8_t config)
{
    if(ds18b20_send_command(dev_num, DS18B20_CMD_SCRATCHPAD_WRITE) == 0)
    {
        onewire_send_byte(alert_h);
        onewire_send_byte(alert_l);
        onewire_send_byte(config);

        return (0);
    }
    else
    {
        return (-1);
    }
}

/*!
 * \brief Read data from scratchpad RAM
 * \param[out] buffer - pointer to data buffer to read the data into
 * \param[in] len - amount of bytes to read
 * \retval 0  - OK
 * \retval -1 - illegal device number
 * \retval -2 - invalid length
 */
static int8_t ds18b20_scratchpad_read(uint8_t dev_num, uint8_t* buffer, uint8_t len)
{
    uint8_t i;

    if(dev_num >= ds18b20_device_count)
    {
        /* broadcast only allowed if there is only one device on the bus */
        if (!((dev_num == DEV_NUM_BROADCAST) && (ds18b20_device_count == 1)))
        {
            return (-1);
        }
    }
    
    if(len > 0)
    {
        if(len > 9)
        {
            len = 9;
        }

        if (ds18b20_send_command(dev_num, DS18B20_CMD_SCRATCHPAD_READ) != 0)
        {
            return (-1);
        }   

        for(i = 0; i < len; i++)
        {
            buffer[i] = onewire_receive_byte();
        }
    }  
    else
    {
        return (-2);
    }

    return (0);
}

/*!
 * \brief Perform a 1-Wire ROM search
 * \returns Number of found DS18B20 sensors
 * \details Saves the ROM codes of found devices in device list. 
 *          The search algorithm is derived from Maxim AppNote 187 https://www.maximintegrated.com/en/app-notes/index.mvp/id/187
 */
static int8_t ds18b20_rom_search(void)
{
    int8_t last_discrepancy = -1;
    int8_t last_zero = -1;
    uint64_t rom_buffer = 0;
    uint8_t search_direction = 0;
    uint8_t last_device_flag = 0;
    uint8_t current_bit_pos = 0;
    uint8_t current_bit;
    uint8_t current_bit_comp;
    /* begin search algorithm */

    /* reset all slaves and send ROM_SEARCH command*/
    ds18b20_send_command(DEV_NUM_BROADCAST, DS18B20_CMD_ROM_SEARCH);

    while(last_device_flag == 0)
    {
        current_bit_pos = 0;
        last_zero = -1;

        while(current_bit_pos < 64)
        {
            /* read bit and complement from bus */
            current_bit = onewire_read_bit();
            current_bit_comp = onewire_read_bit();

            if((current_bit == 1) && (current_bit_comp == 1))
            {
                /* error, no activity on bus */
                return (-1);
            }
            else
            if((current_bit == 0) && (current_bit_comp == 1))
            {
                /* the received bit was 0 */
                search_direction = 0;
            }
            else
            if((current_bit == 1) && (current_bit_comp == 0))
            {
                /* the received bit was 1 */
                search_direction = 1;
            }
            else
            if((current_bit == 0) && (current_bit_comp == 0))
            {
                /* both 0 and 1 was received */
                if (current_bit_pos == last_discrepancy)
                {
                    search_direction = 1;
                }
                else
                if (current_bit_pos > last_discrepancy)
                {
                    search_direction = 0;
                    last_discrepancy = current_bit_pos;
                }
                else
                {   
                    search_direction = ((rom_buffer & (0x01 << current_bit_pos)) >> current_bit_pos);
                }
                
                if (search_direction == 0)
                {
                    last_zero = current_bit_pos;
                }
            }

            rom_buffer &= ~(0x01 << current_bit_pos);
            rom_buffer |= (search_direction << current_bit_pos);
            onewire_write_bit(search_direction);
            current_bit++;
        }

        /* device found, store ROM-code in list only if it is a DS18B20 temprature sensor*/
        if((rom_buffer & 0xFF) == DS18B20_ROM_FAMILY_CODE)
        {
            ds18b20_device_list[ds18b20_device_count].rom = rom_buffer;
            ds18b20_device_count++;
        }

        last_discrepancy = last_zero;

        if(last_discrepancy == -1)
        {
            /* no more unknown devices on the bus, search complete */
            last_device_flag = 1;
        }
    }

    return (ds18b20_device_count);
}   
