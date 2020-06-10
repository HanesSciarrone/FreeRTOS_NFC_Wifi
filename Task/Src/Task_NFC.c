/*
 * Task_NFC.c
 *
 *  Created on: Jun 9, 2020
 *      Author: hanes
 */
#include "Task_NFC.h"
#include "cmsis_os.h"
#include "NFC.h"
#include "spi.h"

/* Variable to define task */
static osThreadId CardNFC_Handle;
static uint32_t BufferNFC[128];
static osStaticThreadDef_t CardNFC_ControlBlock;

/* Variable to define Queue of task NFC */
static osMessageQId QueueNFC_Handle;
static uint8_t Queue_BufferNFC[ 10 * sizeof( uint8_t ) ];
static osStaticMessageQDef_t NFC_QueueControlBlock;

/* Variable that link SPI function to NFC drivers */
static NFC_CommInterface commInterface_NFC;

/* BEGIN: Prototype of private function */
static uint8_t ModelNFC_Init(void);
static uint8_t CommNFC_Init(void);
static uint8_t TaskNFC_Init(void);
static void Task_CardNFC(void const * argument);
/* END: Prototype of private function */


/**
 * \brief Function to initialize module NFC with PN532.
 *
 * \return Return 1 if operation was success or 0 in other case
 */
static uint8_t ModelNFC_Init(void)
{
	uint32_t success;
	uint8_t model, version, subversion;

	success = NFC_GetFirmwareVersion();

	if (success == 0)
	{
		return 0;
	}

	model = (success & 0x00FF0000)>>16;
	version = (success & 0x0000FF00) >> 8;
	subversion = (success & 0x000000FF);

	// Set the max number of retry attempts to read from a card
	// This prevents us from waiting forever for a card, which is
	// the default behaviour of the PN532.
	success = NFC_SetPassiveActivationRetries(0xFF);
	if( success == 0)
	{
		return 0;
	}

	// configure board to read RFID tags
	success = NFC_SAMConfig();
	if( success == 0)
	{
		return 0;
	}
}


/**
 * \brief Function to initialize peripheral functionality used in
 * module NFC.
 *
 * \return Return 1 if operation was success or 0 in other case.
 */
static uint8_t CommNFC_Init(void)
{
	commInterface_NFC.GetByte;
	commInterface_NFC.GetIRQ;
	commInterface_NFC.SendByte;
	commInterface_NFC.SetSelect;

	return NFC_CommInit(&commInterface_NFC);
}


/**
 *	\brief Function to initialize Semaphore, Mutex and Queue neccesary to oparete with this task
 *
 *	\return Return 1 is operation was success or 0 in other case.
 */
static uint8_t TaskNFC_Init(void)
{
	  osMessageQStaticDef(QueueNFC, 10, uint8_t, Queue_BufferNFC, &NFC_QueueControlBlock);
	  QueueNFC_Handle = osMessageCreate(osMessageQ(QueueNFC), NULL);

	  if (CardNFC_Handle == NULL)
	  {
		  return 0;
	  }

	  return 1;
}


/**
* \brief Function implementing the CardNFC thread.
*
* \param argument: Argument to pass a task.
*
* \return None
*/
static void Task_CardNFC(void const * argument)
{
	uint8_t uid[7] = {0, 0, 0, 0, 0, 0, 0}, length_uid;

	if (CommNFC_Init() == 0)
	{
		osThreadTerminate(CardNFC_Handle);
	}

	if (ModelNFC_Init() == 0)
	{
		osThreadTerminate(CardNFC_Handle);
	}

	/* Infinite loop */
	for(;;)
	{
		if (NFC_ReadPassiveTargetID(PN532_MIFARE_ISO14443A, uid, length_uid, 1000))
		{
			osDelay(1000);
		}

		osDelay(1);
	}

	// If task exit of while so destroy task. This is for caution
	osThreadTerminate(CardNFC_Handle);
}

int8_t TaskNCF_Create(void)
{
	int8_t returnValue;

	/* Definition and creation of Queue, Mutex or Semaphore */
	if (TaskNFC_Init() == 0)
	{
		return -1;
	}

	osThreadStaticDef(CardNFC, Task_CardNFC, osPriorityNormal, 0, 128, BufferNFC, &CardNFC_ControlBlock);
	CardNFC_Handle = osThreadCreate(osThread(CardNFC), NULL);

	if (CardNFC_Handle == NULL)
	{
		return -1;
	}

	return 1;
}
