
#ifndef INC_TASK_INCLUDE_COLLECTOR_MATH_TASK_H_
#define INC_TASK_INCLUDE_COLLECTOR_MATH_TASK_H_

/*	HAL_FREERTOS	*/
#include 	"stm32f1xx_hal.h"
#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"
#include 	"cmsis_os.h"
#include 	"uart_cobs_service.h"
#include 	"uart_freertos.h"
/*	Acceleration sensor	*/
#include 	"adxl345.h"
#include 	"adxl345_rtos.h"
/*	Furie include 	*/
#include 	"FFT_FREERTOS.h"

/*********************************************************************************************/
/*								Data Type Declaration 										 */
/*********************************************************************************************/
typedef enum
{
	Enable_Time							=	0,
	Enable_Furie 							 ,
	Enable_Envelope_Furie
}Furie_Task_Status_TypeDef;
typedef enum
{
	Temp_Disable 						=	0,
	Temp_Enable
}Temperature_Measure_Status_TypeDef;
typedef enum
{
	Disable 							=	0,
	Enable
}Measurement_Axis_Status_TypeDef;
typedef enum
{
	Outage 								=	0,
	Transmit_Transaction
}Status_Transmit_Data_TypeDef;
typedef enum
{
	One_Axis_Enable 					=	0,
	Two_Axis_Enable 						 ,
	Three_Axis_Enable
}Num_Axis_TypeDef;

typedef Num_Axis_TypeDef Status_Activ_AXIS;
typedef struct
{
	// 	Settings and Information for Devise
	uint16_t 							Length_Realization;
	Furie_Task_Status_TypeDef  			FStatus;
	Temperature_Measure_Status_TypeDef  TempStatus;
	//	Status Active Axis Measurements
	Measurement_Axis_Status_TypeDef		X_Axis;
	Measurement_Axis_Status_TypeDef		Y_Axis;
	Measurement_Axis_Status_TypeDef		Z_Axis;
	//	Pointer on Buffer
	float 								*Signal_Data;
	compl_t								*Furie_Compl;
	float 								*Furie;
	float 								*Envelope;
	// 	Status Transmit Data
	Status_Transmit_Data_TypeDef 		Status_Signal;
	Status_Transmit_Data_TypeDef 		Status_Furie;
	Status_Transmit_Data_TypeDef 		Status_Envelope;
	//	Pointer on Transaction Protocol
	uart_cobs_service_t 				*Cobs_UART;
	//	Acceleration Task Settings
	adxl345_task_handle_t 				*hadxl;
	//	Ready Buffer Type_Def
	SemaphoreHandle_t 					Status_Signal_Ready;
	SemaphoreHandle_t 					Status_Furie_Ready;
}User_Data_Settings_TypeDef;

/*********************************************************************************************/
/*								    Task Declaration 										 */
/*********************************************************************************************/
void 		Data_Collector_Task(void const * argument);

osThreadId 	Data_Collector_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, User_Data_Settings_TypeDef* Settings);

void 		Furie_Transform_Task(void const * argument);

osThreadId 	Furie_Transform_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, User_Data_Settings_TypeDef* Settings);

/*********************************************************************************************/
/*						  		 	 Free Memoru Task Function								 */
/*********************************************************************************************/

void 		Data_Collector_free(void);
void 		Furie_Transform_free(void);

/*********************************************************************************************/
/*						   Error Create Memory Function Declaration 						 */
/*********************************************************************************************/
void 		vApplicationMallocFailedHook(void);

void 		Create_Mutex_Malloc_Failed(uart_cobs_service_t *uart_cobs_service);
BaseType_t 	Read_Mutex_Malloc_Failed(void);

/*********************************************************************************************/
/*						      Free Buffers Function Declaration 							 */
/*********************************************************************************************/

void 		Free_RTOS_Buffer(const void *pBuff);

#endif /* INC_TASK_INCLUDE_COLLECTOR_MATH_TASK_H_ */
