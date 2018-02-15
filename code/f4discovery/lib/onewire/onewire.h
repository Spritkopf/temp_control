


/*** include guard  */

 //  LICENSE ////

#include <stdint.h>


/*!
 * \brief Init 1-Wire bus master 
 */
void onewire_init(void);

/*!
 * \brief Send 1-Wire reset pulse
 * \returns: presence - 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_reset(void);

/*!
 * \brief Transfer 1 byte
 * \param[in] tx_byte: data to send
 * \returns answer byte
 */
uint8_t onewire_xfer_byte(uint8_t tx_byte);