/*
 * Task_Wifi.c
 *
 *  Created on: Jun 14, 2020
 *      Author: hanes
 */

#include <string.h>
#include "cmsis_os.h"

#include "Task_Wifi.h"
#include "Task_NFC.h"
#include "Wifi_UART.h"
#include "ESP8266.h"

#define SSID			"Hanes y Euge"
#define	PASSWORD		"Murcielago35"
#define HOST			"mqtt.eclipse.org"

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

/* Private variable */
uint8_t host[30], ssid[20], password[20], protocol[10];
static ESP8266_CommInterface_s commInterface;
static ESP8266_NetworkParameters_s network;
static ESP8266_ServerParameters_s service;

/**
 * @brief Initialize module ESP8266 before start to work
 *
 * @retval Return 1 was success or 0 in other way.
 */
static uint8_t WifiModule_Init(void);

/**
 * @brief Function to initialize communication interface of
 * library ESP8266
 *
 * @retval Return 1 if was successor 0 in  oher way.
 */
static uint8_t WifiModule_Comm_Init(void);

/**
 * @brief Function to initialize Queue, Semaphore and Mutex.
 *
 * @retval 1 if operation was success or 0 in other case.
 */
static uint8_t TaskWifi_SignalSync_Init(void);

/**
* @brief Function implementing the TaskWifi thread.
* @param argument: Argument to pass a task.
* @retval None
*/
static void ModuleWifi(void *argument);


static uint8_t WifiModule_Init(void)
{
	// Configure module for avoid echo
	if ( ESP8266_SetEcho(0) != ESP8266_OK)
	{
		return 0;
	}

	// Configure module as Station Mode
	if ( ESP8266_SetModeWIFI((uint8_t *)"1") != ESP8266_OK)
	{
		return 0;
	}

	// Configure module as Station Mode
	if ( ESP8266_SetMultipleConnection((uint8_t *)"0") != ESP8266_OK)
	{
		return 0;
	}

	return 1;
}

static uint8_t WifiModule_Comm_Init(void)
{
	ESP8266_StatusTypeDef_t status;

	commInterface.send = &WIFI_UART_Send;
	commInterface.recv = &WIFI_UART_Receive;

	status = ESP8266_CommInterface_Init(&commInterface);

	return (status == ESP8266_OK) ? 1 : 0;
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

static void ModuleWifi(void *argument)
{
	osStatus_t result = osErrorTimeout;
	ESP8266_StatusTypeDef_t status;
	uint8_t message[10], index, state = 0, retry;

	/* Initialization of library ESP8266 */
	if (!WifiModule_Comm_Init())
	{
		osThreadTerminate(TaskWifiHandle);
	}

	/* Initialization of module ESP8266 */
	if (!WifiModule_Init())
	{
		osThreadTerminate(TaskWifiHandle);
	}

	status = ESP8266_DisconnectAllNetwork();

	TaskNFC_SemaphoreGive();

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

		retry = 0;
		state = 0;
		while (state != 5)
		{
			switch (state)
			{
				case 0:
				{
					status = ESP8266_StatusNetwork();

					if (status != ESP8266_OK)
					{
						state = 1;
					}
					else
					{
						state = 2;
					}
				}
				break;

				case 1:
				{
					status = ESP8266_ConnectionNetwork(&network);


					if (status != ESP8266_OK && retry >= 3)
					{
						state = 5;
						retry = 0;
					}
					else if (status != ESP8266_OK && retry < 3)
					{
						state = 1;
						retry++;
					}
					else
					{
						state = 5;
						retry = 0;
						osDelay(TIME_MS_CMD/portTICK_PERIOD_MS);
					}
				}
				break;

				default:
					state = 5;
			}
		}

		osDelay(1);
	}

	// If task exit of while so destroy task. This is for caution
	osThreadTerminate(TaskWifiHandle);
}

int8_t TaskWifi_Started(void)
{
	uint16_t port;

	// Initialization of variable
	strncpy((char *)host, "\0", 30);
	strncpy((char *)protocol, "\0", 10);
	strncpy((char *)ssid, "\0", 20);
	strncpy((char *)password, "\0", 20);

	strncpy((char *)host, HOST, strlen(HOST));
	strncpy((char *)protocol, "TCP", strlen("TCP"));

	strncpy((char *)ssid, SSID, strlen(SSID));
	strncpy((char *)password, PASSWORD, strlen(PASSWORD));
	port = 1883;

	network.ssid = ssid;
	network.password = password;

	service.host = host;
	service.protocol = protocol;
	service.port = port;

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

