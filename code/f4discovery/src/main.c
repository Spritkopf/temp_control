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

#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include <ds18b20/ds18b20.h>

#include <ssd1306/ssd1306.h>

uint32_t tick = 0;

/* sleep for delay milliseconds */
static void delay(uint32_t delay_msec);
static void discovery_led_setup(void);
static void discovery_button_setup(void);


uint32_t button_flag = 0;

uint32_t sensor_count = 0;


int main(void)
{
    int8_t presence = 0;
    float temp = 1.0;
    char buf[30];

    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    /* set systick for 1ms ticks */
    systick_set_reload(21000);
    systick_clear();
    systick_interrupt_enable();
    systick_counter_enable();

    /* initi f4 discovery button / leds */
    discovery_led_setup();
    discovery_button_setup();

    delay(100);
    ssd1306_init();

    /* init DS18B20 temoerature sensor */
    sensor_count = ds18b20_init();

    ds18b20_set_resolution(0, DS18B20_RES_10B);

    delay(100);
    ssd1306_init();

    ssd1306_set_cursor(0,30);
    sprintf(buf, "Sensors: %i", sensor_count);
    ssd1306_put_str((char*)buf, font_7x10);

    while (1) {
        if(button_flag == 1)
        {
            //button_flag = 0;

            /* when button is pressed, turn LED on for a short time (also serves as debouncing) */
            gpio_set(GPIOD, GPIO12);
            delay(300);
            gpio_clear(GPIOD, GPIO12);


            /* start a measurement */
            ds18b20_start_conversion(0);

            delay(1000);
            temp = ds18b20_get_temperature(0);

            sprintf(buf, "%i.%i C", (int)temp, (int)((temp-(int)temp)*1000));

            ssd1306_set_cursor(0,0);
            ssd1306_put_str((char*)buf, font_7x10);
            ssd1306_update();


            /* place breakpoint here, inspect variable 'temp' */

        }
    } 


    return 0;
}

void sys_tick_handler(void)
{
    tick++;

}

/* sleep for delay milliseconds */
static void delay(uint32_t delay_msec)
{
    uint32_t wake = tick + delay_msec;
    while (wake > tick);
}

static void discovery_led_setup(void)
{
    /* Enable GPIOD clock. */
    rcc_periph_clock_enable(RCC_GPIOD);

    /* Set GPIO12 (in GPIO port D) to 'output push-pull'. */
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
            GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
}

static void discovery_button_setup(void)
{
    /* Enable GPIOA clock. */
    rcc_periph_clock_enable(RCC_GPIOA);

    nvic_enable_irq(NVIC_EXTI0_IRQ);
    nvic_set_priority(NVIC_EXTI0_IRQ, 1);

    /* Set GPIOA0 to 'input floating'. */
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);

    exti_select_source(EXTI0, GPIOA);
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
    exti_enable_request(EXTI0);

}


void exti0_isr(void)
{
    exti_reset_request(EXTI0);
    button_flag = 1;
}

