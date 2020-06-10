/*
 * NFC.h
 *
 *  Created on: May 24, 2020
 *      Author: hanes
 */

#ifndef INC_NFC_H_
#define INC_NFC_H_

#include "stm32f7xx_hal.h"

/** \brief Comment #define if you don't work with RTOS */
#define NFC_RTOS


#define PN532_PREAMBLE 		(0x00)
#define PN532_STARTCODE1 	(0x00)
#define PN532_STARTCODE2 	(0xFF)
#define PN532_POSTAMBLE 	(0x00)

#define PN532_HOSTTOPN532 	(0xD4)
#define PN532_PN532TOHOST 	(0xD5)

/// PN532 Commands
#define PN532_COMMAND_DIAGNOSE 					(0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION 		(0x02)
#define PN532_COMMAND_GETGENERALSTATUS 			(0x04)
#define PN532_COMMAND_READREGISTER 				(0x06)
#define PN532_COMMAND_WRITEREGISTER 			(0x08)
#define PN532_COMMAND_READGPIO 					(0x0C)
#define PN532_COMMAND_WRITEGPIO 				(0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE 		(0x10)
#define PN532_COMMAND_SETPARAMETERS 			(0x12)
#define PN532_COMMAND_SAMCONFIGURATION 			(0x14)
#define PN532_COMMAND_POWERDOWN 				(0x16)
#define PN532_COMMAND_RFCONFIGURATION 			(0x32)
#define PN532_COMMAND_RFREGULATIONTEST 			(0x58)
#define PN532_COMMAND_INJUMPFORDEP 				(0x56)
#define PN532_COMMAND_INJUMPFORPSL 				(0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET 		(0x4A)
#define PN532_COMMAND_INATR 					(0x50)
#define PN532_COMMAND_INPSL 					(0x4E)
#define PN532_COMMAND_INDATAEXCHANGE 			(0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU 		(0x42)
#define PN532_COMMAND_INDESELECT 				(0x44)
#define PN532_COMMAND_INRELEASE 				(0x52)
#define PN532_COMMAND_INSELECT 					(0x54)
#define PN532_COMMAND_INAUTOPOLL 				(0x60)
#define PN532_COMMAND_TGINITASTARGET 			(0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES 		(0x92)
#define PN532_COMMAND_TGGETDATA 				(0x86)
#define PN532_COMMAND_TGSETDATA 				(0x8E)
#define PN532_COMMAND_TGSETMETADATA 			(0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND 	(0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR 	(0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS 		(0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE 			(0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET 		(0x4B)

#define PN532_WAKEUP 							(0x55)

#define PN532_SPI_STATREAD 						(0x02)
#define PN532_SPI_DATAWRITE 					(0x01)
#define PN532_SPI_DATAREAD 						(0x03)
#define PN532_SPI_READY 						(0x01)

#define PN532_I2C_ADDRESS 						(0x48 >> 1)
#define PN532_I2C_READBIT 						(0x01)
#define PN532_I2C_BUSY 							(0x00)
#define PN532_I2C_READY 						(0x01)
#define PN532_I2C_READYTIMEOUT 					(20)

#define PN532_MIFARE_ISO14443A 					(0x00)

/// Mifare Commands
#define MIFARE_CMD_AUTH_A 						(0x60)
#define MIFARE_CMD_AUTH_B 						(0x61)
#define MIFARE_CMD_READ 						(0x30)
#define MIFARE_CMD_WRITE 						(0xA0)
#define MIFARE_CMD_TRANSFER 					(0xB0)
#define MIFARE_CMD_DECREMENT 					(0xC0)
#define MIFARE_CMD_INCREMENT 					(0xC1)
#define MIFARE_CMD_STORE 						(0xC2)
#define MIFARE_ULTRALIGHT_CMD_WRITE 			(0xA2)

/// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE 					(0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT 				(0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT 			(0x02)
#define NDEF_URIPREFIX_HTTP 					(0x03)
#define NDEF_URIPREFIX_HTTPS 					(0x04)
#define NDEF_URIPREFIX_TEL 						(0x05)
#define NDEF_URIPREFIX_MAILTO 					(0x06)
#define NDEF_URIPREFIX_FTP_ANONAT 				(0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT 				(0x08)
#define NDEF_URIPREFIX_FTPS 					(0x09)
#define NDEF_URIPREFIX_SFTP 					(0x0A)
#define NDEF_URIPREFIX_SMB 						(0x0B)
#define NDEF_URIPREFIX_NFS 						(0x0C)
#define NDEF_URIPREFIX_FTP 						(0x0D)
#define NDEF_URIPREFIX_DAV 						(0x0E)
#define NDEF_URIPREFIX_NEWS 					(0x0F)
#define NDEF_URIPREFIX_TELNET 					(0x10)
#define NDEF_URIPREFIX_IMAP 					(0x11)
#define NDEF_URIPREFIX_RTSP 					(0x12)
#define NDEF_URIPREFIX_URN 						(0x13)
#define NDEF_URIPREFIX_POP 						(0x14)
#define NDEF_URIPREFIX_SIP 						(0x15)
#define NDEF_URIPREFIX_SIPS 					(0x16)
#define NDEF_URIPREFIX_TFTP 					(0x17)
#define NDEF_URIPREFIX_BTSPP 					(0x18)
#define NDEF_URIPREFIX_BTL2CAP 					(0x19)
#define NDEF_URIPREFIX_BTGOEP 					(0x1A)
#define NDEF_URIPREFIX_TCPOBEX 					(0x1B)
#define NDEF_URIPREFIX_IRDAOBEX 				(0x1C)
#define NDEF_URIPREFIX_FILE 					(0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID 				(0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG 				(0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT 				(0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW 				(0x21)
#define NDEF_URIPREFIX_URN_EPC 					(0x22)
#define NDEF_URIPREFIX_URN_NFC 					(0x23)


/// Size maximum of buffer to store data received from PN532
#define PN532_BUFFERSIZE 64

/**
 *  Structure to communicate with interface used to
 *  operate with PN532.
 */
typedef struct
{
	uint8_t (*GetByte)(void);		///< Pointer to function to receive single byte from PN532
	void (*SendByte)(uint8_t);		///< Pointer to function to sent single byte to PN532
	void (*SetSelect)(uint8_t);		///< Pointer to function to enable o disable interface communication
	uint8_t (*GetIRQ)(void);		///< Pointer to function to test pin IRQ of PN532
}NFC_CommInterface;


/**
 * \brief Function to initialize pointer of communication interface functionality.
 *
 * \param[in] interface Pointer to contain all functions of interface;
 *  \return Return 1 if operation was success or 0 the other way.
 */
uint8_t NFC_CommInit(NFC_CommInterface *interface);


/// Generic PN532 functions


/** \brief Return hardware and software version from PN532.
 * 	The hardware and software version from PN532 is stored in a 32 bit variable.
 *
 *  MSB byte has no relevant information.
 *
 *  Byte 2: Chip type: eg. PN5[32] in HEX
 * 	Byte 3: Version number before decimal point eg. [1].6
 *  Byte 4: Version number after decimal point e.g. 1.[6]
 *
 *  \return Return variable containing hardware and software version from PN532
 *  or 0 the other way.
 */
uint32_t NFC_GetFirmwareVersion(void);

/**
 * 	\brief Configures the SAM (Secure Access Module)
 *
 * 	\return Return 1 if everything executed properly, 0 for an error
 */
uint8_t NFC_SAMConfig(void);

/**
 * 	\brief Set retries of PN532. Sets the MxRtyPassiveActivation byte
 * 	 of the RFConfiguration register.
 *
 * 	 \param[in] maxRetries Maximum retries to read
 *
 * 	 \return Return 1 if everything executed properly, 0 for an error
 */
uint8_t NFC_SetPassiveActivationRetries(uint8_t maxRetries);


/// Functions to ISO14443A

/**
 * 	\brief Read UID of an ISO14443A card
 *
 * 	\param[in] card_			Baudrate Card baud rate. Possible value this field is:
 *
 * 								0x00 - 106 kbps type A (ISO/IEC14443 Type A)
 * 								0x01 : 212 kbps (FeliCa polling)
 * 								0x02 : 424 kbps (FeliCa polling)
 * 								0x03 : 106 kbps type B (ISO/IEC14443-3B)
 * 								0x04 : 106 kbps Innovision Jewel tag
 *
 * 	\param[in,out] uid 			Pointer to character array to store UID.
 * 	\param[in,out] length_uid	Pointer to variable to hold UID length (4 or 7)
 * 	\param[in] timeout			Timeout in mS default to allow PN532 to receive answer form card.
 *
 * 	\return Return state of real action. 1 was success, 0 failed
 */
uint8_t NFC_ReadPassiveTargetID(const uint8_t card_Baudrate, uint8_t *uid, uint8_t *length_uid, const uint16_t timeout);

#endif /* INC_NFC_H_ */
