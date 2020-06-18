/*
 * Wifi_UART.h
 *
 *  Created on: 12 jun. 2020
 *      Author: hanes
 */

#ifndef INC_WIFI_UART_H_
#define INC_WIFI_UART_H_

#include "stm32f7xx_hal.h"

/// Comment this #define if you don't work with RTOS
#define WIFI_UART_RTOS

/// Define time wait
#define DEFAULT_TIME_OUT	3000

#define Wifi_Tx_Pin GPIO_PIN_12
#define Wifi_Tx_GPIO_Port GPIOC
#define Wifi_Rx_Pin GPIO_PIN_2
#define Wifi_Rx_GPIO_Port GPIOD

/**
 * \brief Function to initialize peripheral UART used to communicate between
 * Wifi and CPU.
 *
 * \return Return 1 if initialization was success or 0 in other way.
 */
uint8_t WIFI_UART_Init(void);


/**
  * @brief  Send Data to the ESP8266 module over the UART interface.
  *         This function allows sending data to the  ESP8266 WiFi Module, the
  *          data can be either an AT command or raw data to send over
             a pre-established WiFi connection.
  * @param pData: data to send.
  * @param Length: the data length.
  * @retval 0 on success, -1 otherwise.
  */
int8_t WIFI_UART_Send(uint8_t* data, uint32_t length);


/**
  * @brief  Receive Data from the ESP8266 module over the UART interface.
  *         This function receives data from the  ESP8266 WiFi module, the
  *         data is fetched from a wifi buffer that is asynchonously and continuously
            filled with the received data.
  * @param[in]  buffer a buffer inside which the data will be read.
  * @param[in]  length the Maximum size of the data to receive.
  * \return The actual data size that has been received.
  */
int32_t WIFI_UART_Receive(uint8_t* buffer, uint32_t length);

#endif /* INC_WIFI_UART_H_ */
