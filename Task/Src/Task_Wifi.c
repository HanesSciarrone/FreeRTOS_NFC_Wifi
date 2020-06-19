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
#include "Task_Display.h"
#include "Wifi_UART.h"
#include "ESP8266.h"
#include "MQTTPacket.h"

#define SSID			"Hanes y Euge"
#define	PASSWORD		"Murcielago35"
#define HOST			"broker.hivemq.com"

#define KEEPALIVE_CONNECTION	60UL

typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;

/* Definitions for TaskWifi */
osThreadId_t TaskWifiHandle;
uint32_t TaskWifi_Buffer[ 1024 ];
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
static uint8_t host[30], ssid[20], password[20], protocol[10];
static uint8_t moduleResponse[MAX_BUFFER_SIZE];
static ESP8266_CommInterface_s commInterface;
static ESP8266_NetworkParameters_s network;
static ESP8266_ServerParameters_s service;


/**
 * @brief Convert character to hexadecimal with letter uppercase.
 *
 * @param[in] value Value on decimal to convert hexadecimal
 */
static uint8_t ModuleWifi_ConvertHex(uint8_t value);

/**
 * @brief Convert string to hexadecimal.
 *
 * @param[in] msg String that you will convert to hexadecimal.
 * @param[in] hexa String out with character on hexadecimal.
 * @param[in] size Length of string msg.
 */
static void WifiModule_IntToHex(uint8_t *msg, uint8_t *hexa, uint32_t size);

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

static uint8_t ModuleWifi_ConvertHex(uint8_t value)
{
	uint8_t returnValue = '0';
	switch(value)
	{
		case 0:
		{
			returnValue = '0';
		}
		break;

		case 1:
		{
			returnValue = '1';
		}
		break;

		case 2:
		{
			returnValue = '2';
		}
		break;

		case 3:
		{
			returnValue = '3';
		}
		break;

		case 4:
		{
			returnValue = '4';
		}
		break;

		case 5:
		{
			returnValue = '5';
		}
		break;

		case 6:
		{
			returnValue = '6';
		}
		break;

		case 7:
		{
			returnValue = '7';
		}
		break;

		case 8:
		{
			returnValue = '8';
		}
		break;

		case 9:
		{
			returnValue = '9';
		}
		break;

		case 10:
		{
			returnValue = 'A';
		}
		break;

		case 11:
		{
			returnValue = 'B';
		}
		break;

		case 12:
		{
			returnValue = 'C';
		}
		break;

		case 13:
		{
			returnValue = 'D';
		}
		break;

		case 14:
		{
			returnValue = 'E';
		}
		break;

		case 15:
		{
			returnValue = 'F';
		}
		break;
	}

	return returnValue;
}

