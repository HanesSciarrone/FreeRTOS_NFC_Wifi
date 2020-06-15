/*
 * ESP8266.h
 *
 *  Created on: Apr 18, 2020
 *      Author: hanes
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#define WIFI_RTOS

//  Time of wait to start read response of command send.
#define TIME_MS_MSG_RESPONSE	250

typedef enum {
	  ESP8266_OK                            = 0,
	  ESP8266_ERROR                         = 1,
	  ESP8266_BUSY                          = 2,
	  ESP8266_ALREADY_CONNECTED             = 3,
	  ESP8266_CONNECTION_CLOSED             = 4,
	  ESP8266_TIMEOUT                       = 5,
}ESP8266_StatusTypeDef_t;

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

/**
 *  Structure of communicate interface used to
 *  operate with ESP8266.
 */
typedef struct
{
	int8_t(*send)(uint8_t*, uint32_t);		///< Pointer to function to sent data to ESP8266
	int32_t(*recv)(uint8_t*, uint32_t);		///< Pointer to function to receive data from ESP8266
}ESP8266_CommInterface_s;


ESP8266_StatusTypeDef_t ESP8266_CommInterface_Init(ESP8266_CommInterface_s *interface);
ESP8266_StatusTypeDef_t ESP8266_SetEcho(const uint8_t mode);
ESP8266_StatusTypeDef_t ESP8266_SetModeWIFI(uint8_t *mode);
ESP8266_StatusTypeDef_t ESP8266_SetMultipleConnection(uint8_t *mode);
ESP8266_StatusTypeDef_t ESP8266_StatusNetwork(void);
ESP8266_StatusTypeDef_t ESP8266_DisconnectAllNetwork(void);
ESP8266_StatusTypeDef_t ESP8266_ConnectionNetwork(const ESP8266_NetworkParameters_s *network);
ESP8266_StatusTypeDef_t ESP8266_ConnectionServer(const ESP8266_ServerParameters_s *service);
ESP8266_StatusTypeDef_t ESP8266_SentData(uint8_t *buffer, uint32_t length);
ESP8266_StatusTypeDef_t ESP8266_ReceiveData(uint8_t *buffer, uint32_t *length);
ESP8266_StatusTypeDef_t ESP8266_ConnectionClose(void);

ESP8266_StatusTypeDef_t ESP8266_Close(void);
//ESP8266_StatusTypeDef_t GenerateRequest(void *data);
#endif /* INC_ESP8266_H_ */
