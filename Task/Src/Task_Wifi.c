/*
 * Task_Wifi.c
 *
 *  Created on: Jun 14, 2020
 *      Author: hanes
 */

#include <string.h>
#include "cmsis_os.h"

#include "Task_Wifi.h"
#include "Wifi_UART.h"


typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;

/* Definitions for TaskWifi */
osThreadId_t TaskWifiHandle;
uint32_t TaskWifi_Buffer[ 512 ];
osStaticThreadDef_t TaskWifi_ControlBlock;
const osThreadAttr_t TaskWifi_attributes = {
  .name = "TaskWifi",
  .stack_mem = &TaskWifi_Buffer[0],
  .stack_size = sizeof(TaskWifi_Buffer),
  .cb_mem = &TaskWifi_ControlBlock,
  .cb_size = sizeof(TaskWifi_ControlBlock),
  .priority = (osPriority_t) osPriorityNormal1,
};

/* Definitions for wifi_Queue */
osMessageQueueId_t wifi_QueueHandle;
uint8_t wifi_Queue_Buffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t wifi_Queue_ControlBlock;
const osMessageQueueAttr_t wifi_Queue_attributes = {
  .name = "wifi_Queue",
  .cb_mem = &wifi_Queue_ControlBlock,
  .cb_size = sizeof(wifi_Queue_ControlBlock),
  .mq_mem = &wifi_Queue_Buffer,
  .mq_size = sizeof(wifi_Queue_Buffer)
};

/* Definitions for wifi_Mutex_NewMsg */
osMutexId_t wifi_Mutex_NewMsgHandle;
osStaticMutexDef_t Wifi_NewMsg_ControlBlock;
const osMutexAttr_t wifi_Mutex_NewMsg_attributes = {
  .name = "wifi_Mutex_NewMsg",
  .cb_mem = &Wifi_NewMsg_ControlBlock,
  .cb_size = sizeof(Wifi_NewMsg_ControlBlock),
};

/* Definitions for wifi_Sem_Operation */
osSemaphoreId_t wifi_Sem_OperationHandle;
osStaticSemaphoreDef_t wifi_Operative_ControlBlock;
const osSemaphoreAttr_t wifi_Sem_Operation_attributes = {
  .name = "wifi_Sem_Operation",
  .cb_mem = &wifi_Operative_ControlBlock,
  .cb_size = sizeof(wifi_Operative_ControlBlock),
};


/**
* @brief Function implementing the TaskWifi thread.
* @param argument: Argument to pass a task.
* @retval None
*/
static void ModuleWifi(void *argument);

/**
 * @brief Function to initialize Queue, Semaphore and Mutex.
 *
 * @retval 1 if operation was success or 0 in other case.
 */
static uint8_t TaskWifi_SignalSync_Init(void);



static void ModuleWifi(void *argument)
{
	osStatus_t result = osErrorTimeout;
	uint8_t message[10], index;

	/* Infinite loop */
	for(;;)
	{
		osSemaphoreAcquire(wifi_Sem_OperationHandle, portMAX_DELAY);

		strncpy((char *)message, "\0", 10);
		index = 0;
		do
		{
			result = osMessageQueueGet(wifi_QueueHandle, &message[index], 0, 0);
			index++;
		} while (result == osOK);
		osDelay(1);
	}

	// If task exit of while so destroy task. This is for caution
	osThreadTerminate(TaskWifiHandle);
}

static uint8_t TaskWifi_SignalSync_Init(void)
{
	/* creation of wifi_Queue */
	if ((wifi_QueueHandle = osMessageQueueNew (10, sizeof(uint8_t), &wifi_Queue_attributes)) == NULL)
	{
		return 0;
	}

	/* creation of wifi_Mutex_NewMsg */
	if ((wifi_Mutex_NewMsgHandle = osMutexNew(&wifi_Mutex_NewMsg_attributes)) == NULL)
	{
		return 0;
	}

	/* creation of wifi_Sem_Operationnin block state */
	if ((wifi_Sem_OperationHandle = osSemaphoreNew(1, 0, &wifi_Sem_Operation_attributes)) == NULL )
	{
		return 0;
	}

	return 1;
}

int8_t TaskWifi_Started(void)
{
	if (!TaskWifi_SignalSync_Init())
	{
		return -1;
	}

	/* creation of TaskWifi */
	TaskWifiHandle = osThreadNew(ModuleWifi, NULL, &TaskWifi_attributes);
	if (TaskWifiHandle == NULL)
	{
		return -1;
	}

	return 1;
}

void TaskWifi_MsgSendValidation(uint8_t *uid, uint8_t length)
{
	uint8_t i;

	osMutexAcquire(wifi_Mutex_NewMsgHandle, portMAX_DELAY);
	for (i = 0; i < length; i++)
	{
		osMessageQueuePut(wifi_QueueHandle, &uid[i], 0U, 0);
	}
	osMutexRelease(wifi_Mutex_NewMsgHandle);

	osSemaphoreRelease(wifi_Sem_OperationHandle);
}

