/*
 * Task_Display.c
 *
 *  Created on: Jun 18, 2020
 *      Author: hanes
 */
#include "cmsis_os.h"
#include "string.h"

#include "Task_Display.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_sdram.h"

/* Typedef private ---------------------------------------------------------- */
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;


/* Definitions for Task Display */
osThreadId_t TaskDisplayHandle;
uint32_t Buffer_TaskDisplay[512];
osStaticThreadDef_t TaskDisplay_ControlBlock;
const osThreadAttr_t TaskDisplay_attributes = {
  .name = "TaskDisplay",
  .stack_mem = &Buffer_TaskDisplay[0],
  .stack_size = sizeof(Buffer_TaskDisplay),
  .cb_mem = &TaskDisplay_ControlBlock,
  .cb_size = sizeof(TaskDisplay_ControlBlock),
  .priority = (osPriority_t) osPriorityBelowNormal7,
};

/* Definitions for Display_Queue */
osMessageQueueId_t display_QueueHandle;
uint8_t display_Queue_Buffer[ 5 * sizeof( uint8_t ) ];
osStaticMessageQDef_t display_Queue_ControlBlock;
const osMessageQueueAttr_t display_Queue_attributes = {
  .name = "Display_Queue",
  .cb_mem = &display_Queue_ControlBlock,
  .cb_size = sizeof(display_Queue_ControlBlock),
  .mq_mem = &display_Queue_Buffer,
  .mq_size = sizeof(display_Queue_Buffer)
};

/* Definitions for wifi_Mutex_NewMsg */
osMutexId_t display_Mutex_NewMsgHandle;
osStaticMutexDef_t display_NewMsg_ControlBlock;
const osMutexAttr_t display_Mutex_NewMsg_attributes = {
  .name = "Display_Mutex_NewMsg",
  .cb_mem = &display_NewMsg_ControlBlock,
  .cb_size = sizeof(display_NewMsg_ControlBlock),
};

/* Private define ------------------------------------------------------------*/
#define LAYER0_ADDRESS               (LCD_FB_START_ADDRESS)
#define SIZE_BUFFER_MESSAGE			 50

/* Private function prototypes -----------------------------------------------*/

static void Display_Show(uint8_t *msg);

/**
 * @brief Function to initialize display, set layer and set color background
 *
 * @retval Return 1 if was success in other case 0
 */
static uint8_t Display_Init(void);

/**
* @brief Function implementing the TaskDisplay thread.
* @param argument: Argument to pass a task.
* @retval None
*/
static void Display(void *argument);

/**
 * @brief Function to initialize Queue, Semaphore and Mutex.
 *
 * @retval 1 if operation was success or 0 in other case.
 */
static uint8_t TaskDisplay_SignalSync_Init(void);

/* Private variables ---------------------------------------------------------*/
uint8_t msgDisplay[SIZE_BUFFER_MESSAGE];


static void Display_Show(uint8_t *msg)
{
	uint8_t msgPart1[30], msgPart2[300];
	uint8_t *ptr = NULL, length = 0, character = ';';

	if (msg == NULL)
	{
		return;
	}

	length = strlen((char *)msg);			// Get length of string
	strncpy((char *)msgPart1, "\0", 20);	// Initialize with 0
	strncpy((char *)msgPart2, "\0", 20);	// Initialize with 0

	ptr = (uint8_t *)strstr((char *)msg, (char *)&character);			// Search character ;

	BSP_LCD_Clear(LCD_COLOR_BLUE);
	BSP_LCD_SetBackColor(LCD_COLOR_GRAY);
	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	BSP_LCD_FillRect(200, 190, 400, 100);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&Font24);

	if( ptr != NULL)
	{
		strncpy((char *)msgPart1, (char *)msg, ptr-msg);
		ptr++; // Delete character ';'
		strncpy((char *)msgPart2, (char *)ptr, length-(ptr-msg));
		BSP_LCD_DisplayStringAt(0, LINE(9), msgPart1, CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, LINE(11), msgPart2, CENTER_MODE);
	}
	else
	{
		strncpy((char *)msgPart1, (char *)msg, length);
		BSP_LCD_DisplayStringAt(0, LINE(10), msgPart1, CENTER_MODE);
	}

}

