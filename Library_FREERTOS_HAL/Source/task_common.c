/* task_common source file - edit for customize */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"
#include "cmsis_os.h"

#include "main.h"

#include "cmsis_os.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

#include "task_common.h"

#include "io/crc_freertos.h"
#include "io/eeprom_freertos.h"
#include "io/gpio_freertos.h"
#include "io/spi_freertos.h"
#include "io/uart_freertos.h"
#include "io/adc_freertos.h"
#include "iot/lorawan/loramac_broker.h"
#include "timer.h"
#include "adxl345_rtos.h"
#include "vibrov_rtos.h"
#include "vibrov_system_task.h"
#include "board.h"
#include "rtc-board.h"
adc_freertos_t adc_rtos = {.hadc = &hadc1};

static spi_freertos_t spi2_rtos = {.hspi = &hspi2};
static spi_freertos_t spi3_rtos = {.hspi = &hspi3};

static uart_freertos_t uart2_rtos = {.huart = &huart2};
#ifdef SERIAL_DEBUG
#include "protocol/serial_terminal.h"
static uart_freertos_t uart3_rtos = {.huart = &huart3};
#endif

static loramac_broker_task_handle_t loramac_htask = LORAMAC_HTASK;

static const iir_float_coeff_t filter_coeff[FILTER_NUM_SECTIONS] = FILTER_COEFF;

static adxl345_task_handle_t adxl345_htask = ADXL345_HTASK;
static vibrov_task_handle_t vibrov_htask = VIBROV_HTASK;
static vibrov_system_task_handle_t vibrov_system_htask = VIBROV_SYSTEM_HTASK;

#ifdef SERIAL_DEBUG
static serial_terminal_t term = {
	.term = &uart3_rtos,
	.mode = SERIAL_TERMINAL_POLLING,
	.stdin_buf_size = 0,
	.stdout_buf_size = 80,
	.stderr_buf_size = 0,
	.mutex_timeout = portMAX_DELAY,
	.transfer_timeout = portMAX_DELAY};
#endif

extern RTC_HandleTypeDef RtcHandlel;
/* Common tasks init */
void task_common_init(void)
{
	spi_freertos_init(&spi2_rtos);
	spi_freertos_init(&spi3_rtos);
	uart_freertos_init(&uart2_rtos);
	adc_freertos_init(&adc_rtos);
	crc_freertos_init();

#ifdef SERIAL_DEBUG
	uart_freertos_init(&uart3_rtos);
	serial_terminal_init(&term);
#endif
	
	loramac_broker_task_create("loramac_broker_routine",
		osPriorityNormal, 0, 768, &loramac_htask);

	adxl345_task_create("adxl345_routine",
			osPriorityNormal, 0, 128, &adxl345_htask);

	vibrov_htask.adxl_queue = adxl345_htask.fifo_frame_ptr_queue;
	vibrov_task_create("vibrov_routine", osPriorityAboveNormal, 0, 512,
		&vibrov_htask);

	vibrov_system_htask.vibrov_handle = &vibrov_htask;
	vibrov_system_task_create("vsystem_routine", osPriorityHigh, 0, 128,
		&vibrov_system_htask);

	timer_task_create("tim", osPriorityHigh, 0, 256);
}

void peripherial_deinit(void)
{
	HAL_SPI_DeInit(spi2_rtos.hspi);
	HAL_SPI_DeInit(spi3_rtos.hspi);
	HAL_UART_DeInit(uart2_rtos.huart);
#ifdef SERIAL_DEBUG
	HAL_UART_DeInit(uart3_rtos.huart);
#endif
	HAL_ADC_DeInit(adc_rtos.hadc);
	return;
}

void peripherial_init(void)
{
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_SPI3_Init();
	MX_ADC1_Init();
	MX_USART2_UART_Init();
	MX_SPI2_Init();
	MX_USART3_UART_Init();
	MX_CRC_Init();
	return;
}
