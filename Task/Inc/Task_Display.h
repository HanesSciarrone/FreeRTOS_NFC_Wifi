/*
 * Task_Display.h
 *
 *  Created on: Jun 18, 2020
 *      Author: hanes
 */

#ifndef INC_TASK_DISPLAY_H_
#define INC_TASK_DISPLAY_H_

#include "stm32f7xx_hal.h"

typedef enum
{
	MESSAGE_ERROR_NFC,
	MESSAGE_ERROR_WIFI,
	MESSAGE_PROCESS_INFO,
	MEESAGE_CONNECT_NETWORK,
	MESSAGE_CONNECTION,
	MESSAGE_REQ_SERVER,
	MESSAGE_RESP_SERVER,
	MESSAGE_PROCESS_AGAIN,
	MESSAGE_RESPONSE,

}Display_TypeMsg_t;


/* Public functions prototypes ---------------------------------------------*/

/**
* @brief  CPU L1-Cache enable.
*
* @retval None
*/
void CPU_CACHE_Enable(void);

/**
 * @brief Create task of display and run first display function.
 *
 * @retval Return 1 if was success on other way -1.
 */
int8_t TaskDisplay_Started(void);

/**
 * @brief Show message of start program.
 *
 * @retval None
 */
void Display_MessageStart(void);


/**
 * @brief Function to tell task display that show message through
 * screen.
 *
 * @param[in] typeMsg Type of message that you would like show.
 */
void Display_MsgShow(uint8_t typeMsg);

/**
 * @brief Function to tell task display that show response of server or pump controller on
 * screen.
 *
 * @param[in] msg Message that you would like show.
 * @param[in] length Length of message that you would like show.
 */
void Display_MsgShowResponse(uint8_t *msg, uint32_t length);

#endif /* INC_TASK_DISPLAY_H_ */
