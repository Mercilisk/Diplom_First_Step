#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "adxl345_rtos.h"
#include "adxl345.h"
#include "exti_freertos.h"

typedef enum
{
	ADXL345_EXTI1	= (1U<<0),
	ADXL345_EXTI2	= (1U<<1),
	ADXL345_SUSPEND = (1U<<2),
	ADXL345_RESUME	= (1U<<3)
} adxl345_event_t;

typedef enum
{
	ADXL345_RUN,
	ADXL345_STANDBY
} adxl345_power_state_t;

static void adxl345_int1(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken);
static void adxl345_int2(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken);
static osThreadId adxl345_task_id;
static adxl345_task_handle_t* htask;

void adxl345_task(void const * argument)
{	
	htask = (adxl345_task_handle_t *) argument;
	adxl345_power_state_t state = ADXL345_STANDBY;

	uint32_t event;
	uint8_t sample_idx = 0;
	uint8_t frame_idx = 0;
	uint8_t i = 0;
	
	/* Initializing ADXL345 */
	if(adxl345_init(&(htask->hadxl)) != ADXL345_OK)
		Error_Handler();

	/* Allocating FIFO memory  */
	adxl345_acc_data_t *fifo = pvPortMalloc(htask->fifo_frame_size*
		htask->fifo_frame_qty*sizeof(adxl345_acc_data_t));
	if(fifo == NULL)
		Error_Handler();
	adxl345_acc_data_t *ptr_to_send = NULL;
	
	/* Flush ADXL345 internal FIFO */
	if(adxl345_fifo_flush(&(htask->hadxl)) != ADXL345_OK)
		Error_Handler();
	/* Suspend acc */
	if(adxl345_stop(&(htask->hadxl)) != ADXL345_OK)
						Error_Handler();
	
	/* Registering interrupt handlers */
	if(exti_freertos_register(htask->hadxl.int1.pin, adxl345_int1) != pdTRUE)
		Error_Handler();
/*	if(exti_freertos_register(htask->hadxl.int2.pin, adxl345_int2) != pdTRUE)
		Error_Handler();
*/
	/* Notify than init complete */
	xSemaphoreGive(htask->suspend_notify);

	/* Event loop */
	while(1)
	{
		xTaskNotifyWait(pdFALSE, UINT32_MAX, &event, portMAX_DELAY);

		switch(state)
		{
		case ADXL345_RUN:
			/* Suspend request */
			if(event & ADXL345_SUSPEND)
			{
				/* Stop ADXL345 */
				if(adxl345_stop(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				/* Flush ADXL345 internal FIFO */
				if(adxl345_fifo_flush(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				xSemaphoreGive(htask->suspend_notify);
				state = ADXL345_STANDBY;
				break;
			}
			/* ADXL345 internal FIFO overflow */
			if(event & ADXL345_EXTI2)
			{
				if(!adxl345_get_int_src(&(htask->hadxl)) & ADXL345_INT_OVERRUN)
					break;
				/* Stop ADXL345 */
				if(adxl345_stop(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				/* Flush ADXL345 internal FIFO */
				if(adxl345_fifo_flush(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				/* ADXL345 hard fault */
				if(!(event & ADXL345_EXTI1))
					if(adxl345_init(&(htask->hadxl)) != ADXL345_OK)
						Error_Handler();
				/* Start ADXL345 again */
				if(adxl345_start(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				if(adxl345_get_int_src(&(htask->hadxl)) & ADXL345_INT_OVERRUN)
					xTaskNotify(adxl345_task_id, ADXL345_EXTI2, eSetBits);
				break;
			}
			/* ADXL345 internal FIFO watermark is exceeded */
			if(event & ADXL345_EXTI1)
			{
				/* Get burst of data from ADXL345 internal FIFO */
				for(i = 0; i < htask->hadxl.settings.fifo_watermark; i++)
					adxl345_get_data(&(htask->hadxl),
						&(fifo[frame_idx*htask->hadxl.settings.fifo_watermark +
							sample_idx++]));
				/* Generate interrupt event if FIFO watermark is still exceeded */
				if(adxl345_get_int_src(&(htask->hadxl)) & ADXL345_INT_WATERMARK)
					xTaskNotify(adxl345_task_id, ADXL345_EXTI1, eSetBits);
				/* Send frame pointer to queue if frame is full, switch to next */
				if(sample_idx >= htask->fifo_frame_size)
				{
					sample_idx = 0;
					ptr_to_send = &(fifo[frame_idx*htask->hadxl.settings.fifo_watermark]);
					xQueueSend(htask->fifo_frame_ptr_queue, &ptr_to_send, 0);
					if(++frame_idx >= htask->fifo_frame_qty)
						frame_idx = 0;
				}
			}
			break;
		case ADXL345_STANDBY:
			/* Resume request */
			if(event & ADXL345_RESUME)
			{
				/* Start ADXL345 */
				if(adxl345_start(&(htask->hadxl)) != ADXL345_OK)
					Error_Handler();
				state = ADXL345_RUN;
			}
			break;
		}
	}
}

osThreadId adxl345_task_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size,
	adxl345_task_handle_t* htask)
{
	/* create semaphores, mutexes and queues */
	htask->fifo_frame_ptr_queue = xQueueCreate(htask->fifo_frame_qty,
		sizeof(adxl345_acc_data_t*));
	htask->suspend_notify = xSemaphoreCreateBinary();
	
	/* create tread */
	osThreadDef_t thread = {
		.name		= name,
		.pthread	= adxl345_task,
		.tpriority	= priority,
		.instances	= instances,
		.stacksize	= stack_size
	};

	adxl345_task_id = osThreadCreate(&thread, (void *) htask);
	return adxl345_task_id;
}

BaseType_t adxl345_suspend(TickType_t timeout)
{
	xTaskNotify(adxl345_task_id, ADXL345_SUSPEND, eSetBits);
	return xSemaphoreTake(htask->suspend_notify, timeout);
}

BaseType_t adxl345_wait_for_init(TickType_t timeout)
{
	return xSemaphoreTake(htask->suspend_notify, timeout);
}

void adxl345_resume(void)
{
	xTaskNotify(adxl345_task_id, ADXL345_RESUME, eSetBits);
}

static void adxl345_int1(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken)
{
	xTaskNotifyFromISR(adxl345_task_id, ADXL345_EXTI1, eSetBits,
		pxHigherPriorityTaskWoken);
}


static void adxl345_int2(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken)
{
	xTaskNotifyFromISR(adxl345_task_id, ADXL345_EXTI2, eSetBits,
		pxHigherPriorityTaskWoken);
}
