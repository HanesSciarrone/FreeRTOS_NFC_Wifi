/*
 * NFC.c
 *
 *  Created on: May 24, 2020
 *      Author: hanes
 */

#include <string.h>
#include "NFC.h"

#ifdef NFC_RTOS
#include "cmsis_os.h"
#endif

#define true	(1)
#define false	(0)

static uint8_t pn532ack[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
static uint8_t pn532response_firmwarevers[6] = {0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5};
static uint8_t pn532_buffer[PN532_BUFFERSIZE];
static NFC_CommInterface *commInterface;

static void NFC_Delay(const uint32_t time);
static void NFC_ReadData(uint8_t *buffer, uint32_t amount);
static void NFC_WriteCommand(uint8_t *cmd, uint16_t cmd_length);
static uint8_t NFC_IsReady(void);
static uint8_t NFC_WaitReady(const uint16_t timeout);
static uint8_t NFC_ReadACK(void);
static uint8_t NFC_SendCommandCheckAck(uint8_t *cmd, const uint16_t cmd_length, const uint16_t timeout);

static void NFC_Delay(const uint32_t time)
{
	#ifdef NFC_RTOS
		osDelay(time);
	#else
		uint32_t startTick = HAL_GetTick();
		while(HAL_GetTick() - startTick < time);
	#endif
}

static void NFC_ReadData(uint8_t *buffer, uint32_t amount)
{
	uint32_t i;

	// Enable PN532 and wait 1 mS
	commInterface->SetSelect(true);
	NFC_Delay(1);

	// Send message to PN532 to request information
	commInterface->SendByte(PN532_SPI_DATAREAD);

	for (i = 0; i < amount; i++)
	{
		buffer[i] = commInterface->GetByte();	// put data in buffer at position i
	}

	// Disable PN532
	commInterface->SetSelect(false);

}

static void NFC_WriteCommand(uint8_t *cmd, uint16_t cmd_length)
{
	uint8_t checksum;	// variable to store checksum
	uint16_t i;

	cmd_length++;	// increment command length for TFI

	// Enable PN532 and wait 1 mS
	commInterface->SetSelect(true);
	NFC_Delay(1);

	checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;

	commInterface->SendByte(PN532_SPI_DATAWRITE);	// inform SPI port of PN532 that data will be written
	commInterface->SendByte(PN532_PREAMBLE);		// Send preamble
	commInterface->SendByte(PN532_STARTCODE1);		// Send start of packet 1/2
	commInterface->SendByte(PN532_STARTCODE2);		// Send start of packet 2/2
	commInterface->SendByte(cmd_length);			// Send packet length (LEN)
	commInterface->SendByte(~cmd_length + 1);		// Send packet length checksum
	commInterface->SendByte(PN532_HOSTTOPN532);		// Inform PN532 that data direction is from host to PN532 (TFI)

	checksum += PN532_HOSTTOPN532;

	for (i = 0; i < cmd_length-1; i++)
	{
		commInterface->SendByte(cmd[i]);			// Send data to PN532
		checksum += cmd[i];
	}

	commInterface->SendByte(~checksum);				// Send checksum (DCS)
	commInterface->SendByte(PN532_POSTAMBLE);		// Send postamble

	commInterface->SetSelect(false);
}

static uint8_t NFC_IsReady(void)
{
	if( !commInterface->GetIRQ() )
	{
		return false;
	}

	return true;
}

static uint8_t NFC_WaitReady(const uint16_t timeout)
{
	#ifdef NFC_RTOS

		uint16_t currentTime = 0;
		// For as long as the timeout is reached try to detect ready signal from PN532
		do
		{
			if (NFC_IsReady())
			{
				return true;
			}
			osDelay(1);
			currentTime++;
		} while(currentTime < timeout);
	#else
		uint32_t startTick = HAL_GetTick();

		// For as long as the timeout is reached try to detect ready signal from PN532
		do
		{
			if (NFC_IsReady())
			{
				return true;
			}
		} while (HAL_GetTick() - startTick < timeout);
	#endif

	return false;
}

static uint8_t NFC_ReadACK(void)
{
	uint8_t ackBuffer[6];		// Array to store read message form PN532

	NFC_ReadData(ackBuffer, 6);	// Read message form PN532

	// compare message with ACK message syntax and return true when ACK is received.
	return !strncmp((const char *)ackBuffer, (const char *)pn532ack, 6) ? true : false;
}

static uint8_t NFC_SendCommandCheckAck(uint8_t *cmd, const uint16_t cmd_length, const uint16_t timeout)
{

	// write the command
	NFC_WriteCommand(cmd, cmd_length);

	// Wait for chip to say its ready!
	if ( !NFC_WaitReady(timeout) )
	{
		return false;
	}

	// read acknowledge
	if (!NFC_ReadACK())
	{
		return false;
	}

	// Wait for chip to say its ready!
	if ( !NFC_WaitReady(timeout) )
	{
		return false;
	}

	return true;
}



uint8_t NFC_CommInit(NFC_CommInterface *interface)
{
	if (interface == NULL)
	{
		return false;
	}

	commInterface = interface;
	return true;
}

uint32_t NFC_GetFirmwareVersion(void)
{
	uint32_t response = 0;
	uint8_t offset = 0;

	pn532_buffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;	// Set buffer position 0 with command

	// Send command with length of 1 byte, when result is false byte is not send.
	if (!NFC_SendCommandCheckAck(pn532_buffer, 1, 2))
	{
		return false;
	}

	// Data was successfully send now retrieve data read data packet
	NFC_ReadData(pn532_buffer, 12);

	// When buffer is not equal firmware version return
	if (strncmp((char *)pn532_buffer, (char *)pn532response_firmwarevers, 6) != 0)
	{
		return false;
	}


	/* Shift relevant data from response in to uint32_t,
	 * while SPI is used though conditional operator select
	 * a shift of 6 positions
	 */
	 offset = 6;	 						// Skip a response byte when using I2C to ignore extra data.
	 response = pn532_buffer[offset++];		// Position 6 of pn532_packetbuffer into response
	 response <<= 8;							// Shift response 8 bits left
	 response |= pn532_buffer[offset++];	// Operation OR position 7 of pn532_packetbuffer with response
	 response <<= 8;							// Shift response 8 bits left
	 response |= pn532_buffer[offset++];	// Operation OR position 8 of pn532_packetbuffer with response
	 response <<= 8;							// Shift response 8 bits left
	 response |= pn532_buffer[offset];		// Operation OR position 9 of pn532_packetbuffer with response

	 return response;
}

uint8_t NFC_SAMConfig(void)
{
	pn532_buffer[0] = PN532_COMMAND_SAMCONFIGURATION;
	pn532_buffer[1] = 0x01;		// Normal Mode
	pn532_buffer[2] = 0x14;		// Timeout 50ms * 20 = 1 second
	pn532_buffer[3] = 0x01;		// P70_IRQ used

	// send message
	if (!NFC_SendCommandCheckAck(pn532_buffer, 4, 2))
	{
		return false;
	}

	// Read data
	NFC_ReadData(pn532_buffer, 8);

	return (pn532_buffer[6] == 0x15) ? true : false;
}

uint8_t NFC_SetPassiveActivationRetries(uint8_t maxRetries)
{
	pn532_buffer[0] = PN532_COMMAND_RFCONFIGURATION;
	pn532_buffer[1] = 0x05;		// Config item 5 (MaxRetries)
	pn532_buffer[2] = 0xFF;		// MxRtyATR (default = 0xFF)
	pn532_buffer[3] = 0x01;		// MxRtyPSL (default = 0x01)
	pn532_buffer[4] = maxRetries;

	if (!NFC_SendCommandCheckAck(pn532_buffer, 5, 2))
	{
		return false;
	}

	return true;
}

uint8_t NFC_ReadPassiveTargetID(const uint8_t card_Baudrate, uint8_t *uid, uint8_t *length_uid, const uint16_t timeout)
{
	pn532_buffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pn532_buffer[1] = 0x01;	// Maximum 1 card at once (We can set a 0x02 as maximum)
	pn532_buffer[2] = card_Baudrate;

	// Send command, command length and timeout.
	if (!NFC_SendCommandCheckAck(pn532_buffer, 3, timeout))
	{
		return false; // No cards read
	}

	// Wait for a card to enter the field (only possible with I2C)
	if (!NFC_WaitReady(timeout))
	{
		return false;
	}

	// Read data of packet
	NFC_ReadData(pn532_buffer, 20);

	/* ISO14443A card response should be in the following format:

	    byte            Description
	    -------------   ------------------------------------------
	    b0..6           Frame header and preamble
	    b7              Tags Found
	    b8              Tag Number (only one used in this example)
	    b9..10          SENS_RES
	    b11             SEL_RES
	    b12             NFCID Length
	    b13..NFCIDLen   NFCID                                      */

	// Test for the number of tags found. Should be 1!
	if (pn532_buffer[7] != 1)
	{
		return false;	// No tags found, end of read, return false
	}

	/* Card appears to be Mifare Classic */
	*length_uid = pn532_buffer[12];	// set value of NFCID length to uidLength

	// Move UID to packet buffer
	for (uint8_t i=0; i < pn532_buffer[12]; i++)
	{
		uid[i] = pn532_buffer[13+i];
	}

	return true; // return success as card is read.
}


