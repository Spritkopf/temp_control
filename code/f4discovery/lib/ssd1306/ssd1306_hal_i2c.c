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

/*!
 * \file ssd1306_hal.h
 * \brief I2C Hardware abstraction layer for the SSD1306 display driver
 */

#include <ssd1306/ssd1306_hal.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#define SSD1306_I2C_INSTANCE		I2C1
#define SSD1306_I2C_PERIPH_CLK		RCC_I2C1
#define SSD1306_I2C_GPIO_CLK		RCC_GPIOB
#define SSD1306_I2C_GPIO_PORT		GPIOB
#define SSD1306_I2C_GPIO_SCL_PIN	GPIO6
#define SSD1306_I2C_GPIO_SDA_PIN	GPIO7
#define SSD1306_I2C_GPIO_AF			GPIO_AF4

#define SSD1306_I2C_ADDR			0x3C  // left shifted: 0xF0  // pcb: 0x78

void (*delay_ms_cb)(uint32_t delay_ms);
/*!
 * \brief Initialize display interface (i2c)
 * \returns 0 if OK, -1 on initialization error
 */
int8_t ssd1306_hal_init(void)
{
	rcc_periph_clock_enable(SSD1306_I2C_PERIPH_CLK);
	rcc_periph_clock_enable(SSD1306_I2C_GPIO_CLK);

	i2c_reset(SSD1306_I2C_INSTANCE);

	/* Setup GPIO pins for I2C peripheral */
	gpio_mode_setup(SSD1306_I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
			SSD1306_I2C_GPIO_SCL_PIN | SSD1306_I2C_GPIO_SDA_PIN);
	gpio_set_af(SSD1306_I2C_GPIO_PORT, SSD1306_I2C_GPIO_AF,
			SSD1306_I2C_GPIO_SCL_PIN | SSD1306_I2C_GPIO_SDA_PIN);

	i2c_set_speed(SSD1306_I2C_INSTANCE, i2c_speed_fm_400k, rcc_apb1_frequency/1000000);

	i2c_peripheral_enable(SSD1306_I2C_INSTANCE);



	return (0);
}

/*!
 * \brief Send a command to the display controller
 * \param[in] cmd	byte to send
 * \returns 0 if OK, -1 on IO error
 */
int8_t ssd1306_hal_send_command(uint8_t cmd)
{
	uint8_t tx_data[2];

	tx_data[0] = 0x00;	/* write to mem Addr 0 */
	tx_data[1] = cmd;	/* data byte */

	/* send data */
	i2c_transfer7(SSD1306_I2C_INSTANCE, SSD1306_I2C_ADDR, tx_data, 2, NULL, 0);

	return (0);
}
/*!
 * \brief Send data to the display controller
 * \param[in] data	pointer to buffer
 * \param[in] len amount of bytes to send
 * \returns 0 if OK, -1 on IO error
 */
int8_t ssd1306_hal_send_data(uint8_t* data, uint32_t len)
{
	uint8_t tx_data[len+1];
	uint32_t i;

	tx_data[0] = 0x40;  /* memory address to write to */

	for (i = 0; i < len; i++) {
		tx_data[i+1] = data[i];
	}

	/* send data */
	i2c_transfer7(SSD1306_I2C_INSTANCE, SSD1306_I2C_ADDR, tx_data, len+1, NULL, 0);

	return (0);
}


/*!
 * \brief Blocking millisecond delay
 * \param[in] delay_ms	amount of milliseconds to wait
 * \returns 0 if OK, -1 on IO error
 */
void ssd1306_hal_delay_ms(uint32_t delay_ms)
{
	if(delay_ms_cb != NULL)
	{
		delay_ms_cb(delay_ms);
	}
}
