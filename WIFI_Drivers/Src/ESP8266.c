/*
 * ESP8266.c
 *
 *  Created on: Apr 18, 2020
 *      Author: hanes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ESP8266.h"

#ifdef	WIFI_RTOS
#include "cmsis_os.h"
#endif
//#include "MQTTPacket.h"

#define MAX_BUFFER_SIZE		1500

#define AT_OK				"OK\r\n"
#define AT_SEND_LENGTH_OK	"OK\r\n>"
#define AT_SEND_OK			"SEND OK\r\n"
#define AT_ERROR			"ERROR\r\n"
#define AT_STRING_IPD		"+IPD,"
#define AT_RES_BUSYP_STR	"busy p..."
#define AT_RES_BUSYS_STR	"busy s..."
#define AT_BUSY				"busy"
#define AT_IPD_STRING_OK	"OK\r\n\r\n"
#define AT_STATUS_OK		"STATUS:2\r\n"

#define KEEPALIVE_CONNECTION	60UL

typedef enum
{
	FALSE,
	TRUE,
	false = 0,
	true
}ESP8266_bool_t;

/* Declaretion of global variable private */
uint8_t bufferRx[MAX_BUFFER_SIZE];
uint8_t tempBuffer[100];
uint32_t templength = 0;

Wifi_CommInterface_s *commInterface;
/* -------------------------------------- */


static void ESP8266_Delay(uint32_t delay);
static uint16_t ESP8266_CloseConnection(uint8_t *command, uint16_t size);
static uint16_t ESP8266_StatusConnection(uint8_t *command, uint16_t size);
static uint16_t ESP8266_SendLengthData(uint8_t *command, uint16_t size, const uint32_t lengthData);
static uint16_t ESP8266_ConnectionServer(uint8_t *command, uint16_t size, const ESP8266_ServerParameters_s *parameters);
static uint16_t ESP8266_DisconnectAccessPoint(uint8_t *command, uint16_t sizeCommand);
static uint16_t ESP8266_JoinAccessPoint(uint8_t *command, uint16_t size, const ESP8266_NetworkParameters_s *parameters);
static uint16_t ESP8266_SetMultipleConnection(uint8_t *command, uint16_t sizeCommand, const uint8_t mode);
static uint16_t ESP8266_SetModeWIFI(uint8_t *command, uint16_t sizeCommand, const uint8_t mode);
static uint16_t ESP8266_SetEcho(uint8_t *command, uint16_t size, const uint8_t mode);
static ESP8266_StatusTypeDef_t ESP8266_SendData(uint8_t *data, uint16_t length, const uint8_t *pattern);
static ESP8266_StatusTypeDef_t ESP8266_SendCommand(uint8_t *command, uint16_t length, const uint8_t *pattern);



static void ESP8266_Delay(uint32_t delay)
{
#ifdef WIFI_RTOS
	osDelay(delay / portTICK_PERIOD_MS);
#else
	uint32_t startTick;
	startTick = HAL_GetTick();
	while( HAL_GetTick() - startTick < delay );
#endif
}

static uint16_t ESP8266_CloseConnection(uint8_t *command, uint16_t size)
{
	if ( command == NULL )
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	strncpy((char *)command, "AT+CIPCLOSE\r\n", 13);

	return 14;
}

static uint16_t ESP8266_StatusConnection(uint8_t *command, uint16_t size)
{
	if (command == NULL)
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	strncpy((char *)command, "AT+CIPSTATUS\r\n", 14);

	return 15;
}

static uint16_t ESP8266_SendLengthData(uint8_t *command, uint16_t size, const uint32_t lengthData)
{
	int length = 0;
	if ( command == NULL )
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	strncpy((char *)command, "AT+CIPSEND=", 11);
	length = 11;

	itoa(lengthData, (char *)(command+length), 10);
	length = strlen((char *)command);

	strncpy((char *)(command+length), "\r\n", 2);
	length += 3;

	return length;
}

static uint16_t ESP8266_ConnectionServer(uint8_t *command, uint16_t size, const ESP8266_ServerParameters_s *parameters)
{
	int i, offset, length;
	if( command ==  NULL || parameters->host == NULL || parameters->protocol == NULL || parameters->port <= 0 )
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	strncpy((char *)command, "AT+CIPSTART=\"",13);
	length = offset = 13;

	for (i = 0; i < strlen((char *)parameters->protocol); i++)
	{
		command[offset+i] = *(parameters->protocol+i);
		length++;
	}

	strncpy((char *)(command+length),"\",\"", 3);
	length += 3;

	offset = length;
	for (i = 0; i < strlen((char *)parameters->host); i++)
	{
		command[offset+i] = *(parameters->host+i);
		length++;
	}

	strncpy((char *)(command+length),"\",", 2);
	length += 2;

	itoa(parameters->port, (char *)(command+length), 10);
	length = strlen((char *)command);
	strncpy((char *)(command+length),"\r\n", 2);
	length += 3;

	return length;
}

