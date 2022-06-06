#include 	"Collector_Math_Task.h"
#include 	"cobs.h"

uint16_t *First_Select_Axis_Data = NULL;
uint16_t *Second_Select_Axis_Data = NULL;

void Data_Collector_Task(void const * argument)
{
	User_Data_Settings_TypeDef *Setting_TypeDef
										= 	(User_Data_Settings_TypeDef *) argument;

	Setting_TypeDef -> Signal_Data 		=	(float*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(float));
	Setting_TypeDef -> Status_Signal_Ready
										=	xSemaphoreCreateBinary();
	adxl345_acc_data_t	*data_read;
	Status_Activ_AXIS Status 			=	One_Axis_Enable;
	Num_Axis_TypeDef  Number_Activ_Axis =	(Num_Axis_TypeDef) (Setting_TypeDef -> X_Axis +
												   	   	   	    Setting_TypeDef -> Y_Axis +
																Setting_TypeDef -> Z_Axis - 1);
	switch(Number_Activ_Axis)
	{
	case Two_Axis_Enable:
		First_Select_Axis_Data 			=	(uint16_t*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(uint16_t));
		break;
	case Three_Axis_Enable:
		Second_Select_Axis_Data			=	(uint16_t*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(uint16_t));
		break;
	default:
		break;
	}
	uint16_t Index_Count 				=	0;
	while(1)
	{
		adxl345_resume();
		xQueueReceive(Setting_TypeDef->hadxl->fifo_frame_ptr_queue, &data_read, portMAX_DELAY);
		if(Setting_TypeDef -> FStatus != Enable_Time)
		{
			xSemaphoreTake(Setting_TypeDef -> Status_Furie_Ready, portMAX_DELAY);
		}
		switch(Number_Activ_Axis)
		{
		case One_Axis_Enable:
			//-----------------------------------------------------------------------------------------------//
			//-----------------------------------COLLECTION WITCH X AXIS-------------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if (Setting_TypeDef -> X_Axis == Enable)
			{
				for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
									 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = adxl345_convert_float_mpss( (data_read + i) -> x);
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//-----------------------------------COLLECTION WITCH Y AXIS-------------------------------------//
			//-----------------------------------------------------------------------------------------------//
			else if (Setting_TypeDef -> Y_Axis == Enable)
			{
				for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
									 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = adxl345_convert_float_mpss( (data_read + i) -> y);
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//-----------------------------------COLLECTION WITCH Z AXIS-------------------------------------//
			//-----------------------------------------------------------------------------------------------//
			else
			{
				for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
									 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = adxl345_convert_float_mpss( (data_read + i) -> z);
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//---------------------------------ACTIV TRANSMIT AND CALCULATION--------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if ( Index_Count >= Setting_TypeDef -> Length_Realization)
			{
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreGive(Setting_TypeDef -> Status_Signal_Ready);
				}
				uart_cobs_send(Setting_TypeDef -> Cobs_UART,
							   Setting_TypeDef -> Signal_Data,
							   4 * Setting_TypeDef -> Length_Realization,
							   portMAX_DELAY);
				xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreTake(Setting_TypeDef -> Status_Furie_Ready, portMAX_DELAY);
				}

				Index_Count 		=	0;
			}
			break;
		case Two_Axis_Enable:
			//-----------------------------------------------------------------------------------------------//
			//------------------------------COLLECTION WITCH X AND Y OR Z AXIS-------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if (Setting_TypeDef -> X_Axis == Enable)
			{
				if (Status == One_Axis_Enable)
				{
					for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
										 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
					{
						*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss( (data_read + i) -> x);
						if (Setting_TypeDef -> Y_Axis == Enable)
						{
							*(First_Select_Axis_Data + Index_Count) 	=	(data_read + i) -> y;
						}
						else
						{
							*(First_Select_Axis_Data + Index_Count) 	=	(data_read + i) -> z;
						}
					}
				}
				else
				{
					for (; Index_Count < Setting_TypeDef -> Length_Realization; Index_Count++)
					{
						*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss(*(First_Select_Axis_Data + Index_Count));
					}
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//---------------------------------COLLECTION WITCH Y AND Z AXIS---------------------------------//
			//-----------------------------------------------------------------------------------------------//
			else
			{
				if (Status == One_Axis_Enable)
				{
					for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
										 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
					{
						*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss( (data_read + i) -> y);
						*(First_Select_Axis_Data + Index_Count) 		=	(data_read + i) -> z;
					}
				}
				else
				{
					for (; Index_Count < Setting_TypeDef -> Length_Realization; Index_Count++)
					{
						*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss(*(First_Select_Axis_Data + Index_Count));
					}
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//---------------------------------ACTIV TRANSMIT AND CALCULATION--------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if ( Index_Count >= Setting_TypeDef -> Length_Realization)
			{
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreGive(Setting_TypeDef -> Status_Signal_Ready);
				}
				uart_cobs_send(Setting_TypeDef -> Cobs_UART,
							   Setting_TypeDef -> Signal_Data,
							   4 * Setting_TypeDef -> Length_Realization,
							   portMAX_DELAY);
				xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreTake(Setting_TypeDef -> Status_Furie_Ready, portMAX_DELAY);
				}
				if (Status == One_Axis_Enable)
				{
					Status 				= 	Two_Axis_Enable;
				}
				else
				{
					Status 				= 	One_Axis_Enable;
				}
				Index_Count 			=	0;
			}
			break;
		case Three_Axis_Enable:
			//-----------------------------------------------------------------------------------------------//
			//-----------------------------COLLECTION WITCH X AND Y AND Z AXIS-------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if (Status == One_Axis_Enable)
			{
				for (uint16_t i = 0; i < Setting_TypeDef -> hadxl -> fifo_frame_size &&
									 Index_Count < Setting_TypeDef -> Length_Realization; i++, Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss( (data_read + i) -> x);
					if (Setting_TypeDef -> Y_Axis == Enable)
					{
						*(First_Select_Axis_Data + Index_Count) 	=	(data_read + i) -> y;
					}
					else
					{
						*(First_Select_Axis_Data + Index_Count) 	=	(data_read + i) -> z;
					}
				}
			}
			else if (Status == Two_Axis_Enable)
			{
				for (; Index_Count < Setting_TypeDef -> Length_Realization; Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss(*(First_Select_Axis_Data + Index_Count));
				}
			}
			else
			{
				for (; Index_Count < Setting_TypeDef -> Length_Realization; Index_Count++)
				{
					*(Setting_TypeDef -> Signal_Data + Index_Count) = 	adxl345_convert_float_mpss(*(Second_Select_Axis_Data + Index_Count));
				}
			}
			//-----------------------------------------------------------------------------------------------//
			//---------------------------------ACTIV TRANSMIT AND CALCULATION--------------------------------//
			//-----------------------------------------------------------------------------------------------//
			if ( Index_Count >= Setting_TypeDef -> Length_Realization)
			{
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreGive(Setting_TypeDef -> Status_Signal_Ready);
				}
				uart_cobs_send(Setting_TypeDef -> Cobs_UART,
							   Setting_TypeDef -> Signal_Data,
							   4 * Setting_TypeDef -> Length_Realization,
							   portMAX_DELAY);
				xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
				if(Setting_TypeDef -> FStatus != Enable_Time)
				{
					xSemaphoreTake(Setting_TypeDef -> Status_Furie_Ready, portMAX_DELAY);
				}
				if (Status == One_Axis_Enable)
				{
					Status 				= 	Two_Axis_Enable;
				}
				else if (Status == Two_Axis_Enable)
				{
					Status 				= 	Three_Axis_Enable;
				}
				else
				{
					Status 				= 	One_Axis_Enable;
				}
				Index_Count 			=	0;
			}
			break;
		}

	}

}

void Furie_Transform_Task(void const * argument)
{
	User_Data_Settings_TypeDef *Setting_TypeDef
										= 	(User_Data_Settings_TypeDef *) argument;

	Setting_TypeDef -> Furie_Compl 		=	(compl_t*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(compl_t));
	Setting_TypeDef -> Furie 			=	(float*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(float));
	if (Setting_TypeDef -> FStatus == Enable_Envelope_Furie)
	{
		Setting_TypeDef -> Envelope 	=	(float*) pvPortMalloc(Setting_TypeDef -> Length_Realization * sizeof(float));
	}
	Setting_TypeDef -> Status_Furie_Ready
										=	xSemaphoreCreateBinary();
	while(1)
	{
		xSemaphoreTake(Setting_TypeDef -> Status_Signal_Ready, portMAX_DELAY);
		switch(Setting_TypeDef -> FStatus)
		{
		case(Enable_Furie):
			SignalFastFurieTransformFREERTOS(Setting_TypeDef -> Signal_Data,
											 Setting_TypeDef -> Furie_Compl,
											 Setting_TypeDef -> Length_Realization,
											 Setting_TypeDef -> Status_Furie_Ready);
			for(uint16_t i; i < Setting_TypeDef -> Length_Realization; i++)
			{
				*(Setting_TypeDef -> Furie + i) = Compl_Abs((Setting_TypeDef -> Furie_Compl + i));
			}
			xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
			uart_cobs_send(Setting_TypeDef -> Cobs_UART,
						   Setting_TypeDef -> Furie,
						   4 * Setting_TypeDef -> Length_Realization,
						   portMAX_DELAY);
			xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
			break;
		case(Enable_Envelope_Furie):
			Envelope(Setting_TypeDef -> Envelope,
					 Setting_TypeDef -> Signal_Data,
					 Setting_TypeDef -> Furie_Compl,
					 Setting_TypeDef -> Length_Realization,
					 Setting_TypeDef -> Status_Furie_Ready);
		  	FastFurieTransform(Setting_TypeDef -> Furie_Compl, Setting_TypeDef -> Length_Realization);
			for(uint16_t i; i < Setting_TypeDef -> Length_Realization; i++)
			{
				*(Setting_TypeDef -> Furie + i) = Compl_Abs((Setting_TypeDef -> Furie_Compl + i));
			}
			xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
			uart_cobs_send(Setting_TypeDef -> Cobs_UART,
						   Setting_TypeDef -> Furie,
						   4 * Setting_TypeDef -> Length_Realization,
						   portMAX_DELAY);
			SignalFastFurieTransform(Setting_TypeDef -> Envelope,
									 Setting_TypeDef -> Furie_Compl,
									 Setting_TypeDef -> Length_Realization);
			for(uint16_t i; i < Setting_TypeDef -> Length_Realization; i++)
			{
				*(Setting_TypeDef -> Envelope + i) = Compl_Abs((Setting_TypeDef -> Furie_Compl + i));
			}
			xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
			uart_cobs_send(Setting_TypeDef -> Cobs_UART,
						   Setting_TypeDef -> Envelope,
						   4 * Setting_TypeDef -> Length_Realization,
						   portMAX_DELAY);
			xSemaphoreTake(Setting_TypeDef -> Cobs_UART -> Transmit_Ready, portMAX_DELAY);
			break;
		default:
			break;
		}
	}
}

osThreadId Data_Collector_task_id 		= 	NULL;

osThreadId Data_Collector_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, User_Data_Settings_TypeDef* Settings)
{

	/* create treads */
	osThreadDef_t thread = {
		.name		= name,
		.pthread	= Data_Collector_Task,
		.tpriority	= priority,
		.instances	= instances,
		.stacksize	= stack_size
	};

	Data_Collector_task_id 				= 	osThreadCreate(&thread, (void *) Settings);
	return Data_Collector_task_id;
}

osThreadId Furie_Transform_task_id 		= 	NULL;

osThreadId Furie_Transform_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, User_Data_Settings_TypeDef* Settings)
{

	/* create treads */
	osThreadDef_t thread = {
		.name		= name,
		.pthread	= Furie_Transform_Task,
		.tpriority	= priority,
		.instances	= instances,
		.stacksize	= stack_size
	};

	Furie_Transform_task_id 		=	osThreadCreate(&thread, (void *) Settings);
	return Furie_Transform_task_id;
}

