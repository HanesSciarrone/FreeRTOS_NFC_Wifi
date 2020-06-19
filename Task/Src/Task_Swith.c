/*
 * Task_Swith.c
 *
 *  Created on: Jun 19, 2020
 *      Author: hanes
 */

#include "cmsis_os.h"
#include "Task_Switch.h"
#include "Task_Wifi.h"

typedef StaticTask_t osStaticThreadDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;

/* Definitions for TaskNFC */
osThreadId_t TaskSwitchHandle;
uint32_t Buffer_TaskSwitch[64];
osStaticThreadDef_t TaskSwitch_ControlBlock;
const osThreadAttr_t TaskSwitch_attributes = {
  .name = "TaskSwitch",
  .stack_mem = &Buffer_TaskSwitch[0],
  .stack_size = sizeof(Buffer_TaskSwitch),
  .cb_mem = &TaskSwitch_ControlBlock,
  .cb_size = sizeof(TaskSwitch_ControlBlock),
  .priority = (osPriority_t) osPriorityBelowNormal7,
};

/* Definitions for swtich_Sem_Interrupt */
osSemaphoreId_t swtich_Sem_InterrutHandle;
osStaticSemaphoreDef_t swtich_Interrut_ControlBlock;
const osSemaphoreAttr_t switch_Sem_Interrupt_attributes = {
  .name = "switch_Sem_Interrupt",
  .cb_mem = &swtich_Interrut_ControlBlock,
  .cb_size = sizeof(swtich_Interrut_ControlBlock),
};


static uint8_t TaskSwitch_SignalSync_Init(void);
static void ModuleSwtich(void *argument);

static uint8_t TaskSwitch_SignalSync_Init(void)
{
	/* creation of swtich_Sem_Interrupt block state */
	if ((swtich_Sem_InterrutHandle = osSemaphoreNew(1, 0, &switch_Sem_Interrupt_attributes)) == NULL )
	{
		return 0;
	}

	return 1;
}

static void ModuleSwtich(void *argument)
{
	uint8_t message[4] = {100, 210, 125, 96};

	for(;;)
	{
		osSemaphoreAcquire(swtich_Sem_InterrutHandle, portMAX_DELAY);
		TaskWifi_MsgSendValidation(message, 4);
	}

	// If task exit of while so destroy task, semaphore, mutex, and queue. This is for caution
	osSemaphoreDelete(swtich_Sem_InterrutHandle);
	osThreadTerminate(TaskSwitchHandle);
}

int8_t TaskSwitch_Started(void)
{
	if (!TaskSwitch_SignalSync_Init())
	{
		return -1;
	}

	/* creation of TaskWifi */
	TaskSwitchHandle = osThreadNew(ModuleSwtich, NULL, &TaskSwitch_attributes);
	if (TaskSwitchHandle == NULL)
	{
		return -1;
	}

	return 1;
}
