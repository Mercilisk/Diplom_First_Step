#ifndef ADXL345_RTOS_H
#define ADXL345_RTOS_H

#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------
  Includes
----------------------------------------------------------------------*/

#include "adxl345.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/*----------------------------------------------------------------------
  Data type declarations
----------------------------------------------------------------------*/

typedef struct __packed
{
	adxl345_handle_t	hadxl;
	SemaphoreHandle_t	suspend_notify;
	uint8_t				fifo_frame_size;
	QueueHandle_t		fifo_frame_ptr_queue;
	uint8_t				fifo_frame_qty;
} adxl345_task_handle_t;

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

/* ADXL345 task create */
osThreadId adxl345_task_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size,
	adxl345_task_handle_t* htask);

/* ADXL345 task routine */
void adxl345_task(void const * argument);

BaseType_t adxl345_suspend(TickType_t timeout);
void adxl345_resume(void);
BaseType_t adxl345_wait_for_init(TickType_t timeout);

#ifdef __cplusplus
}
#endif
#endif /* ADXL345_RTOS_H */
