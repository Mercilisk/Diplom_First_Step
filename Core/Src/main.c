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

#define 	Debug_Active

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

#define Length_Realization 		((uint16_t) 1024)

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

uart_cobs_service_t 	Cobs_UART;

adxl345_handle_t 		ADXL;
adxl345_task_handle_t 	hadxl;
spi_freertos_t			spi_rtos;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
void ADXL345_Data_Collector_Task(void const * argument);

/* USER CODE BEGIN PFP */
void ADXL345_Config(void);
void UART_Cobs_Config(void);
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
  osThreadDef(defaultTask, ADXL345_Data_Collector_Task, osPriorityNormal, 0, 1536);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ADXL345_Config();
  //UART_Cobs_Config();
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
void ADXL345_Config()
{

	ADXL.spi.spi_rtos							=	&spi_rtos;
	ADXL.spi.spi_rtos->hspi 					=	&hspi2;
	if(spi_freertos_init(ADXL.spi.spi_rtos) == SPI_FREERTOS_EXIST)
	{
		Error_Handler();
	}
	ADXL.spi.nss.pin							=	SPI_CS_Pin;
	ADXL.spi.nss.port 							=	SPI_CS_GPIO_Port;
	ADXL.settings.data_rate 					=	ADXL345_DATARATE_3200_HZ;
	ADXL.settings.meas_range					=	ADXL345_RANGE_16_G;
	ADXL.settings.align							=	ADXL345_ALIGN_LSB;
	ADXL.settings.resolution 					=	ADXL345_RESOLUTION_FULL;
	//	Offset Parameter
	ADXL.settings.offset.x 						=	0;
	ADXL.settings.offset.y 						=	0;
	ADXL.settings.offset.z 						=	0;
	//	Interrupt Settings
	ADXL.settings.int_en 						=	/*ADXL345_INT_DATA_READY;//*/ ADXL345_INT_WATERMARK | ADXL345_INT_OVERRUN;
	ADXL.settings.map_to_int2 					=	/*ADXL345_INT_DATA_READY;//*/~ADXL345_INT_WATERMARK & ADXL345_INT_OVERRUN;
	ADXL.int1.pin 								=	GPIO_Int1_Accelerometer_Pin;
	ADXL.int1.port								=	GPIO_Int1_Accelerometer_GPIO_Port;
	ADXL.int2.pin 								=	GPIO_Int2_Accelerometer_Pin;
	ADXL.int2.port								=	GPIO_Int2_Accelerometer_GPIO_Port;
	ADXL.settings.fifo_watermark 				=	/*0x00; //*/0x25;
	ADXL.settings.fifo_mode 					=	/*ADXL345_FIFO_BYPASS; //*/ADXL345_FIFO_FIFO;
	ADXL.settings.fifo_trigger 					=	ADXL345_FIFO_TRIG_INT1;
	ADXL.mutex_timeout 							=	100;
	ADXL.transfer_timeout 						=	100;

	//	Configurate FREE RTOS AND ADXL345

	hadxl.hadxl 								=	ADXL;
	hadxl.fifo_frame_size 						=	128;
	hadxl.fifo_frame_qty 						=	8;
	adxl345_task_create(						"Task_ADXL345_RTOS",
												osPriorityNormal,
												0,
												512,
												&hadxl);

/*#ifdef 	Debug_Active
	if (adxl345_init(&ADXL) != ADXL345_OK)
	{
		perror("Error: Accelerometer is not found");
		while(1)
		{
		}
	}
#else*/
	/*	Find Accelerometer ADXL345	*/
	/*while(adxl345_init(&ADXL) != ADXL345_OK)
	{
	}
#endif*/

}

void UART_Cobs_Config(void)
{

	uart_cobs_service_tx_create(					"Task_uart_cobs_service_tx",
													osPriorityNormal,
													0,
													256,
													&Cobs_UART
								);
	Cobs_UART.huart 								=	(uart_freertos_t*) pvPortMalloc(sizeof(uart_freertos_t));
	uart_freertos_init(								Cobs_UART.huart
					  );
	Cobs_UART.max_frame_size 						=	(size_t) Length_Realization;
	Cobs_UART.huart -> huart						=	(UART_HandleTypeDef *) &huart2;
	Cobs_UART.queue_depth 							=	1;
	Cobs_UART.mode 									=	UART_COBS_INTERRUPT;

}

/*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_Int1_Accelerometer_Pin)
	{
		adxl345_int2(GPIO_Int1_Accelerometer_Pin, NULL);
	}
}*/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_ADXL345_Data_Collector_Task */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_ADXL345_Data_Collector_Task */
void ADXL345_Data_Collector_Task(void const * argument)
{
  /* USER CODE BEGIN 5 */
	float Signal[256];
	adxl345_acc_data_t	data[128], data_read;
	uint16_t Index_Count;
Start_Mesurments:

	Index_Count 					= 	0;
	//adxl345_start(&ADXL);

	/* Infinite loop */
	while(Index_Count <= Length_Realization)
	{
		adxl345_resume();
		xQueueReceive(hadxl.fifo_frame_ptr_queue, &data_read, portMAX_DELAY);

		memcpy (&data, &data_read, sizeof(data));

		for (uint16_t i = 0; i<127; i++)
		{
			Signal[i] = adxl345_convert_float_mpss(data[i].y);
		}
		Index_Count++;
	}

	//uart_cobs_send(&Cobs_UART, &Signal, Length_Realization, 10 * portTICK_PERIOD_MS);
	goto Start_Mesurments;
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
