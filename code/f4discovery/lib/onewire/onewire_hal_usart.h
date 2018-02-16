

/*** include guard  */


 //  LICENSE ////




#include <stdint.h>


/*!
 * \brief Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_init(void);

/*!
 * \brief De-Initialize USART peripheral in onewire half-duplex mode
 */
void onewire_hal_usart_deinit(void);

/*!
 * \brief Reset the line
 * \returns 1 if device(s) present on the bus, otherwise 0
 */
uint8_t onewire_hal_usart_reset_line(void);

/*!
 * \brief Send one byte
 * \param[in] tx_data_byte: data byte to send
 * \returns the received byte
 */
uint8_t onewire_hal_usart_send_byte(uint8_t tx_data_byte);

/*!
 * \brief Receive one byte
 * \returns the received byte
 */
uint8_t onewire_hal_usart_receive_byte(void);