static uint8_t Display_Init(void)
{
	if (BSP_LCD_Init() != LCD_OK)
	{
		return 0;
	}

	BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
	BSP_LCD_SelectLayer(0);

	BSP_LCD_Clear(LCD_COLOR_BLUE);
	BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
	BSP_LCD_SetTextColor(LCD_COLOR_TRANSPARENT);
	BSP_LCD_FillRect(0, 0, 800, 480);
	return 1;
}

static void Display(void *argument)
{
	Display_TypeMsg_t type;

	for(;;)
	{
		osMessageQueueGet(display_QueueHandle, &type, 0U, portMAX_DELAY);

		switch (type)
		{
			case MESSAGE_ERROR_NFC:
			{
				Display_Show((uint8_t *)"Problem with NFC");
			}
			break;

			case MESSAGE_ERROR_WIFI:
			{
				Display_Show((uint8_t *)"Problem with WIFI");
			}
			break;

			case MESSAGE_PROCESS_INFO:
			{
				Display_Show((uint8_t *)"Processing information");
			}
			break;

			case MEESAGE_CONNECT_NETWORK:
			{
				Display_Show((uint8_t *)"Connecting network");
			}
			break;

			case MESSAGE_CONNECTION:
			{
				Display_Show((uint8_t *)"Connected to network");
			}
			break;

			case MESSAGE_REQ_SERVER:
			{
				Display_Show((uint8_t *)"Send message;Please Wait");
			}
			break;

			case MESSAGE_RESP_SERVER:
			{
				Display_Show((uint8_t *)"Waiting response;of service");
			}
			break;

			case MESSAGE_PROCESS_AGAIN:
			{
				Display_Show((uint8_t *)"Please, enter ID;again");
			}
			break;

			case MESSAGE_RESPONSE:
			{
				Display_Show(msgDisplay);
			}
			break;

			default:
				Display_Show((uint8_t *)"Ready");
		}


		osDelay(1);
	}

	// If task exit of while so destroy task, semaphore, mutex, and queue. This is for caution
	osMessageQueueDelete(display_QueueHandle);
	osMutexDelete(display_Mutex_NewMsgHandle);
	osThreadTerminate(TaskDisplayHandle);
}

static uint8_t TaskDisplay_SignalSync_Init(void)
{
	/* creation of wifi_Queue */
	if ((display_QueueHandle = osMessageQueueNew (5, sizeof(uint8_t), &display_Queue_attributes)) == NULL)
	{
		return 0;
	}

	/* creation of wifi_Mutex_NewMsg */
	if ((display_Mutex_NewMsgHandle = osMutexNew(&display_Mutex_NewMsg_attributes)) == NULL)
	{
		return 0;
	}

	return 1;
}

int8_t TaskDisplay_Started(void)
{
	if (!Display_Init())
	{
		return -1;
	}

	if (!TaskDisplay_SignalSync_Init())
	{
		return -1;
	}

	TaskDisplayHandle = osThreadNew(Display, NULL, &TaskDisplay_attributes);

	if (TaskDisplayHandle == NULL)
	{
		return -1;
	}

	return 1;
}

void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

void Display_MessageStart(void)
{
	  BSP_LCD_Clear(LCD_COLOR_BLUE);
	  BSP_LCD_SetBackColor(LCD_COLOR_GRAY);
	  BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	  BSP_LCD_FillRect(200, 200, 400, 80);
	  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	  BSP_LCD_SetFont(&Font24);
	  BSP_LCD_DisplayStringAt(0, LINE(9), (uint8_t *)"Starting Software.", CENTER_MODE);
}

void Display_MsgShow(uint8_t typeMsg)
{
	osMessageQueuePut(display_QueueHandle, &typeMsg, 0U, 0);
}

void Display_MsgShowResponse(uint8_t *msg, uint32_t length)
{
	uint8_t typeMsg = MESSAGE_RESPONSE;

	osMutexAcquire(display_Mutex_NewMsgHandle, portMAX_DELAY);

	strncpy((char *)msgDisplay, "\0", SIZE_BUFFER_MESSAGE);
	strncpy((char *)msgDisplay, (char *)msg, length);
	osMessageQueuePut(display_QueueHandle, &typeMsg, 0U, 0);

	osMutexRelease(display_Mutex_NewMsgHandle);
}