static void WifiModule_IntToHex(uint8_t *msg, uint8_t *hexa, uint32_t size)
{
	uint32_t i, index = 0;
	uint8_t temp;

	for (i = 0; i < size; i++)
	{
		temp = *(msg+i) & 0xF0;
		temp >>= 4;
		*(hexa + index++) = ModuleWifi_ConvertHex(temp);
		temp = *(msg+i) & 0x0F;
		*(hexa + index++) = ModuleWifi_ConvertHex(temp);
	}
}

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
	Display_TypeMsg_t msgDisplay;
	ESP8266_StatusTypeDef_t status;
	MQTTPacket_connectData dataConnection = MQTTPacket_connectData_initializer;
	MQTTString topicString = MQTTString_initializer;

	uint32_t length = 0;
	uint16_t subcribe_MsgID;
	int32_t requestQoS, subcribeCount, granted_QoS;
	uint8_t message[10], data[30], index, state = 0, retry;
	uint8_t sessionPresent, connack_rc, buffer[200];


	/* Initialization of library ESP8266 */
	if (!WifiModule_Comm_Init())
	{
		msgDisplay = MESSAGE_ERROR_WIFI;
		Display_MsgShow(msgDisplay);
		osDelay(20/portTICK_PERIOD_MS);
		osSemaphoreDelete(wifi_Sem_OperationHandle);
		osMutexDelete(wifi_Mutex_NewMsgHandle);
		osMessageQueueDelete(wifi_QueueHandle);
		osThreadTerminate(TaskWifiHandle);
	}

	/* Initialization of module ESP8266 */
	if (!WifiModule_Init())
	{
		msgDisplay = MESSAGE_ERROR_WIFI;
		Display_MsgShow(msgDisplay);
		osDelay(20/portTICK_PERIOD_MS);
		osSemaphoreDelete(wifi_Sem_OperationHandle);
		osMutexDelete(wifi_Mutex_NewMsgHandle);
		osMessageQueueDelete(wifi_QueueHandle);
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

		msgDisplay = MESSAGE_PROCESS_INFO;
		Display_MsgShow(msgDisplay);
		osDelay(20/portTICK_PERIOD_MS);

		index--;
		strncpy((char *)data, "\0", 30);
		WifiModule_IntToHex(message, data, index);

		retry = 0;
		state = 0;
		while (state != 9)
		{
			switch (state)
			{
				// Ask for status of connection
				case 0:
				{
					status = ESP8266_StatusNetwork();

					if (status != ESP8266_OK)
					{
						msgDisplay = MEESAGE_CONNECT_NETWORK;
						Display_MsgShow(msgDisplay);
						state = 2;
						state = 1;
					}
					else
					{
						state = 2;
					}
					osDelay(TIME_MS_ESTABLISH_SERVER/portTICK_PERIOD_MS);
				}
				break;

				// Connect to Wifi
				case 1:
				{
					status = ESP8266_ConnectionNetwork(&network);


					if (status == ESP8266_OK)
					{
						state = 0;
						msgDisplay = MESSAGE_CONNECTION;
						Display_MsgShow(msgDisplay);
						osDelay(TIME_MS_ESTABLISH_SERVER/portTICK_PERIOD_MS);
					}
				}
				break;

				// Connecto to server
				case 2:
				{
					status = ESP8266_ConnectionServer(&service);

					if (status != ESP8266_OK)
					{
						state = 0;
					}
					else
					{
						state = 3;
					}
				}

				//Send Connect MQTT
				case 3:
				{
					retry = 0;
					while (retry < 3)
					{
						dataConnection.MQTTVersion = 3;
						dataConnection.clientID.cstring = "Hanes";
						dataConnection.keepAliveInterval = KEEPALIVE_CONNECTION;
						dataConnection.will.qos = 0;

						strncpy((char *)buffer, "\0", sizeof(buffer));
						length = MQTTSerialize_connect(buffer, sizeof(buffer), &dataConnection);
						status = ESP8266_SentData(buffer, length);

						if (status != ESP8266_OK)
						{
							ESP8266_GetModuleResponse(moduleResponse, MAX_BUFFER_SIZE);

							if (strstr((char *)moduleResponse, "CLOSED\r\n") != NULL)
							{
								retry = 3;
								state = 2;
							}
							else
							{
								retry++;
								state = 7;
							}
						}
						else
						{
							strncpy((char *)buffer, "\0", sizeof(buffer));
							length = 0;
							status = ESP8266_ReceiveData(buffer, &length);

							if (status != ESP8266_OK)
							{
								state = 7;
							}
							else
							{
								if ( MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer, strlen((char *)buffer)) != 1 )
								{
									state = 7;
								}
								else
								{
									state = 4;
									break;
								}
							}

							retry++;
						}
					}

					if (retry >= 3 && state == 7)
					{
						msgDisplay = MESSAGE_PROCESS_AGAIN;
						Display_MsgShow(msgDisplay);
						osDelay(10/portTICK_PERIOD_MS);
					}
				}
				break;

				// Send Subscribe MQTT
				case 4:
				{
					retry = 0;
					while (retry < 3)
					{
						MQTTString topicSubcribeString = MQTTString_initializer;
						topicSubcribeString.cstring = "SUB_RTOS1";
						requestQoS = 0;

						length = MQTTSerialize_subscribe(buffer, sizeof(buffer), 0, 1, 1, &topicSubcribeString, (int *)&requestQoS);
						status = ESP8266_SentData(buffer, length);

						if( status != ESP8266_OK )
						{
							ESP8266_GetModuleResponse(moduleResponse, MAX_BUFFER_SIZE);
							if( strstr((char *)moduleResponse, "CLOSED\r\n") != NULL )
							{
								retry = 3;
								state = 2;
							}
							else
							{
								state = 7;
								retry++;
							}
						}
						else
						{
							strncpy((char *)buffer, "\0", sizeof(buffer));
							length = 0;
							status =  ESP8266_ReceiveData(buffer, &length);

							if (status != ESP8266_OK)
							{
								state = 7;
							}
							else
							{
								if ( MQTTDeserialize_suback(&subcribe_MsgID, 1, (int *)&subcribeCount, (int *)&granted_QoS, buffer, strlen((char *)buffer)) != 1 )
								{
									state = 7;
								}
								else
								{
									msgDisplay = MESSAGE_REQ_SERVER;
									Display_MsgShow(msgDisplay);
									osDelay(10/portTICK_PERIOD_MS);
									state = 5;
									break;
								}
							}

							retry++;
						}
					}

					if (retry >= 3 && state == 7)
					{
						msgDisplay = MESSAGE_PROCESS_AGAIN;
						Display_MsgShow(msgDisplay);
						osDelay(10/portTICK_PERIOD_MS);
					}
				}
				break;

				// Send message
				case 5:
				{
					topicString.cstring = "PUB_RTOS1";
					length = MQTTSerialize_publish(buffer, sizeof(buffer), 0, 0, 0, 0, topicString, (unsigned char*)data, strlen((char *)data));
					status = ESP8266_SentData(buffer, length);

					if( status != ESP8266_OK )
					{

						msgDisplay = MESSAGE_PROCESS_AGAIN;
						Display_MsgShow(msgDisplay);
						state = 7;
						break;
					}
					else
					{
						state = 6;
					}
				}
				break;

				// Receive Command
				case 6:
				{
					msgDisplay = MESSAGE_RESP_SERVER;
					Display_MsgShow(msgDisplay);
					osDelay(10/portTICK_PERIOD_MS);

					strncpy((char *)buffer, "\0", sizeof(buffer));
					length = 0;
					status =  ESP8266_ReceiveData(buffer, &length);

					if (status != ESP8266_OK)
					{
						msgDisplay = MESSAGE_PROCESS_AGAIN;
						Display_MsgShow(msgDisplay);
					}
					else
					{
						if( strlen((char *)buffer) != 0 )
						{
							uint8_t granted_Dup, granted_Retained, *granted_Payload;
							int32_t granted_QoS, granted_PayloadLen;
							uint16_t granted_PacketID;
							MQTTString granted_TopicString;

							MQTTDeserialize_publish(&granted_Dup, (int *)&granted_QoS, &granted_Retained, &granted_PacketID, &granted_TopicString, (unsigned char **)&granted_Payload, (int *)&granted_PayloadLen, (unsigned char *)buffer, (int)granted_PayloadLen);

							strncpy((char *)data, "\0", sizeof(data));
							strncpy((char *)data, (char *)granted_Payload, granted_PayloadLen);
							Display_MsgShowResponse(data, granted_PayloadLen);
							osDelay(10/portTICK_PERIOD_MS);
						}
					}

					state = 7;
				}
				break;

				// Send unsubscribe MQTT
				case 7:
				{
					strncpy((char *)buffer, "\0", sizeof(buffer));
					length = MQTTSerialize_disconnect(buffer, sizeof(buffer));
					status = ESP8266_SentData(buffer, length);

					state = 8;
				}
				break;

				// Close connection of server
				case 8:
				{
					status = ESP8266_ConnectionClose();

					state = 9;
				}
				break;

				default:
					state = 0;
			}
		}

		osDelay(1);
	}

	// If task exit of while so destroy task, semaphore, mutex, and queue. This is for caution
	osSemaphoreDelete(wifi_Sem_OperationHandle);
	osMutexDelete(wifi_Mutex_NewMsgHandle);
	osMessageQueueDelete(wifi_QueueHandle);
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

