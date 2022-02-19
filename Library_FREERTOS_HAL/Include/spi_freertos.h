#ifndef SPI_FREERTOS_H
#define SPI_FREERTOS_H
#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------
  Includes
----------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

#include "main.h"
#include "gpio_freertos.h"
/* HAL */
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "semphr.h"

/*----------------------------------------------------------------------
  Defines
----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
  Data type declarations
----------------------------------------------------------------------*/

/* Status codes */
typedef enum
{
	SPI_FREERTOS_OK			= 0x00U,	// Success
	SPI_FREERTOS_ERR		= 0x01U,	// Error
	SPI_FREERTOS_NODEV		= 0x02U,	// No such device
	SPI_FREERTOS_BUSY		= 0x03U,	// Device is busy
	SPI_FREERTOS_TIMEOUT	= 0x04U,	// Timeout
	SPI_FREERTOS_EXIST		= 0x05U		// SPI already exists
} spi_freertos_status;

/* SPI FreeRTOS structures */
typedef struct __packed
{
	/* SPI interface */
	SPI_HandleTypeDef		*hspi;
	/* SPI mutual exception */
	SemaphoreHandle_t		mutex;
	/* SPI transfer complete semaphores */
	SemaphoreHandle_t		rx_complete;
	SemaphoreHandle_t		tx_complete;
} spi_freertos_t;

typedef struct __packed
{
	/* SPI interface with RTOS extentions */
	spi_freertos_t		*spi_rtos;
	/* SPI NSS */
	gpio_freertos_t		nss;
	/* Check settings callback */
	void		(*check_spi_conf_callback)(SPI_HandleTypeDef *hspi);
} spi_freertos_nss_t;

/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

/* Initialize SPI with FreeRTOS mutexes and semaphores */
spi_freertos_status spi_freertos_init(spi_freertos_t* spi);

/* Deinitialize SPI with FreeRTOS mutexes and semaphores */
void spi_freertos_deinit(spi_freertos_t* spi);

/* Set callback for check SPI configuration (CPHA, CPOL, rate, etc.)
 * before transaction */
void spi_freertos_set_check_config_callback(spi_freertos_nss_t* spi,
	void (*check_spi_conf_callback)(SPI_HandleTypeDef *hspi));

/* Clear callback for check SPI configuration (CPHA, CPOL, rate, etc.)
 * before transaction */
void spi_freertos_clear_check_config_callback(spi_freertos_nss_t* spi);

/* Write registers through SPI */
spi_freertos_status spi_freertos_write(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_wait_timeout, uint32_t transfer_wait_timeout);

/* Read registers through SPI */
spi_freertos_status spi_freertos_read(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_wait_timeout, uint32_t transfer_wait_timeout);

/* Write registers through SPI using DMA */
spi_freertos_status spi_freertos_write_dma(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_timeout, TickType_t transfer_timeout);

/* Read registers through SPI using DMA */
spi_freertos_status spi_freertos_read_dma(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_wait_timeout, TickType_t transfer_wait_timeout);

/* Write through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_write(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout);

/* Read through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_read(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout);

/* Full-duplex transaction through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_rw(spi_freertos_nss_t* spi,
	const void* txbuf, const void* rxbuf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout);

/* Write through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_write_dma(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout);

/* Read through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_read_dma(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout);

/* Full-duplex transaction through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_rw_dma(spi_freertos_nss_t* spi,
	const void* txbuf, const void* rxbuf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout);

/* Abort SPI transactions */
spi_freertos_status spi_freertos_abort(spi_freertos_nss_t* spi,
	TickType_t mutex_timeout);

/* Set NSS pin to high */
void spi_freertos_nss_high(spi_freertos_nss_t* spi);

/* Set NSS pin to low */
void spi_freertos_nss_low(spi_freertos_nss_t* spi);

/* SPI ISR callback implemetations */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

#ifdef __cplusplus
}
#endif
#endif /* SPI_FREERTOS_H */
