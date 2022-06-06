#ifndef UART_COBS_SERVICE_H
#define UART_COBS_SERVICE_H
#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------
  Includes
----------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "uart_freertos.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"

/*----------------------------------------------------------------------
  Data type declarations
----------------------------------------------------------------------*/

typedef enum
{
	UART_COBS_POLLING,
	UART_COBS_INTERRUPT,
	UART_COBS_DMA
} uart_cobs_mode_t;

typedef struct __packed
{
	void* data;
	size_t size;
} uart_cobs_frame_t;

typedef struct __packed
{
	uart_freertos_t		*huart;
	size_t				max_frame_size;
	uint8_t				queue_depth;
	uart_cobs_mode_t	mode;
	QueueHandle_t		input_queue;
	QueueHandle_t		output_queue;
	SemaphoreHandle_t 	Transmit_Ready;
} uart_cobs_service_t;

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

/* send and receive of data */
size_t uart_cobs_send(uart_cobs_service_t* h, void* data, size_t size,
	TickType_t timeout);
size_t uart_cobs_recv(uart_cobs_service_t* h, void** data, TickType_t timeout);

/* task create */
osThreadId uart_cobs_service_rx_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, uart_cobs_service_t* h);
osThreadId uart_cobs_service_tx_create(char *name, osPriority priority,
	uint32_t instances, uint32_t stack_size, uart_cobs_service_t* h);

/* task routines */
void uart_cobs_service_rx_task(void const * argument);
void uart_cobs_service_tx_task(void const * argument);

/* task delete */
void uart_cobs_service_rx_free(void);
void uart_cobs_service_tx_free(void);

#ifdef __cplusplus
}
#endif
#endif /* UART_COBS_SERVICE_H */
