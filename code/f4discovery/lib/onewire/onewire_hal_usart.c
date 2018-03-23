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

#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <onewire/onewire_hal_usart.h>

#define USART_INSTANCE          USART2
#define USART_BAUDRATE_RESET    9600            /* baudrate for onewire reset cmd */
#define USART_BAUDRATE_COMM     115200          /* baudrate for normal onewire communication */
#define USART_PERIPH_CLK        RCC_USART2
#define USART_IRQ               NVIC_USART2_IRQ 

#define USART_GPIO_PIN_TX       GPIO2
#define USART_GPIO_PIN_RX       GPIO3
#define USART_GPIO_PORT         GPIOA
#define USART_GPIO_PORT_CLK     RCC_GPIOA

#define ONEWIRE_RESET_PULSE     0xF0            /* 0xF0 represents a 1-Wire reset pulse @ 9600 Baudrate */
#define ONEWIRE_READ_TIMEOUT    10000           /* timeout for blocking uart read */
#define ONEWIRE_READ_SLOT       0xFF            
#define ONEWIRE_WRITE_SLOT_1    0xFF            
#define ONEWIRE_WRITE_SLOT_0    0x00

static void onewire_hal_usart_setup(uint32_t baudrate);
static uint8_t onewire_hal_usart_byte_to_bit(uint8_t input_byte);
static void onewire_hal_usart_send(uint8_t tx_data_byte);
static uint8_t onewire_hal_usart_read(void);


uint16_t receive_buffer = 0;
uint8_t receive_flag = 0;

/*!
 * \brief Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_init(void)
{
    rcc_periph_clock_enable(USART_GPIO_PORT_CLK);
    rcc_periph_clock_enable(USART_PERIPH_CLK);

    /* set USART GPIO pins to output alternate function, open drain) */
    gpio_mode_setup(USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_GPIO_PIN_TX | USART_GPIO_PIN_RX);
    gpio_set_output_options(USART_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, USART_GPIO_PIN_TX | USART_GPIO_PIN_RX);

    /* select proper alternate function mapping for USARt GPIO pins */
    gpio_set_af(USART_GPIO_PORT, GPIO_AF7, USART_GPIO_PIN_TX | USART_GPIO_PIN_RX);

    /* configure USART peripheral in Onewire half-duplex mode */
    usart_set_databits(USART_INSTANCE, 8);
    usart_set_stopbits(USART_INSTANCE, USART_STOPBITS_1);
    usart_set_mode(USART_INSTANCE, USART_MODE_TX_RX);
    usart_set_parity(USART_INSTANCE, USART_PARITY_NONE);
    usart_set_flow_control(USART_INSTANCE, USART_FLOWCONTROL_NONE);

    /* enable USART half duplex mode, raw register access because it's not supported in libopencm3 yet */
    USART_CR2(USART_INSTANCE) &= ~USART_CR2_LINEN;
    USART_CR2(USART_INSTANCE) &= ~USART_CR2_CLKEN;
    USART_CR3(USART_INSTANCE) &= ~USART_CR3_SCEN;
    USART_CR3(USART_INSTANCE) &= ~USART_CR3_IREN;
    USART_CR3(USART_INSTANCE) |= USART_CR3_HDSEL;

    usart_enable_rx_interrupt(USART_INSTANCE);
    nvic_enable_irq(USART_IRQ);

    /* set baudrate and enable usart peripheral */
    onewire_hal_usart_setup(USART_BAUDRATE_RESET);
}

