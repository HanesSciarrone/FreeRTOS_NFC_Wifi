/*
 * Task_NFC.c
 *
 *  Created on: Jun 9, 2020
 *      Author: hanes
 */
#include "string.h"

#include "Task_NFC.h"
#include "cmsis_os.h"
#include "NFC_SPI.h"
#include "NFC.h"

/* [BEGIN] Private typedef ------------ */
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* [END] Private typedef -------------- */


/* Definitions for TaskNFC */
osThreadId_t TaskNFCHandle;
uint32_t Buffer_TaskNFC[512];
osStaticThreadDef_t TaskNFC_ControlBlock;
const osThreadAttr_t TaskNFC_attributes = {
  .name = "TaskNFC",
  .stack_mem = &Buffer_TaskNFC[0],
  .stack_size = sizeof(Buffer_TaskNFC),
  .cb_mem = &TaskNFC_ControlBlock,
  .cb_size = sizeof(TaskNFC_ControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};


/* Definitions for uidNFC_Queue */
osMessageQueueId_t uidNFC_QueueHandle;
uint8_t uidNFC_Queue_Buffer[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t uidNFC_Queue_ControlBlock;
const osMessageQueueAttr_t uidNFC_Queue_attributes = {
  .name = "uidNFC_Queue",
  .cb_mem = &uidNFC_Queue_ControlBlock,
  .cb_size = sizeof(uidNFC_Queue_ControlBlock),
  .mq_mem = &uidNFC_Queue_Buffer,
  .mq_size = sizeof(uidNFC_Queue_Buffer)
};


/* Variable that link SPI function to NFC drivers */
static NFC_CommInterface commInterface_NFC;

/* [BEGIN]: Prototype of private function */

/**
 * \brief Function to initialize module NFC with PN532 and
 * start to RFID operation.
 *
 * \return Return 1 if operation was success or 0 in other case
 */
static uint8_t NFC_Module_Init(void);

/**
 * \brief Function to initialize peripheral functionality used in
 * module NFC.
 *
 * \return Return 1 if operation was success or 0 in other case.
 */
static uint8_t NFC_CommInterface_Init(void);

/**
 *	\brief Function to initialize Queue neccesary to operate with this task
 *
 *	\return Return 1 is operation was success or 0 in other case.
 */
static uint8_t NFC_QueueInit(void);

/**
* \brief Function implementing the CardNFC thread.
*
* \param argument: Argument to pass a task.
*
* \return None
*/
static void CardNFC(void *argument);

/* [END]: Prototype of private function */



static uint8_t NFC_Module_Init(void)
{
	uint32_t success;
//	uint8_t model, version, subversion;

	success = NFC_GetFirmwareVersion();

	if (success == 0)
	{
		return 0;
	}

//	model = (success & 0x00FF0000)>>16;
//	version = (success & 0x0000FF00) >> 8;
//	subversion = (success & 0x000000FF);

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

	return 1;
}

static uint8_t NFC_CommInterface_Init(void)
{
	commInterface_NFC.GetByte = &NFC_SPI_GetByte;
	commInterface_NFC.GetIRQ = &NFC_SPI_GetIRQ;
	commInterface_NFC.SendByte = &NFC_SPI_SendByte;
	commInterface_NFC.SetSelect = &NFC_SPI_SetSelect;

	return NFC_CommInit(&commInterface_NFC);
}

static uint8_t NFC_QueueInit(void)
{
	  /* creation of uidNFC_Queue */
	  uidNFC_QueueHandle = osMessageQueueNew (10, sizeof(uint8_t), &uidNFC_Queue_attributes);

	  if (uidNFC_QueueHandle == NULL)
	  {
		  return 0;
	  }

	  return 1;
}

static void CardNFC(void *argument)
{
	uint8_t uid[7] = {0, 0, 0, 0, 0, 0, 0}, length_uid;

	if (NFC_CommInterface_Init() == 0)
	{
		osThreadTerminate(TaskNFCHandle);
	}

	if (NFC_Module_Init() == 0)
	{
		osThreadTerminate(TaskNFCHandle);
	}

	/* Infinite loop */
	for(;;)
	{
		if (NFC_ReadPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &length_uid, 1000))
		{
			strncpy((char *)uid, "\0", 7);
			osDelay(1000/portTICK_PERIOD_MS);
		}

		osDelay(1/portTICK_PERIOD_MS);
	}

	// If task exit of while so destroy task. This is for caution
	osThreadTerminate(TaskNFCHandle);
}

int8_t TaskNCF_Started(void)
{
	/* Definition and creation of Queue */
	if (NFC_QueueInit() == 0)
	{
		return -1;
	}

	TaskNFCHandle = osThreadNew(CardNFC, NULL, &TaskNFC_attributes);

	if (TaskNFCHandle == NULL)
	{
		return -1;
	}

	return 1;
}