static uint16_t ESP8266_DisconnectAccessPoint(uint8_t *command, uint16_t sizeCommand)
{
	if (command == NULL || sizeCommand <= 10)
	{
		return 0;
	}

	strncpy((char *)command, "\0", sizeCommand);
	strncpy((char *)command, "AT+CWQAP\r\n", 11);

	return 10;

}

static uint16_t ESP8266_JoinAccessPoint(uint8_t *command, uint16_t size, const ESP8266_NetworkParameters_s *parameters)
{
	uint32_t i, offset = 0, length = 0;

	if( command ==  NULL || parameters->ssid == NULL || parameters->password == NULL )
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	strncpy((char *)command, "AT+CWJAP_CUR=\"", 14);
	length = 14;
	offset = length;
	for (i = 0; i < strlen((char *)parameters->ssid); i++)
	{
		command[offset+i] = *(parameters->ssid + i);
		length++;
	}
	strncpy((char *)(command+length), "\",\"", 3);
	length += 3;

	offset = length;
	for (i = 0; i < strlen((char *)parameters->password); i++)
	{
		command[offset+i] = *(parameters->password + i);
		length++;
	}

	strncpy((char *)(command+length), "\"\r\n", 3);
	length += 3;

	return length;
}

static uint16_t ESP8266_SetMultipleConnection(uint8_t *command, uint16_t sizeCommand, const uint8_t mode)
{
	if (command == NULL || sizeCommand <= 13)
	{
		return 0;
	}

	strncpy((char *)command, "\0", sizeCommand);
	strncpy((char *)command, "AT+CIPMUX=", 11);
	command[10] = mode + '0';
	command[11] = '\r';
	command[12] = '\n';

	return 14;
}

static uint16_t ESP8266_SetModeWIFI(uint8_t *command, uint16_t sizeCommand, const uint8_t mode)
{

	if (command == NULL || sizeCommand <= 13)
	{
		return 0;
	}

	strncpy((char *)command, "\0", sizeCommand);
	strncpy((char *)command, "AT+CWMODE=", 11);
	command[10] = mode + '0';
	command[11] = '\r';
	command[12] = '\n';

	return 14;
}

static uint16_t ESP8266_SetEcho(uint8_t *command, uint16_t size, const uint8_t mode)
{
	if (command == NULL || size<= 6)
	{
		return 0;
	}

	strncpy((char *)command, "\0", size);
	switch(mode)
	{
		case 1:
		{
			strncpy((char *)command, "ATE1\r\n", 7);
		}
		break;

		default:
		{
			strncpy((char *)command, "ATE0\r\n", 7);
		}
	}

	return 7;
}

static ESP8266_StatusTypeDef_t ESP8266_SendData(uint8_t *data, uint16_t length, const uint8_t *pattern)
{
	uint32_t offset = 0;
	uint8_t charRx = 0;

	if (commInterface->send(data, length) != 0)
	{
		return ESP8266_ERROR;
	}

	strncpy((char *)bufferRx, "\0", MAX_BUFFER_SIZE);
	while( commInterface->recv(&charRx, 1) != 0 )
	{
		bufferRx[offset++] = charRx;

		if( offset == MAX_BUFFER_SIZE )
		{
			offset = 0;
		}

		if(  strstr((char *)bufferRx, AT_RES_BUSYP_STR) != NULL  || strstr((char *)bufferRx, AT_RES_BUSYS_STR) != NULL )
		{
			return ESP8266_OK;
		}
		else if ( strstr((char *)bufferRx, AT_ERROR) != NULL )
		{
			return ESP8266_ERROR;
		}
		else if( strstr((char *)bufferRx, (const char *)pattern) != NULL )
		{
			return ESP8266_OK;
		}
	}

	return ESP8266_ERROR;
}

