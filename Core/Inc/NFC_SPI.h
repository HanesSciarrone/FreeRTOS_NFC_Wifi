/*
 * NFC_SPI.h
 *
 *  Created on: Jun 11, 2020
 *      Author: hanes
 */

#ifndef INC_NFC_SPI_H_
#define INC_NFC_SPI_H_

/// Comment this #define if you don't work with RTOS
#define NFC_SPI_RTOS

/// Time maximum to transmit data for SPI
#define SPI_NFC_TIMEOUT_TRANSMISSION	1000

/// Time maximum to wait reception data for SPI
#define SPI_NFC_TIMEOUT_RECEPTION		1000

/// Port and pin's number of clock SPI
#define SPI_SCK_Pin GPIO_PIN_12
#define SPI_SCK_GPIO_Port GPIOA

/// Port and pin's number of MOSI SPI
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOB

/// Port and pin's number of MISO SPI
#define SPI_MISO_Pin GPIO_PIN_14
#define SPI_MISO_GPIO_Port GPIOB

/// Port and pin's number of Chip Select SPI
#define SPI_CS_Pin GPIO_PIN_11
#define SPI_CS_GPIO_Port GPIOA

/// Port and pin's number of IRQ to PN532 NFC
#define NFC_IRQ_Pin GPIO_PIN_6
#define NFC_IRQ_GPIO_Port GPIOH


/**
 * \brief Initialize of SPI interface.
 *
 * \return Return 1 if initialize was success or 0 the other way.
 */
uint8_t NFC_SPI_Init(void);


/**
 * \brief Receive single byte for SPI
 *
 * \return Received byte.
 */
uint8_t NFC_SPI_GetByte(void);


/**
 * \brief Send single byte for SPI
 *
 * \param[in] byte Byte to be sent over SPI
 */
void NFC_SPI_SendByte(const uint8_t byte);


/**
 * \brief Set SPI Chip enable signal.
 * Setting the CS line will select the PN532, true is selected and false is not selected.
 *
 *  \param[in] state State of select signal, 1 to enable or other value to disable SPI
 */
void NFC_SPI_SetSelect(const uint8_t state);


/**
 *  \brief Test PN532 IRQ signal.
 *  The PN532 will toggle the IRQ to active when data is ready to be received from PN532
 *
 *  \return Return 1 if IRQ signal is active, 0 if not active.
 */
uint8_t NFC_SPI_GetIRQ(void);

#endif /* INC_NFC_SPI_H_ */
