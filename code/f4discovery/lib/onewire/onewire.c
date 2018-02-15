

#include <onewire/onewire.h>
#include <onewire/onewire_hal_usart.h>

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
    uint8_t presence = onewire_hal_usart_reset_line();

    if((presence >= 0x10) && (presence <= 0x90))
    {
        return (1);
    }
    else
    {
        return (0);
    }
    
}

/*!
 * \brief Transfer 1 byte
 * \param[in] tx_byte: data to send
 * \returns answer byte
 */
uint8_t onewire_xfer_byte(uint8_t tx_byte)
{
    uint8_t rx_byte = onewire_hal_usart_xfer_byte(tx_byte);

    return (rx_byte);
}