/*!
 * \brief De-Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_deinit(void)
{
    nvic_disable_irq(USART_IRQ);
    usart_disable(USART_INSTANCE);

}

/*!
 * \brief Reset the line
 * \returns 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_hal_usart_reset_line(void)
{
    uint8_t result = 0;
    uint8_t rx_usart_byte = 0;

    /* re-configure USART baudrate to match 1-Wire Reset-Pulse requirements */
    onewire_hal_usart_setup(USART_BAUDRATE_RESET);

    /* transmit raw value for "reset pulse" */
    onewire_hal_usart_send(ONEWIRE_RESET_PULSE);      

    /* read raw usart value which represents one bit */
    rx_usart_byte = onewire_hal_usart_read();

    onewire_hal_usart_setup(USART_BAUDRATE_COMM);    

    if(rx_usart_byte == ONEWIRE_RESET_PULSE)
    {
        /* no device present */
        result = 0;
    }
    else
    {
        result = 1;   
    }
    return (result);
}


/*!
 * \brief Send one bit
 * \param[in] tx_onewire_bit: data to send (1 | 0)
 */
void onewire_hal_usart_send_slot(uint8_t tx_onewire_bit)
{
    uint8_t tx_byte;
    
    if(tx_onewire_bit == 1)
    {
        tx_byte = ONEWIRE_WRITE_SLOT_1;
    }
    else
    if(tx_onewire_bit == 0)
    {
        tx_byte = ONEWIRE_WRITE_SLOT_0;
    }
    
    onewire_hal_usart_send(tx_byte);
}

/*!
 * \brief Receive one bit
 * \returns the received bit
 */
uint8_t onewire_hal_usart_read_slot(void)
{
    uint8_t rx_usart_byte;
    uint8_t rx_bit;

    onewire_hal_usart_send(ONEWIRE_READ_SLOT);

    /* read raw usart value which represents one bit */
    rx_usart_byte = onewire_hal_usart_read();

    /* convert USART value to onewire bit */
    rx_bit = onewire_hal_usart_byte_to_bit(rx_usart_byte);

    return (rx_bit);
}

/******************************************************************
* BEGIN OF STATIC FUNCTIONS
******************************************************************/

/*!
 * \brief setup the usart peripheral
 * \details this is an extra function because the baudrate needs to be changed during reste commands
 */
static void onewire_hal_usart_setup(uint32_t baudrate)
{
    usart_disable(USART_INSTANCE);
    usart_set_baudrate(USART_INSTANCE, baudrate);
    usart_enable(USART_INSTANCE);
}

/*!
 * \brief Send one byte over USART
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
static void onewire_hal_usart_send(uint8_t tx_data_byte)
{
    uint16_t tx_word = (uint16_t)tx_data_byte;

    receive_flag = 0;

    usart_send_blocking(USART_INSTANCE, tx_word);

    while (!usart_get_flag(USART_INSTANCE, USART_SR_TC));
}

/*!
 * \brief Fetch a received byte from the receive buffer
 * \returns the received byte or 0 on timeout
 */
static uint8_t onewire_hal_usart_read(void)
{
    uint32_t timeout = ONEWIRE_READ_TIMEOUT;

    while ((receive_flag == 0) && (timeout--));

    if(timeout == 0)
    {
        /* timeout -> return 0x00 */
        return (0x00);
    }
    return (uint8_t)(receive_buffer & 0xFF);
}


/*!
 * \brief get the onewire-bit value for a received USART byte
 * \param[in] input_byte: input byte received over USART
 * \retval 1: if received a one-wire '1' (0xFF)
 * \retval 0: if received a one-wire '0' (0xFE or less)
 */
static uint8_t onewire_hal_usart_byte_to_bit(uint8_t input_byte)
{
    if(input_byte == 0xFF)
    {
        return (0x01);
    }
    else
    {
        return (0x00);
    }
}


/******************************************************************
* END OF STATIC FUNCTIONS
******************************************************************/

/*
 * \brief Usart interrupt service routine
 * \details Receives the byte from USARt RX buffer and saves it in the receive buffer.
 *          The receive flag notifies the application, that a byte was received
 */
void usart2_isr()
{
    if (((USART_CR1(USART_INSTANCE) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART_INSTANCE) & USART_SR_RXNE) != 0)) 
    {
        receive_buffer = usart_recv_blocking(USART_INSTANCE);
        receive_flag = 1;
    }
}
