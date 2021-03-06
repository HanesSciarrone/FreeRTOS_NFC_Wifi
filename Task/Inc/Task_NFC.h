/*
 * Task_NFC.h
 *
 *  Created on: Jun 9, 2020
 *      Author: hanes
 */

#ifndef INC_TASK_NFC_H_
#define INC_TASK_NFC_H_

#include "stm32f7xx_hal.h"

/**
 * @brief Function to create task of NFC.
 *
 * @retval Return 1 is operation was success or -1 in other case
 */
int8_t TaskNFC_Started(void);


/**
 * @brief Give semaphore to start of task NFC. This function can execute one time,
 * then semaphore is deleted.
 */
void TaskNFC_SemaphoreGive(void);

#endif /* INC_TASK_NFC_H_ */
