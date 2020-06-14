/*
 * Task_Wifi.c
 *
 *  Created on: 13 jun. 2020
 *      Author: hanes
 */
#include <string.h>
#include "cmsis_os.h"
#include "ESP8266.h"
#include "Task_Wifi.h"
#include "Wifi_UART.h"

#define SSID		"Hanes y Euge"
#define PASSWORD	"Murcielago35"
#define BROKER_MQTT	"mqtt.eclipse.org"

typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;

/* Definitions for TaskWIFI */
osThreadId_t TaskWIFIHandle;
uint32_t TaskWIFI_Buffer[ 512 ];
osStaticThreadDef_t TaskWIFI_ControlBlock;
const osThreadAttr_t TaskWIFI_attributes = {
  .name = "TaskWIFI",
  .stack_mem = &TaskWIFI_Buffer[0],
  .stack_size = sizeof(TaskWIFI_Buffer),
  .cb_mem = &TaskWIFI_ControlBlock,
  .cb_size = sizeof(TaskWIFI_ControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};

/* Definitions for Wifi_Queue */
osMessageQueueId_t wifi_QueueHandle;
uint8_t wifi_Queue_Buffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t wifi_Queue_ControlBlock;
const osMessageQueueAttr_t wifi_Queue_attributes = {
  .name = "Wifi_Queue",
  .cb_mem = &wifi_Queue_ControlBlock,
  .cb_size = sizeof(wifi_Queue_ControlBlock),
  .mq_mem = &wifi_Queue_Buffer,
  .mq_size = sizeof(wifi_Queue_Buffer)
};

/* Definitions for wifi_Mutex_NewMsg */
osMutexId_t wifi_Mutex_NewMsgHandle;
osStaticMutexDef_t wifi_NewMsg_ControlBlock;
const osMutexAttr_t wifi_Mutex_NewMsg_attributes = {
  .name = "wifi_Mutex_NewMsg",
  .cb_mem = &wifi_NewMsg_ControlBlock,
  .cb_size = sizeof(wifi_NewMsg_ControlBlock),
};

/* Definitions for wifi_Sem_Operation */
osSemaphoreId_t wifi_Sem_OperationHandle;
osStaticSemaphoreDef_t wifi_Operative_ControlBlock;
const osSemaphoreAttr_t wifi_Sem_Operation_attributes = {
  .name = "wifi_Sem_Operation",
  .cb_mem = &wifi_Operative_ControlBlock,
  .cb_size = sizeof(wifi_Operative_ControlBlock),
};

/* Variable that link UART function to WIFI drivers */
static Wifi_CommInterface_s commInterface_wifi;
static ESP8266_NetworkParameters_s network;



/**
 * \brief Initialize module of communication of ESP8266 for
 * operate with CPU.
 *
 * \return Return 1 if operation was success or 0 in other way.
 */
static uint8_t ModuleWifi_CommInterface_Init(void);


/**
 * \brief Function to initialize mutex, semaphore and queue used
 * on task of Wifi.
 *
 * \return Return 1 if initialization was success or 0 on other way
 */
static uint8_t Wifi_SignalSync_Init(void);


/**
* \brief Function implementing the TaskWIFI thread.
*
* \param argument: Argument to pass a task.
*
* \retval None
*/
static void ModuleWifi(void *argument);


static uint8_t ModuleWifi_CommInterface_Init(void)
{
	ESP8266_StatusTypeDef_t status;

	commInterface_wifi.send = &Wifi_UART_Send;
	commInterface_wifi.recv = &Wifi_UART_Receive;

	status = ESP8266_CommInterface_Init(&commInterface_wifi);

	return (status == ESP8266_OK) ? 1 : 0;
}

static uint8_t Wifi_SignalSync_Init(void)
{
	/* creation of Wifi_Queue */
	wifi_QueueHandle = osMessageQueueNew (10, sizeof(uint8_t), &wifi_Queue_attributes);
	if (wifi_QueueHandle == NULL)
	{
		return 0;
	}

	/* creation of wifi_Mutex_NewMsg */
	wifi_Mutex_NewMsgHandle = osMutexNew(&wifi_Mutex_NewMsg_attributes);
	if (wifi_Mutex_NewMsgHandle == NULL)
	{
		return 0;
	}

	/* creation of wifi_Sem_Operation */
	wifi_Sem_OperationHandle = osSemaphoreNew(1, 0, &wifi_Sem_Operation_attributes);
	if (wifi_Sem_OperationHandle == NULL)
	{
		return 0;
	}

	return 1;
}

static void ModuleWifi(void *argument)
{
	ESP8266_StatusTypeDef_t status;
	uint8_t dataSend[10], retry = 0;
	uint8_t ssid[20], password[20];

	strncpy((char *)dataSend, "\0", 10);
	strncpy((char *)ssid, "\0", 20);
	strncpy((char *)password, "\0", 20);

	strncpy((char *)ssid, SSID, strlen(SSID));
	strncpy((char *)password, PASSWORD, strlen(PASSWORD));
	network.ssid = ssid;
	network.password = password;

	// Initialize communication interface ESP8266
	if (ModuleWifi_CommInterface_Init()  == 0)
	{
		osThreadTerminate(TaskWIFIHandle);
		return;
	}

	status = ESP8266_DisconnectAllNetwork();

	// Initialize parameters of module ESP8266
	do
	{
		status = ESP8266_Init();
		retry++;
	} while(status != ESP8266_OK && retry < 3);

	if (status != ESP8266_OK)
	{
		osThreadTerminate(TaskWIFIHandle);
		return;
	}

	// Connection ESP8266 at Wifi
	retry = 0;
	do
	{
		status = ESP8266_ConnectionNetwork(&network);
		retry++;
	} while(status != ESP8266_OK && retry < 3);

	for(;;)
	{
		osDelay(1);
	}

	// If task exit of while so destroy task. This is for caution
	osThreadTerminate(TaskWIFIHandle);
}


int8_t TaskWifi_Started(void)
{
	if (Wifi_SignalSync_Init() == 0)
	{
		return -1;
	}

	/* creation of TaskWIFI */
	TaskWIFIHandle = osThreadNew(ModuleWifi, NULL, &TaskWIFI_attributes);

	if (TaskWIFIHandle == NULL)
	{
		return -1;
	}

	return 1;
}
