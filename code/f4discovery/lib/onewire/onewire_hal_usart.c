


 //  LICENSE ////




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

static void onewire_hal_usart_setup(uint32_t baudrate);
static uint8_t onewire_hal_usart_xfer(uint8_t tx_data_byte);
static uint8_t onewire_bit_to_byte(uint8_t input_byte, uint8_t offset);


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
    gpio_set_af(USART_GPIO_PORT, GPIO_AF7 ,USART_GPIO_PIN_TX | USART_GPIO_PIN_RX);

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

/*  don't use interrupt for now...

    usart_enable_rx_interrupt(USART_INSTANCE);
    nvic_enable_irq(USART_IRQ);
*/
    /* set baudrate and enable usart peripheral */
    onewire_hal_usart_setup(USART_BAUDRATE_COMM);
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

    onewire_hal_usart_setup(USART_BAUDRATE_RESET);

    result = onewire_hal_usart_xfer(0xF0);      /* transmit raw value for "reset pulse" */

    onewire_hal_usart_setup(USART_BAUDRATE_COMM);    

    return (result);
}


/*!
 * \brief Send/receive one byte
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
uint8_t onewire_hal_usart_xfer_byte(uint8_t tx_data_byte)
{
    uint8_t bit_idx;
    uint8_t tx_byte;
    uint8_t rx_byte = 0;
    
    for(bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        tx_byte = onewire_bit_to_byte(tx_data_byte, bit_idx);

        rx_byte = onewire_hal_usart_xfer(tx_byte);
    }

    return (rx_byte);
}



/* --------------static functions---------------------*/

/*!
 * \brief setup the usart peripheral

 \details this is an extra function because the baudrate needs to be changed during reste commands
 */
static void onewire_hal_usart_setup(uint32_t baudrate)
{
    usart_set_baudrate(USART_INSTANCE, baudrate);
    usart_enable(USART_INSTANCE);
}

/*!
 * \brief Send/receive one byte over USART
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
static uint8_t onewire_hal_usart_xfer(uint8_t tx_data_byte)
{
    uint16_t rx_word = 0;
    uint16_t tx_word = (uint16_t)tx_data_byte;

    usart_wait_send_ready(USART_INSTANCE);
    usart_send_blocking(USART_INSTANCE, tx_word);

    usart_wait_recv_ready(USART_INSTANCE);
    rx_word = usart_recv_blocking(USART_INSTANCE);
    

    return ((uint8_t)(rx_word & 0xFF));
}


/*!
 * \brief get the onewire-byte-representation of a single bit
 * \param[in] input_byte: input byte
 * \param[in] offset: number of the bit (0 = LSB)
 * \returns onewire-send-byte representing the bit (1=0xFF; 0=0x00)
 */
static uint8_t onewire_bit_to_byte(uint8_t input_byte, uint8_t offset)
{
    if(offset > 7)
    {
        offset = 7;
    }

    if(((input_byte>>offset) & 0x01) == 0x01)
    {
        return (0xFF);
    }
    else
    {
        return (0x00);
    }
}



