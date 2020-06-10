/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 128 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId CardNFCHandle;
uint32_t BufferNFC[ 128 ];
osStaticThreadDef_t CardNFCControlBlock;
osThreadId LayerWifiHandle;
uint32_t BufferWifi[ 512 ];
osStaticThreadDef_t LayerWifiControlBlock;
osMessageQId QueueNFCHandle;
uint8_t Queue_BufferNFC[ 10 * sizeof( uint8_t ) ];
osStaticMessageQDef_t NFC_QueueControlBlock;
osMutexId Mutex_Wifi_NewMsgHandle;
osStaticMutexDef_t Wifi_NewMsg_ControlBlock;
osMutexId Wifi_Mutex_ReceiveDataHandle;
osStaticMutexDef_t Wifi_ReceiveData_ControlBlock;
osSemaphoreId Wifi_Semaphore_OperateHandle;
osStaticSemaphoreDef_t Wifi_Operation_ControlBlock;
osSemaphoreId Wifi_Sem_ReceptionDataHandle;
osStaticSemaphoreDef_t myCountingSem01ControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Task_CardNFC(void const * argument);
void Task_Wifi(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of Mutex_Wifi_NewMsg */
  osMutexStaticDef(Mutex_Wifi_NewMsg, &Wifi_NewMsg_ControlBlock);
  Mutex_Wifi_NewMsgHandle = osMutexCreate(osMutex(Mutex_Wifi_NewMsg));

  /* definition and creation of Wifi_Mutex_ReceiveData */
  osMutexStaticDef(Wifi_Mutex_ReceiveData, &Wifi_ReceiveData_ControlBlock);
  Wifi_Mutex_ReceiveDataHandle = osMutexCreate(osMutex(Wifi_Mutex_ReceiveData));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of Wifi_Semaphore_Operate */
  osSemaphoreStaticDef(Wifi_Semaphore_Operate, &Wifi_Operation_ControlBlock);
  Wifi_Semaphore_OperateHandle = osSemaphoreCreate(osSemaphore(Wifi_Semaphore_Operate), 1);

  /* definition and creation of Wifi_Sem_ReceptionData */
  osSemaphoreStaticDef(Wifi_Sem_ReceptionData, &myCountingSem01ControlBlock);
  Wifi_Sem_ReceptionDataHandle = osSemaphoreCreate(osSemaphore(Wifi_Sem_ReceptionData), 2048);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of QueueNFC */
  osMessageQStaticDef(QueueNFC, 10, uint8_t, Queue_BufferNFC, &NFC_QueueControlBlock);
  QueueNFCHandle = osMessageCreate(osMessageQ(QueueNFC), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of CardNFC */
  osThreadStaticDef(CardNFC, Task_CardNFC, osPriorityNormal, 0, 128, BufferNFC, &CardNFCControlBlock);
  CardNFCHandle = osThreadCreate(osThread(CardNFC), NULL);

  /* definition and creation of LayerWifi */
  osThreadStaticDef(LayerWifi, Task_Wifi, osPriorityHigh, 0, 512, BufferWifi, &LayerWifiControlBlock);
  LayerWifiHandle = osThreadCreate(osThread(LayerWifi), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Task_CardNFC */
/**
* @brief Function implementing the CardNFC thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_CardNFC */
void Task_CardNFC(void const * argument)
{
  /* USER CODE BEGIN Task_CardNFC */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_CardNFC */
}

/* USER CODE BEGIN Header_Task_Wifi */
/**
* @brief Function implementing the LayerWifi thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Wifi */
void Task_Wifi(void const * argument)
{
  /* USER CODE BEGIN Task_Wifi */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_Wifi */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