static ESP8266_StatusTypeDef_t ESP8266_SendCommand(uint8_t *command, uint16_t length, const uint8_t *pattern)
{
	uint32_t offset = 0;
	uint8_t charRx = 0;

	if (commInterface->send(command, length) != 0)
	{
		return ESP8266_ERROR;
	}

	strncpy((char *)bufferRx, "\0", MAX_BUFFER_SIZE);
	while( commInterface->recv(&charRx, 1) != 0 )
	{
		bufferRx[offset++] = charRx;

		if( offset == MAX_BUFFER_SIZE )
		{
			offset = 0;
		}

		if ( strstr((char *)bufferRx, AT_ERROR) != NULL )
		{
			return ESP8266_ERROR;
		}
		else if( strstr((char *)bufferRx, (const char *)pattern) != NULL ||
				strstr((char *)bufferRx, "WIFI CONNECTED\r\n") != NULL /*||
				strstr((char *)bufferRx, "GOT IP\r\n") != NULL*/)
		{
			return ESP8266_OK;
		}
	}

	return ESP8266_ERROR;
}

ESP8266_StatusTypeDef_t ESP8266_CommInterface_Init(Wifi_CommInterface_s  *interface)
{
	if (interface == NULL)
	{
		return ESP8266_ERROR;
	}

	commInterface = interface;

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_Init(void)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	// Time wait of restart ESP8266
	ESP8266_Delay(ESP_TIME_MS_RESTART);

	// Configure module for avoid echo
	length = ESP8266_SetEcho(command, MAX_BUFFER_SIZE, 0);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);

	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}
	ESP8266_Delay(ESP_TIME_MS_COMMAND);

	// Configure module as Station Mode
	length = ESP8266_SetModeWIFI(command, MAX_BUFFER_SIZE, 1);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);

	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}
	ESP8266_Delay(ESP_TIME_MS_COMMAND);

	// Configure module for single connection
	length = ESP8266_SetMultipleConnection(command, MAX_BUFFER_SIZE, 0);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);


	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}
	ESP8266_Delay(ESP_TIME_MS_COMMAND);

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_StatusNetwork(void)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	length = ESP8266_StatusConnection(command, MAX_BUFFER_SIZE);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_STATUS_OK);

	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_DisconnectAllNetwork(void)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	length = ESP8266_DisconnectAccessPoint(command, MAX_BUFFER_SIZE);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);

	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	ESP8266_Delay(ESP_TIME_MS_COMMAND);
	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_ConnectionNetwork(const ESP8266_NetworkParameters_s *parametersNetwork)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	length = ESP8266_JoinAccessPoint(command, MAX_BUFFER_SIZE, parametersNetwork);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);

	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_EstablichConnection(const ESP8266_ServerParameters_s *parameters)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	length = ESP8266_ConnectionServer(command, MAX_BUFFER_SIZE, parameters);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);


	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_MsgRequest(uint8_t *buffer, uint32_t length)
{
	uint32_t cmdLength = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t status = ESP8266_OK;

	if (buffer == NULL || length == 0)
	{
		return ESP8266_ERROR;
	}

	cmdLength = ESP8266_SendLengthData(command, MAX_BUFFER_SIZE, length);
	status = ESP8266_SendCommand(command, cmdLength, (const uint8_t *)AT_SEND_LENGTH_OK);

	if( status != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	ESP8266_Delay(500);
	status = ESP8266_SendData(buffer, length, (const uint8_t *)AT_SEND_OK);

	return status;
}

ESP8266_StatusTypeDef_t ESP8266_MsgReceiveData(uint8_t *buffer, uint32_t *length)
{
	uint32_t index, i = 0, lengthData = 0;
	uint8_t charRx = 0, lengthFrame[5];
	ESP8266_bool_t newFrame;

	/* Receive frame of data until we receive the last one.
	 *
	 * All frame start with string "+IPD,"
	 * Then receive length of frame, it's maximum value is 1460.
	 * Of the end receive data.
	 *
	 */
	*length = 0;
	strncpy((char *)bufferRx, "\0", MAX_BUFFER_SIZE);
	index = 0;
	newFrame = FALSE;

	while(1)
	{
		if( commInterface->recv(&charRx, 1) != 0 )
		{
			if( newFrame == TRUE )
			{
				if( lengthData-- )
				{
					*buffer++ = charRx;
					(*length)++;
				}
				else
				{
					strncpy((char *)bufferRx, "\0", MAX_BUFFER_SIZE);
					index = 0;
					newFrame = FALSE;
				}
			}
			bufferRx[index++] = charRx;
		}
		else
		{
			/* There is error while I'm reading return error */
			if (newFrame == TRUE && lengthData != 0)
			{
				return ESP8266_ERROR;
			}
			else
			{
				break;
			}
		}

		/* Check if there is overflow on buffer */
		if (index >= MAX_BUFFER_SIZE)
		{
			/* if there is a new frame and length of before frame isn't 0 return error */
			if (newFrame == TRUE && lengthData != 0)
			{
				return ESP8266_ERROR;
			}
			else
			{
				break;
			}
		}

		/* Check if there is a new frame of data */
		if(strstr((char *)bufferRx, (const char *)AT_STRING_IPD) != NULL && newFrame == FALSE)
		{
			// Extract the length of frame
			i = 0;
			strncpy((char *)lengthFrame, "\0", 5);
			do
			{
				commInterface->recv(&charRx, 1);
				lengthFrame[i++] = charRx;
			} while (charRx != ':' && i < 4);

			lengthData = atoi((char *)lengthFrame);
			newFrame = TRUE;
		}

		// Check if there is error
		if (strstr((char *)bufferRx, (const char *)AT_ERROR) != NULL)
		{
			return ESP8266_ERROR;
		}

		// Check is end of frame
		if (strstr((char *)bufferRx, (const char *)AT_IPD_STRING_OK) != NULL)
		{
			newFrame = FALSE;
		}
	}

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t ESP8266_Close(void)
{
	uint32_t length = 0;
	uint8_t command[MAX_BUFFER_SIZE];
	ESP8266_StatusTypeDef_t state;

	length = ESP8266_CloseConnection(command, MAX_BUFFER_SIZE);
	state = ESP8266_SendCommand(command, length, (const uint8_t *)AT_OK);


	if( state != ESP8266_OK )
	{
		return ESP8266_ERROR;
	}

	return ESP8266_OK;
}

ESP8266_StatusTypeDef_t GenerateRequest(void *data)
{
//	ESP8266_Datacompleted_s *information;
	ESP8266_StatusTypeDef_t returnValue = ESP8266_OK;
//
//	MQTTPacket_connectData dataConnection = MQTTPacket_connectData_initializer;
//	MQTTString topicString = MQTTString_initializer;
//	uint8_t sessionPresent, connack_rc, buffer[200];
//	uint32_t length = 0, retry = 0;
//	static uint8_t message = 64;
//
//	information = (ESP8266_Datacompleted_s *)data;
//
//
//	switch (information->state)
//	{
//		case CONNECTION_NETWORK:
//		{
//			returnValue = ESP8266_ConnectionNetwork(information->dataNetwork);
//
//			if( returnValue == ESP8266_OK )
//			{
//				information->state = STATUS_NETWORK;
//			}
//		}
//		break;
//
//		case STATUS_NETWORK:
//		{
//
//			returnValue = ESP8266_StatusNetwork();
//
//			if( returnValue != ESP8266_OK )
//			{
//				information->state = CONNECTION_NETWORK;
//			}
//			else
//			{
//				information->state = ESTABLISH_CONNECTION;
//			}
//		}
//		break;
//
//		case ESTABLISH_CONNECTION:
//		{
//
//			returnValue = ESP8266_EstablichConnection(information->dataServer);
//
//			if (returnValue != ESP8266_OK)
//			{
//				information->state = STATUS_NETWORK;
//			}
//			else
//			{
//				information->state = SEND_CONNECT_MQTT;
//			}
//		}
//		break;
//
//		case SEND_CONNECT_MQTT:
//		{
//			retry = 0;
//			while( retry < 2 )
//			{
//				dataConnection.MQTTVersion = 3;
//				dataConnection.clientID.cstring = "hanes";
//				dataConnection.keepAliveInterval = KEEPALIVE_CONNECTION;
//				dataConnection.will.qos = 0;
//				memset(buffer, 0, sizeof(buffer));
//				length = MQTTSerialize_connect(buffer, sizeof(buffer), &dataConnection);
//				returnValue = ESP8266_MsgRequest(buffer, length);
//
//				if( returnValue != ESP8266_OK )
//				{
//					if( strstr((char *)bufferRx, "CLOSED\r\n") != NULL )
//					{
//						retry = 5;
//						information->state = ESTABLISH_CONNECTION;
//					}
//					else
//					{
//						information->state = CLOSE_CONNECTION;
//						retry++;
//						continue;
//					}
//
//				}
//
//				memset(buffer, 0, sizeof(buffer));
//				length = 0;
//				returnValue =  ESP8266_MsgReceiveData(buffer, &length);
//
//				if( returnValue != ESP8266_OK )
//				{
//					information->state = CLOSE_CONNECTION;
//				}
//				else
//				{
//					if ( MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer, strlen((char *)buffer)) != 1 )
//					{
//						information->state = CLOSE_CONNECTION;
//					}
//					else
//					{
//						information->state = SEND_SUBCRIBE_MQTT;
//						break;
//					}
//				}
//
//				retry++;
//			}
//		}
//		break;
//
//		case SEND_SUBCRIBE_MQTT:
//		{
//			retry = 0;
//			while (retry < 2)
//			{
//				MQTTString topicSubcribeString = MQTTString_initializer;
//				topicSubcribeString.cstring = "SUB_ESP8266";
//				int32_t requestQoS = 0;
//				length = MQTTSerialize_subscribe(buffer, sizeof(buffer), 0, 1, 1, &topicSubcribeString, (int *)&requestQoS);
//				returnValue = ESP8266_MsgRequest(buffer, length);
//
//				if( returnValue != ESP8266_OK )
//				{
//					if( strstr((char *)bufferRx, "CLOSED\r\n") != NULL )
//					{
//						retry = 5;
//						information->state = ESTABLISH_CONNECTION;
//					}
//					else
//					{
//						information->state = CLOSE_CONNECTION;
//						retry++;
//						continue;
//					}
//				}
//
//				memset(buffer, 0, sizeof(buffer));
//				length = 0;
//				returnValue =  ESP8266_MsgReceiveData(buffer, &length);
//				if( returnValue != ESP8266_OK )
//				{
//					information->state = CLOSE_CONNECTION;
//				}
//				else
//				{
//					uint16_t subcribe_MsgID;
//					int32_t subcribeCount, granted_QoS;
//					if ( MQTTDeserialize_suback(&subcribe_MsgID, 1, (int *)&subcribeCount, (int *)&granted_QoS, buffer, strlen((char *)buffer)) != 1 )
//					{
//						information->state = CLOSE_CONNECTION;
//					}
//					else
//					{
//						information->state = SEND_PUBLISH_MQTT;
//						break;
//					}
//				}
//
//				retry++;
//			}
//		}
//		break;
//
//
//		case SEND_PUBLISH_MQTT:
//		{
//			topicString.cstring = "ESP8266";
//
//			length = MQTTSerialize_publish(buffer, sizeof(buffer), 0, 0, 0, 0, topicString, (unsigned char*)&message, 1);
//			returnValue = ESP8266_MsgRequest(buffer, length);
//			if( message == 122 )
//			{
//				message = 64;
//			}
//			message++;
//			if( returnValue != ESP8266_OK )
//			{
//				information->state = CLOSE_CONNECTION;
//				break;
//			}
//			else
//			{
//				information->state = RECEIVE_MESSAGE;
//			}
//
//		}
//		break;
//
//		case RECEIVE_MESSAGE:
//		{
//			memset(buffer, 0, sizeof(buffer));
//			returnValue =  ESP8266_MsgReceiveData(buffer, &length);
//			if (returnValue != ESP8266_OK)
//			{
//				information->state = CLOSE_CONNECTION;
//			}
//			else
//			{
//				if( strlen((char *)buffer) != 0 )
//				{
//					uint8_t granted_Dup, granted_Retained, *granted_Payload;
//					int32_t granted_QoS, granted_PayloadLen;
//					uint16_t granted_PacketID;
//					MQTTString granted_TopicString;
//
//					if( MQTTDeserialize_publish(&granted_Dup, (int *)&granted_QoS, &granted_Retained,
//												&granted_PacketID, &granted_TopicString, (unsigned char **)&granted_Payload,
//												(int *)&granted_PayloadLen, (unsigned char *)buffer, (int)granted_PayloadLen) == 1)
//					{
//						information->state = SEND_UNSUBCRIBE_MQTT;
//					}
//					else
//					{
//						information->state = CLOSE_CONNECTION;
//					}
//				}
//			}
//		}
//		break;
//
//		case SEND_UNSUBCRIBE_MQTT:
//		{
//			memset(buffer, 0, sizeof(buffer));
//			MQTTSerialize_disconnect(buffer, sizeof(buffer));
//			returnValue = ESP8266_MsgRequest(buffer, length);
//
//			information->state = CLOSE_CONNECTION;
//		}
//		break;
//
//		case CLOSE_CONNECTION:
//		{
//			if( returnValue != ESP8266_OK )
//			{
//				ESP8266_Close();
//			}
//			else
//			{
//				returnValue = ESP8266_Close();
//			}
//
//			information->state = STATUS_NETWORK;
//			HAL_Delay(5000);
//		}
//		break;
//
//		default:
//		{
//			information->state = STATUS_NETWORK;
//		}
//	}
//
//
	return returnValue;
}

