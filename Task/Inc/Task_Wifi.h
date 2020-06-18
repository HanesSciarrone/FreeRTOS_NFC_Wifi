/*
 * Task_Wifi.h
 *
 *  Created on: Jun 14, 2020
 *      Author: hanes
 */

#ifndef INC_TASK_WIFI_H_
#define INC_TASK_WIFI_H_

#include "stm32f7xx_hal.h"

#define TIME_MS_CONNECTION			2000
#define TIME_MS_ESTABLISH_SERVER	3000

int8_t TaskWifi_Started(void);

/**
 * @brief Sent message to task of Wifi.
 *
 * @param[in] uid Message vector, maximum of length is 10
 * @param[in] length Length of message to sent
 */
void TaskWifi_MsgSendValidation(uint8_t *uid, uint8_t length);
#endif /* INC_TASK_WIFI_H_ */