/*********************************************************************************************/
/*						  		 	 Free Memoru Task Function								 */
/*********************************************************************************************/

void Data_Collector_free(void)
{
	Free_RTOS_Buffer(First_Select_Axis_Data);
	Free_RTOS_Buffer(Second_Select_Axis_Data);
	if (Data_Collector_task_id != NULL)
	{
		osThreadTerminate(Data_Collector_task_id);
		Data_Collector_task_id = NULL;
	}
}

void Furie_Transform_free(void)
{
	if (Furie_Transform_task_id != NULL)
	{
		osThreadTerminate(Furie_Transform_task_id);
		Furie_Transform_task_id = NULL;
	}
}

/*********************************************************************************************/
/*						  		 	Error Create Memory Function							 */
/*********************************************************************************************/
static SemaphoreHandle_t 		Mutex_Failed_Memoru;
static uart_cobs_service_t 		*COBS_UART_Transmit;

void vApplicationMallocFailedHook(void)
{
	const char Error_Messege[] = "Out of memory";
	uint8_t Error_Messege_COBS[14];
	size_t size = cobs_encode((uint8_t *) Error_Messege, (size_t) 12, (uint8_t *)  Error_Messege_COBS);
	Error_Messege_COBS[size++] = 0;
	uart_freertos_tx_it(COBS_UART_Transmit->huart, Error_Messege_COBS, 14,
					 	portMAX_DELAY, HAL_MAX_DELAY);
	xSemaphoreGive(Mutex_Failed_Memoru);
}

void Create_Mutex_Malloc_Failed(uart_cobs_service_t *uart_cobs_service)
{
	COBS_UART_Transmit 	=	uart_cobs_service;
	Mutex_Failed_Memoru = 	xSemaphoreCreateMutex();
}

BaseType_t Read_Mutex_Malloc_Failed(void)
{
	return(xSemaphoreTake(Mutex_Failed_Memoru, 0));
}

/*********************************************************************************************/
/*						  		 	   Free Buffers Function								 */
/*********************************************************************************************/

void Free_RTOS_Buffer(const void *pBuff)
{
	if (pBuff != NULL)
	{
		vPortFree((void *) pBuff);
	}
}
