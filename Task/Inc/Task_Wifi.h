/*
 * Task_Wifi.h
 *
 *  Created on: 13 jun. 2020
 *      Author: hanes
 */

#ifndef INC_TASK_WIFI_H_
#define INC_TASK_WIFI_H_

#include "stm32f7xx_hal.h"

/**
 * \brief Function to create task of Wifi.
 *
 * \return Return 1 is operation was success or -1 in other case
 */
int8_t TaskWifi_Started(void);

#endif /* INC_TASK_WIFI_H_ */
