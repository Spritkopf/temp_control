


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

#define ONEWIRE_RESET_PULSE     0xFE            /* 0xFE represents a 1-Wire reset pulse @ 9600 Baudrate */
#define ONEWIRE_READ_TIMEOUT    10000           /* timeout for blocking uart read */
#define ONEWIRE_READ_SLOT       0xFF            

static void onewire_hal_usart_setup(uint32_t baudrate);
static uint8_t onewire_bit_to_byte(uint8_t input_byte, uint8_t offset);
static uint8_t onewire_hal_usart_send(uint8_t tx_data_byte);
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

    onewire_hal_usart_setup(USART_BAUDRATE_RESET);

    onewire_hal_usart_send(ONEWIRE_RESET_PULSE);      /* transmit raw value for "reset pulse" */



    onewire_hal_usart_setup(USART_BAUDRATE_COMM);    

    return (result);
}


/*!
 * \brief Send/receive one byte
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
uint8_t onewire_hal_usart_send_byte(uint8_t tx_data_byte)
{
    uint8_t bit_idx;
    uint8_t tx_byte;
    uint8_t rx_byte = 0;
    
    for(bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        tx_byte = onewire_bit_to_byte(tx_data_byte, bit_idx);

        onewire_hal_usart_send(tx_byte);
    }

    /* todo: function for bytes to bits (... or so) */
    return (rx_byte);
}


/*!
 * \brief Receive one byte
 * \returns the received byte
 */
uint8_t onewire_hal_usart_receive_byte(void)
{
    uint8_t bit_idx;
    uint8_t rx_byte = 0;
    uint8_t rx_bits[8];

    onewire_hal_usart_send(ONEWIRE_READ_SLOT);

    for(bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        rx_bits[bit_idx] = onewire_hal_usart_read();
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
    usart_disable(USART_INSTANCE);
    usart_set_baudrate(USART_INSTANCE, baudrate);
    usart_enable(USART_INSTANCE);
}

/*!
 * \brief Send one byte over USART
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
static uint8_t onewire_hal_usart_send(uint8_t tx_data_byte)
{
    uint16_t tx_word = (uint16_t)tx_data_byte;

    usart_send_blocking(USART_INSTANCE, tx_word);

    while (!usart_get_flag(USART_INSTANCE, USART_SR_TC));

    return (0);
}

/*!
 * \brief Receive one byte
 * \returns the received byte
 */
static uint8_t onewire_hal_usart_read(void)
{
    uint32_t timeout = ONEWIRE_READ_TIMEOUT;
    while ((receive_flag ==1) && (timeout--));

    return (uint8_t)(receive_buffer & 0xFF);
}

/*!
 * \brief get the onewire-byte-representation of a single bit
 * \param[in] input_byte: input byte
 * \param[in] offset: number of the bit (0 = LSB)
 * \returns onewire-send-byte representing the bit write slot  (1=0xFF; 0=0x00)
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


void usart2_isr()
{
    if (((USART_CR1(USART_INSTANCE) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART_INSTANCE) & USART_SR_RXNE) != 0)) 
    {
        receive_buffer = usart_recv_blocking(USART_INSTANCE);
        receive_flag = 1;
    }
}