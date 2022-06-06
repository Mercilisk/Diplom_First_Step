/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/*	Подключение акселерометра к микроконтроллеру
 * 	Акселерометр 	STM32F103RB - NUCLEO_F103RB
 * 		GND					   GND
 * 		VCC				   CN07 - PIN16
 * 		CS				   CN10 - PIN17
 * 	   INT1				   ------------
 * 	   INT2				   ------------
 * 	  	SDO				   CN10 - PIN13
 * 		SDA				   CN10 - PIN15
 * 		SCL				   CN10 - PIN11
 *
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/*	HAL_FREERTOS	*/
#include 	"exti_freertos.h"
#include 	"uart_cobs_service.h"
#include 	"uart_freertos.h"
/*	Acceleration sensor	*/
#include 	"adxl345.h"
#include 	"adxl345_rtos.h"
/*	Task	*/
#include 	"Collector_Math_Task.h"

//#define 	Debug_Active

#ifdef 		Debug_Active
	#include 	"stdio.h"
	#include 	<string.h>
#endif
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
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

uart_cobs_service_t 		Cobs_UART;

adxl345_handle_t 			hadxl_settings;
adxl345_task_handle_t 		hadxl_task;
spi_freertos_t				spi_rtos;
User_Data_Settings_TypeDef  Setting_TypeDef;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
void Settings_Controll_Task(void const * argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, Settings_Controll_Task, osPriorityAboveNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin GPIO_Int2_Accelerometer_Pin */
  GPIO_InitStruct.Pin = B1_Pin|GPIO_Int2_Accelerometer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Green_Pin */
  GPIO_InitStruct.Pin = Green_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Green_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_Int1_Accelerometer_Pin */
  GPIO_InitStruct.Pin = GPIO_Int1_Accelerometer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO_Int1_Accelerometer_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */


/*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_Int1_Accelerometer_Pin)
	{
		adxl345_int2(GPIO_Int1_Accelerometer_Pin, NULL);
	}
}*/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_Settings_Controll_Task */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Settings_Controll_Task */
void Settings_Controll_Task(void const * argument)
{
  /* USER CODE BEGIN 5 */
	//-----------------------------------------------------------------------------------------------//
	//---------------------------Base Configuration Accelerometr Struct------------------------------//
	//---------------------------**************************************------------------------------//
	//										SPI CONFIG												 //
	//***********************************************************************************************//
	hadxl_settings.spi.spi_rtos			=	&spi_rtos;
	hadxl_settings.spi.spi_rtos->hspi	=	&hspi2;
	if(spi_freertos_init(hadxl_settings.spi.spi_rtos) == SPI_FREERTOS_EXIST)
	{
		Error_Handler();
	}
	hadxl_settings.spi.nss.pin			=	SPI_CS_Pin;
	hadxl_settings.spi.nss.port 		=	SPI_CS_GPIO_Port;
	//***********************************************************************************************//
	//							        Format Read Accelertion										 //
	//***********************************************************************************************//
	hadxl_settings.settings.align		=	ADXL345_ALIGN_LSB;
	hadxl_settings.settings.resolution	=	ADXL345_RESOLUTION_FULL;
	//***********************************************************************************************//
	//							   Settings FIFO Buffer in accelerometr								 //
	//***********************************************************************************************//
	hadxl_settings.
				settings.fifo_watermark =	0x25;
	hadxl_settings.settings.fifo_mode 	=	ADXL345_FIFO_FIFO;
	hadxl_settings.settings.fifo_trigger=	ADXL345_FIFO_TRIG_INT1;
	//***********************************************************************************************//
	//					Interrupt Settings, GPIO Configuration, FREERTOS Controll					 //
	//***********************************************************************************************//
	hadxl_settings.settings.int_en 		=	ADXL345_INT_WATERMARK | ADXL345_INT_OVERRUN;
	hadxl_settings.settings.map_to_int2	=  ~ADXL345_INT_WATERMARK & ADXL345_INT_OVERRUN;
	hadxl_settings.int1.pin 			=	GPIO_Int1_Accelerometer_Pin;
	hadxl_settings.int1.port			=	GPIO_Int1_Accelerometer_GPIO_Port;
	hadxl_settings.int2.pin 			=	GPIO_Int2_Accelerometer_Pin;
	hadxl_settings.int2.port			=	GPIO_Int2_Accelerometer_GPIO_Port;
	hadxl_settings.mutex_timeout 		=	100;
	hadxl_settings.transfer_timeout 	=	100;
	//---------------------------**************************************------------------------------//
	//----------------------------Start Configure Settigs Task Struct--------------------------------//
	//-----------------------------------------------------------------------------------------------//
	Setting_TypeDef.Length_Realization	=	0;
	Setting_TypeDef.FStatus 			= 	Enable_Time;
	Setting_TypeDef.hadxl 				=	&hadxl_task;
					 hadxl_task.hadxl 	=	hadxl_settings;
					 hadxl_task.
					 	 fifo_frame_size=	128;
					 hadxl_task.
					 	 fifo_frame_qty =	2;
	Setting_TypeDef.TempStatus 			=	Temp_Enable;
	Setting_TypeDef.X_Axis  			=	Disable;
	Setting_TypeDef.Y_Axis  			=	Disable;
	Setting_TypeDef.Z_Axis  			=	Disable;
	Setting_TypeDef.Signal_Data			=	NULL;
	Setting_TypeDef.Furie_Compl			=	NULL;
	Setting_TypeDef.Furie				=	NULL;
	Setting_TypeDef.Envelope			=	NULL;
	Setting_TypeDef.Status_Signal 		=	Outage;
	Setting_TypeDef.Status_Furie 		=	Outage;
	Setting_TypeDef.Status_Envelope 	=	Outage;
	Setting_TypeDef.Cobs_UART			=	&Cobs_UART;
	//-----------------------------------------------------------------------------------------------//
	//--------------------------Create Buffer for Receive Data Settings------------------------------//
	//-------------------------*****************************************-----------------------------//
#define Size_Data_Settings_in_Bytes (uint8_t) 8	/*	2 bytes on length realization
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on mod measurments
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on status activity x_axis
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on status activity y_axis
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on status activity z_axis
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on frequensy discretisation
	 	 	 	 	 	 	 	 	 	 	 	 *	1 bytes on range measure acceleration
	 	 	 	 	 	 	 	 	 	 	 	 */
	uint8_t Receive_Data_Settings[Size_Data_Settings_in_Bytes]
								  	  	=	{0};
	//***********************************************************************************************//
	//						   Configuration and create task reseive data							 //
	//***********************************************************************************************//
	Cobs_UART.huart						=	(uart_freertos_t*) pvPortMalloc(sizeof(uart_freertos_t));
	uart_freertos_init(					Cobs_UART.huart
					  );
	Cobs_UART.max_frame_size 			=	(size_t) Size_Data_Settings_in_Bytes;
	Cobs_UART.huart -> huart			=	(UART_HandleTypeDef *) &huart2;
	Cobs_UART.queue_depth 				=	3;
	Cobs_UART.mode 						=	UART_COBS_INTERRUPT;
	uart_cobs_service_rx_create(		"Task_uart_cobs_service_rx",
										osPriorityHigh,
										0,
										128,
										&Cobs_UART
								);
	//-------------------------******************************************----------------------------//
	//----------------------Brains Setting, Reseive, Create and Delete Tasks-------------------------//
	//-------------------------******************************************----------------------------//
	//									Start Configure Accelerometr 								 //
	//***********************************************************************************************//
	hadxl_settings.settings.data_rate	=	ADXL345_DATARATE_0_10_HZ;
	hadxl_settings.settings.meas_range	=	ADXL345_RANGE_2_G;

	//								НАП�?САТЬ КОД СТАРТОВОЙ КОНФ�?ГУРАЦ�?�?

	hadxl_settings.settings.offset.x	=	0;
	hadxl_settings.settings.offset.y	=	0;
	hadxl_settings.settings.offset.z 	=	0;
	/* Initializing ADXL345 */
	if(adxl345_init(&(hadxl_settings)) != ADXL345_OK)
		Error_Handler();
	//-----------------------------------------------------------------------------------------------//
	//----------------------------------Create Mutex Failed Memoru-----------------------------------//
	//-----------------------------------------------------------------------------------------------//
	Create_Mutex_Malloc_Failed(&Cobs_UART);
	//-------------------------******************************************----------------------------//
	while(1)
	{
		//uart_cobs_recv(&Cobs_UART,  (void **)  &Receive_Data_Settings, portMAX_DELAY);
		Read_Mutex_Malloc_Failed();
		//uint16_t *pLength_Relization 	=	(uint16_t *) 						Receive_Data_Settings;
		Setting_TypeDef.Length_Realization
										=	256;//*pLength_Relization;
		Setting_TypeDef.FStatus 		=	(Furie_Task_Status_TypeDef)   		0;//*(Receive_Data_Settings + 2);
		Setting_TypeDef.X_Axis 			=	(Measurement_Axis_Status_TypeDef)	1;//*(Receive_Data_Settings + 3);
		Setting_TypeDef.Y_Axis 			=	(Measurement_Axis_Status_TypeDef)	0;//*(Receive_Data_Settings + 4);
		Setting_TypeDef.Z_Axis 			=	(Measurement_Axis_Status_TypeDef)	0;//*(Receive_Data_Settings + 5);
		Setting_TypeDef.hadxl->hadxl.settings.data_rate
										=	(adxl345_data_rate_t) 				12;//*(Receive_Data_Settings + 6);
		Setting_TypeDef.hadxl->hadxl.settings.meas_range
										=	(adxl345_range_t) 					3;//*(Receive_Data_Settings + 7);
		//-----------------------------------------------------------------------------------------------//
		//                                         FREE MEMORY                                           //
		//---------------------------------***************************-----------------------------------//
		//                                        STOP INTERUPT                                    	     //
		//***********************************************************************************************//
		taskENTER_CRITICAL();
		//				SIGNALS_BUFFER 				//
		Free_RTOS_Buffer((const void *) Setting_TypeDef.Signal_Data);
		Free_RTOS_Buffer((const void *) Setting_TypeDef.Furie_Compl);
		Free_RTOS_Buffer((const void *) Setting_TypeDef.Furie);
		Free_RTOS_Buffer((const void *) Setting_TypeDef.Envelope);

		//***********************************************************************************************//
		//                                 DELETE UART TRANSMIT TASK                                     //
		//***********************************************************************************************//
		uart_cobs_service_tx_free();
		Free_RTOS_Buffer((const void *) Cobs_UART.input_queue);
		if (Cobs_UART.Transmit_Ready != NULL)
		{
			vSemaphoreDelete(Cobs_UART.Transmit_Ready);
		}
		//***********************************************************************************************//
		//                                 DELETE ACCELEROMETER TASK                                     //
		//***********************************************************************************************//
		adxl345_task_free();
		if (hadxl_task.fifo_frame_ptr_queue != NULL)
		{
			vQueueDelete(hadxl_task.fifo_frame_ptr_queue);
		}
		if (hadxl_task.suspend_notify != NULL)
		{
			vSemaphoreDelete(hadxl_task.suspend_notify);
		}
		//***********************************************************************************************//
		//                                    DELETE COLLECTOR TASK                                      //
		//***********************************************************************************************//
		Data_Collector_free();
		if (Setting_TypeDef.Status_Signal_Ready != NULL)
		{
			vSemaphoreDelete(Setting_TypeDef.Status_Signal_Ready);
		}
		//***********************************************************************************************//
		//                                      DELETE FURIE TASK                                        //
		//***********************************************************************************************//
		Furie_Transform_free();
		if (Setting_TypeDef.Status_Furie_Ready != NULL)
		{
			vSemaphoreDelete(Setting_TypeDef.Status_Furie_Ready);
		}
		//***********************************************************************************************//
		//                                  RESUME ENABLE INTERUPT                                       //
		//***********************************************************************************************//
		taskEXIT_CRITICAL();
		//-----------------------------------------------------------------------------------------------//
		// 						    Configuration and create task transmit data                          //
		//-----------------------------------------------------------------------------------------------//
		Cobs_UART.queue_depth 			=	3;
		Cobs_UART.max_frame_size		=	4 * Setting_TypeDef.Length_Realization;
		Cobs_UART.Transmit_Ready 		= 	xSemaphoreCreateBinary();
		uart_cobs_service_tx_create(	"Task_uart_cobs_service_tx",
										osPriorityNormal,
										0,
										128,
										&Cobs_UART
									);

		if(Read_Mutex_Malloc_Failed() == pdPASS)
		{
			continue;
		}
		//-----------------------------------------------------------------------------------------------//
		//                           CREATE TASK AND INFORMATION FOR SIGNAL                              //
		//-----------------------------------------------------------------------------------------------//
		hadxl_task.fifo_frame_qty	 	=	2;
		hadxl_task.fifo_frame_size		=	(size_t) 	(0.25 * Setting_TypeDef.Length_Realization /
													    /*----------------------------------------*/
																 hadxl_task.fifo_frame_qty);
		adxl345_task_create(			"Task_ADXL345_RTOS",
										osPriorityNormal,
										0,
										128,
										&hadxl_task);
		if(Read_Mutex_Malloc_Failed() == pdPASS)
		{
			continue;
		}
		Data_Collector_create(		"Task_Data_Collector_RTOS",
									osPriorityNormal,
									0,
									128,
									&Setting_TypeDef);
		if(Read_Mutex_Malloc_Failed() == pdPASS)
		{
			continue;
		}
		//-----------------------------------------------------------------------------------------------//
		//                           CREATE TASK AND INFORMATION FOR FURIE                               //
		//-----------------------------------------------------------------------------------------------//
		if (Setting_TypeDef.FStatus != Enable_Time)
		{
			Furie_Transform_create(		"Task_Furei_Calculation_RTOS",
										osPriorityNormal,
										0,
										128,
										&Setting_TypeDef);
		}
		osDelay(portMAX_DELAY);

	}
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
