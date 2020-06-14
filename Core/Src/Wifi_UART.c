/*
 * Wifi_UART.c
 *
 *  Created on: 12 jun. 2020
 *      Author: hanes
 */
#include "Wifi_UART.h"

#ifdef WIFI_UART_RTOS
#include "cmsis_os.h"

typedef StaticSemaphore_t osStaticSemaphoreDef_t;

/* Definitions for wifi_Sem_ReceptionData */
osSemaphoreId_t wifi_Sem_ReceptionDataHandle;
osStaticSemaphoreDef_t Wifi_ReceptionData_ControlBlock;
const osSemaphoreAttr_t wifi_Sem_ReceptionData_attributes = {
  .name = "wifi_Sem_ReceptionData",
  .cb_mem = &Wifi_ReceptionData_ControlBlock,
  .cb_size = sizeof(Wifi_ReceptionData_ControlBlock),
};
#endif


#define true	(1)
#define false	(0)


#define UART_BUFFER_SIZE                         (1024*2)

typedef struct
{
  uint8_t  data[UART_BUFFER_SIZE];
  uint16_t index;
  uint16_t position;
}RxBuffer_t;

static RxBuffer_t bufferWifi;
UART_HandleTypeDef uartWifi;

uint8_t WIFI_UART_Init(void)
{
	uartWifi.Instance = UART5;
	uartWifi.Init.BaudRate = 115200;
	uartWifi.Init.WordLength = UART_WORDLENGTH_8B;
	uartWifi.Init.StopBits = UART_STOPBITS_1;
	uartWifi.Init.Parity = UART_PARITY_NONE;
	uartWifi.Init.Mode = UART_MODE_TX_RX;
	uartWifi.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uartWifi.Init.OverSampling = UART_OVERSAMPLING_16;
	uartWifi.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uartWifi.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&uartWifi) != HAL_OK)
	{
		return false;
	}

#ifdef WIFI_UART_RTOS
	/* Semaphore binary initialize blocked because second parameter is 0 */
	wifi_Sem_ReceptionDataHandle = osSemaphoreNew(UART_BUFFER_SIZE, 0, &wifi_Sem_ReceptionData_attributes);

	if (wifi_Sem_ReceptionDataHandle == NULL)
	{
		return false;
	}
#endif

	/* Once the WiFi UART is intialized, start an asynchrounous recursive
	   listening. the HAL_UART_Receive_IT() call below will wait until one char is
	   received to trigger the HAL_UART_RxCpltCallback(). The latter will recursively
	   call the former to read another char.  */
	bufferWifi.index = 0;
	bufferWifi.position = 0;

	HAL_UART_Receive_IT(&uartWifi, (uint8_t *)&bufferWifi.data[bufferWifi.index], 1);

	return true;
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(uartHandle->Instance==UART5)
	{
		/* UART5 clock enable */
		__HAL_RCC_UART5_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();

		/** UART5 GPIO Configuration
    	PC12     ------> UART5_TX
    	PD2     ------> UART5_RX
		 */
		GPIO_InitStruct.Pin = Wifi_Tx_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
		HAL_GPIO_Init(Wifi_Tx_GPIO_Port, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = Wifi_Rx_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
		HAL_GPIO_Init(Wifi_Rx_GPIO_Port, &GPIO_InitStruct);

		/* UART5 interrupt Init */
		HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(UART5_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
	if(uartHandle->Instance==UART5)
	{
		/* Peripheral clock disable */
		__HAL_RCC_UART5_CLK_DISABLE();

		/**UART5 GPIO Configuration
    	PC12     ------> UART5_TX
    	PD2     ------> UART5_RX
		 */
		HAL_GPIO_DeInit(Wifi_Tx_GPIO_Port, Wifi_Tx_Pin);

		HAL_GPIO_DeInit(Wifi_Rx_GPIO_Port, Wifi_Rx_Pin);

		/* UART5 interrupt Deinit */
		HAL_NVIC_DisableIRQ(UART5_IRQn);
	}
}

int8_t Wifi_UART_Send(uint8_t* data, uint32_t length)
{
#ifdef WIFI_UART_RTOS
	// Transmit on non blocking mode
	if (HAL_UART_Transmit_IT(&uartWifi, (uint8_t*)data, length) != HAL_OK)
	{
		return -1;
	}
#else
	// Transmit on blocking mode
	if (HAL_UART_Transmit(&uartWifi, (uint8_t*)data, length, DEFAULT_TIME_OUT) != HAL_OK)
	{
		return -1;
	}
#endif
	return 0;
}

uint32_t Wifi_UART_Receive(uint8_t* buffer, uint32_t length)
{
	uint32_t tickStart;
	uint32_t readData = 0;

#ifdef WIFI_UART_RTOS
//	osStatus_t status;

	while (length--)
	{
//		status = osSemaphoreAcquire(wifi_Sem_ReceptionDataHandle, DEFAULT_TIME_OUT/portTICK_PERIOD_MS);
//		if (status != osOK )
//		{
//			return readData;
//		}
		tickStart = xTaskGetTickCount();
		do
		{
			if(bufferWifi.position != bufferWifi.index)
			{
				/* serial data available, so return data to user */
				*buffer++ = bufferWifi.data[bufferWifi.position++];
				readData++;

				/* check for ring buffer wrap */
				if (bufferWifi.position >= UART_BUFFER_SIZE)
				{
					/* Ring buffer wrap, so reset head pointer to start of buffer */
					bufferWifi.position = 0;
				}
				break;
			}
		} while((xTaskGetTickCount() - tickStart ) < DEFAULT_TIME_OUT);

	}
#else
	/* Loop until data received */
	while (length--)
	{
		tickStart = HAL_GetTick();
		do
		{
			if(bufferWifi.position != bufferWifi.index)
			{
				/* serial data available, so return data to user */
				*buffer++ = bufferWifi.data[bufferWifi.position++];
				readData++;

				/* check for ring buffer wrap */
				if (bufferWifi.position >= UART_BUFFER_SIZE)
				{
					/* Ring buffer wrap, so reset head pointer to start of buffer */
					bufferWifi.position = 0;
				}
				break;
			}
		}while((HAL_GetTick() - tickStart ) < DEFAULT_TIME_OUT);
	}
#endif

	return readData;
}


/**
  * @brief  Rx Callback when new data is received on the UART.
  * @param  UartHandle: Uart handle receiving the data.
  * @retval None.
  */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uartHandle)
{
#ifdef WIFI_UART_RTOS
	BaseType_t task_more_priority = pdFALSE;
#endif
	/* If ring buffer end is reached reset tail pointer to start of buffer */
	if(++bufferWifi.index >= UART_BUFFER_SIZE)
	{
		bufferWifi.index = 0;
	}

	HAL_UART_Receive_IT(uartHandle, (uint8_t *)&bufferWifi.data[bufferWifi.index], 1);

#ifdef WIFI_UART_RTOS

	/* Internally this function create variable BaseType_t in pdFALSE, call xSemaphoreGiveFromISR
	 * and portYIELD_FROM_ISR with variable created, this happen if function is calling inside of
	 * interruption.
	 */
	portYIELD_FROM_ISR(task_more_priority);
	//osSemaphoreRelease(wifi_Sem_ReceptionDataHandle);
#endif
}
