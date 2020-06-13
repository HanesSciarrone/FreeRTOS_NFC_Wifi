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
//typedef StaticTask_t osStaticThreadDef_t;
//typedef StaticQueue_t osStaticMessageQDef_t;
//typedef StaticSemaphore_t osStaticMutexDef_t;
//typedef StaticSemaphore_t osStaticSemaphoreDef_t;
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 64 * 4
};
/* Definitions for TaskNFC */
//osThreadId_t TaskNFCHandle;
//uint32_t Buffer_TaskNFC[ 512 ];
//osStaticThreadDef_t TaskNFC_ControlBlock;
//const osThreadAttr_t TaskNFC_attributes = {
//  .name = "TaskNFC",
//  .stack_mem = &Buffer_TaskNFC[0],
//  .stack_size = sizeof(Buffer_TaskNFC),
//  .cb_mem = &TaskNFC_ControlBlock,
//  .cb_size = sizeof(TaskNFC_ControlBlock),
//  .priority = (osPriority_t) osPriorityNormal,
//};
/* Definitions for uidNFC_Queue */
//osMessageQueueId_t uidNFC_QueueHandle;
//uint8_t uidNFC_Queue_Buffer[ 10 * sizeof( uint8_t ) ];
//osStaticMessageQDef_t uidNFC_Queue_ControlBlock;
//const osMessageQueueAttr_t uidNFC_Queue_attributes = {
//  .name = "uidNFC_Queue",
//  .cb_mem = &uidNFC_Queue_ControlBlock,
//  .cb_size = sizeof(uidNFC_Queue_ControlBlock),
//  .mq_mem = &uidNFC_Queue_Buffer,
//  .mq_size = sizeof(uidNFC_Queue_Buffer)
//};
/* Definitions for wifi_Mutex_NewMsg */
//osMutexId_t wifi_Mutex_NewMsgHandle;
//osStaticMutexDef_t Wifi_NewMsg_ControlBlock;
//const osMutexAttr_t wifi_Mutex_NewMsg_attributes = {
//  .name = "wifi_Mutex_NewMsg",
//  .cb_mem = &Wifi_NewMsg_ControlBlock,
//  .cb_size = sizeof(Wifi_NewMsg_ControlBlock),
//};
/* Definitions for wifi_Mutex_ReceptionData */
//osMutexId_t wifi_Mutex_ReceptionDataHandle;
//osStaticMutexDef_t wifi_ReceptionData_ControlBlock;
//const osMutexAttr_t wifi_Mutex_ReceptionData_attributes = {
//  .name = "wifi_Mutex_ReceptionData",
//  .cb_mem = &wifi_ReceptionData_ControlBlock,
//  .cb_size = sizeof(wifi_ReceptionData_ControlBlock),
//};
/* Definitions for wifi_Sem_Operation */
//osSemaphoreId_t wifi_Sem_OperationHandle;
//osStaticSemaphoreDef_t Wifi_Operative_ControlBlock;
//const osSemaphoreAttr_t wifi_Sem_Operation_attributes = {
//  .name = "wifi_Sem_Operation",
//  .cb_mem = &Wifi_Operative_ControlBlock,
//  .cb_size = sizeof(Wifi_Operative_ControlBlock),
//};
/* Definitions for wifi_Sem_ReceptionData */
//osSemaphoreId_t wifi_Sem_ReceptionDataHandle;
//osStaticSemaphoreDef_t Wifi_ReceptionData_ControlBlock;
//const osSemaphoreAttr_t wifi_Sem_ReceptionData_attributes = {
//  .name = "wifi_Sem_ReceptionData",
//  .cb_mem = &Wifi_ReceptionData_ControlBlock,
//  .cb_size = sizeof(Wifi_ReceptionData_ControlBlock),
//};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
//void CardNFC(void *argument);
//
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of wifi_Mutex_NewMsg */
//  wifi_Mutex_NewMsgHandle = osMutexNew(&wifi_Mutex_NewMsg_attributes);

  /* creation of wifi_Mutex_ReceptionData */
//  wifi_Mutex_ReceptionDataHandle = osMutexNew(&wifi_Mutex_ReceptionData_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of wifi_Sem_Operation */
//  wifi_Sem_OperationHandle = osSemaphoreNew(1, 1, &wifi_Sem_Operation_attributes);

  /* creation of wifi_Sem_ReceptionData */
//  wifi_Sem_ReceptionDataHandle = osSemaphoreNew(2048, 2048, &wifi_Sem_ReceptionData_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of uidNFC_Queue */
//  uidNFC_QueueHandle = osMessageQueueNew (10, sizeof(uint8_t), &uidNFC_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TaskNFC */
//  TaskNFCHandle = osThreadNew(CardNFC, NULL, &TaskNFC_attributes);

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
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_CardNFC */
/**
* @brief Function implementing the TaskNFC thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CardNFC */
//void CardNFC(void *argument)
//{
//  /* USER CODE BEGIN CardNFC */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END CardNFC */
//}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
