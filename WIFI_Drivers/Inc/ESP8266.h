/*
 * ESP8266.h
 *
 *  Created on: Apr 18, 2020
 *      Author: hanes
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "stm32f7xx_hal.h"

/** \brief Comment this #define if you don't work with RTOS */
#define WIFI_RTOS

#define	ESP_TIME_MS_RESTART		2000
#define ESP_TIME_MS_COMMAND		500

typedef enum {
	  ESP8266_OK                            = 0,
	  ESP8266_ERROR                         = 1,
	  ESP8266_BUSY                          = 2,
	  ESP8266_ALREADY_CONNECTED             = 3,
	  ESP8266_CONNECTION_CLOSED             = 4,
	  ESP8266_TIMEOUT                       = 5,
}ESP8266_StatusTypeDef_t;


typedef enum {
	CONNECTION_NETWORK,
	STATUS_NETWORK,
	ESTABLISH_CONNECTION,
	SEND_CONNECT_MQTT,
	SEND_SUBCRIBE_MQTT,
	SEND_PUBLISH_MQTT,
	RECEIVE_MESSAGE,
	SEND_UNSUBCRIBE_MQTT,
	CLOSE_CONNECTION,
	STATUS_END
}ESP8266_StateRequest_t;

typedef struct
{
	uint8_t *ssid;
	uint8_t *password;
}ESP8266_NetworkParameters_s;

typedef struct
{
	uint8_t *protocol;
	uint8_t *host;
	uint16_t port;
}ESP8266_ServerParameters_s;

typedef struct
{
	ESP8266_NetworkParameters_s *dataNetwork;
	ESP8266_ServerParameters_s *dataServer;
	ESP8266_StateRequest_t state;
	uint8_t *data;
	uint32_t length;
}ESP8266_Datacompleted_s;


/**
 *  Structure of interface communication to operate with
 *  ESP8266
 */
typedef struct
{
	int8_t (*send)(uint8_t*, uint32_t);		///< Pointer to function to sent data to ESP8266
	uint32_t (*recv)(uint8_t*, uint32_t);	///< Pointer to function to receive data from ESP8266
}Wifi_CommInterface_s;


/**
 * \brief Initialization of function used to communicate ESP8266 with CPU.
 *
 * \return Return 0 if operation was success or 1 in other way
 */
ESP8266_StatusTypeDef_t ESP8266_CommInterface_Init(Wifi_CommInterface_s  *interface);

ESP8266_StatusTypeDef_t ESP8266_Init(void);
ESP8266_StatusTypeDef_t ESP8266_StatusNetwork(void);
ESP8266_StatusTypeDef_t ESP8266_DisconnectAllNetwork(void);
ESP8266_StatusTypeDef_t ESP8266_ConnectionNetwork(const ESP8266_NetworkParameters_s *parametersNetwork);
ESP8266_StatusTypeDef_t ESP8266_EstablichConnection(const ESP8266_ServerParameters_s *parameters);
ESP8266_StatusTypeDef_t ESP8266_MsgRequest(uint8_t *buffer, uint32_t length);
ESP8266_StatusTypeDef_t ESP8266_MsgReceiveData(uint8_t *buffer, uint32_t *length);
ESP8266_StatusTypeDef_t ESP8266_Close(void);
ESP8266_StatusTypeDef_t GenerateRequest(void *data);
#endif /* INC_ESP8266_H_ */
