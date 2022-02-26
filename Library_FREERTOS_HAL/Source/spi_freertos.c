/* FreeRTOS */
#include "FreeRTOS.h"
#include "spi_freertos.h"
#include "semphr.h"


/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

/* SPI FreeRTOS list item structure */
struct spi_rtos_list
{
	spi_freertos_t			*spi_rtos;
	struct spi_rtos_list	*next;
};

/* Root of SPI FreeRTOS list */
static struct spi_rtos_list *spi_rtos_list_begin = NULL;

/* Append the item at the end of SPI FreeRTOS list */
static struct spi_rtos_list* spi_rtos_list_append(spi_freertos_t *spi_rtos)
{
	/* if list is empty, create and return the begin of list */
	if(spi_rtos_list_begin == NULL)
	{
		spi_rtos_list_begin = (struct spi_rtos_list*)
			pvPortMalloc(sizeof(struct spi_rtos_list));
		spi_rtos_list_begin->spi_rtos = spi_rtos;
		spi_rtos_list_begin->next = NULL;
		return spi_rtos_list_begin;
	}
	/* else find end of list */
	struct spi_rtos_list *item = spi_rtos_list_begin;
	while(item->next != NULL)
		item = item->next;
	/* and create and append item at the end of list */
	item->next = (struct spi_rtos_list*)
		pvPortMalloc(sizeof(struct spi_rtos_list));
	item->next->spi_rtos = spi_rtos;
	item->next->next = NULL;
	return item->next;
}

/* Delete the item from SPI FreeRTOS list */
static struct spi_rtos_list* spi_rtos_list_delete
	(spi_freertos_t *spi_rtos)
{
	struct spi_rtos_list *item = spi_rtos_list_begin, *tmp;
	/* find list item */
	while(item != NULL)
	{
		/* if item found, delete and replace next item pointer */
		if(item->next->spi_rtos == spi_rtos)
		{
			tmp = item->next;
			item->next = tmp->next;
			vPortFree(tmp);
			break;
		}
		else
			item = item->next;
	}
	return item;
}

/* Find item in SPI FreeRTOS list with mached SPI_HandleTypeDef */
static struct spi_rtos_list* spi_rtos_list_find_item
	(SPI_HandleTypeDef* hspi)
{
	struct spi_rtos_list *item = spi_rtos_list_begin;
	while(item != NULL)
	{
		if(item->spi_rtos->hspi == hspi)
			break;
		else
			item = item->next;
	}
	return item;
}

/* Initialize SPI with FreeRTOS mutexes and semaphores */
spi_freertos_status spi_freertos_init(spi_freertos_t* spi_rtos)
{	
	/* find hspi into list */
	if(spi_rtos_list_find_item(spi_rtos->hspi) != NULL)
		return SPI_FREERTOS_EXIST;
	
	/* if hspi not found, create semaphores and mutexes */
	spi_rtos->mutex = xSemaphoreCreateMutex();
	spi_rtos->tx_complete = xSemaphoreCreateBinary();
	spi_rtos->rx_complete = xSemaphoreCreateBinary();
	
	/* register spi_freertos_base into list */
	spi_rtos_list_append(spi_rtos);
	return SPI_FREERTOS_OK;
}

/* Deinitialize SPI with FreeRTOS mutexes and semaphores */
void spi_freertos_deinit(spi_freertos_t* spi_rtos)
{
	/* remove semaphores, mutexes */
	vSemaphoreDelete(spi_rtos->mutex);
	vSemaphoreDelete(spi_rtos->tx_complete);
	vSemaphoreDelete(spi_rtos->rx_complete);
	/* unregister spi_freertos_base from list */
	spi_rtos_list_delete(spi_rtos);
}

/* Set callback for check SPI configuration (CPHA, CPOL, rate, etc.)
 * before transaction */
void spi_freertos_set_check_config_callback(spi_freertos_nss_t* spi,
	void (*check_spi_conf_callback)(SPI_HandleTypeDef *hspi))
{
	spi->check_spi_conf_callback = check_spi_conf_callback;
}

/* Clear callback for check SPI configuration (CPHA, CPOL, rate, etc.)
 * before transaction */
void spi_freertos_clear_check_config_callback(spi_freertos_nss_t* spi)
{
	spi->check_spi_conf_callback = NULL;
}

/* Write registers through SPI */
spi_freertos_status spi_freertos_write(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_timeout, uint32_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* NSS to low - start of transaction */
	spi_freertos_nss_low(spi);
	
	/* Command data write from buffer */
	if(cmd_size == 0) goto data_stage;
	hal_ret = HAL_SPI_Transmit(spi->spi_rtos->hspi,
		(void *) cmd, cmd_size, transfer_timeout);
	switch(hal_ret)
	{
		case HAL_ERROR:
			ret = SPI_FREERTOS_ERR;
			goto end_of_transaction;
		case HAL_BUSY:
			ret = SPI_FREERTOS_BUSY;
			goto end_of_transaction;
		case HAL_TIMEOUT:
			ret = SPI_FREERTOS_TIMEOUT;
			goto end_of_transaction;
		default:
			break;
	}
	
	data_stage:
	if(data_size == 0) goto end_of_transaction;
	
	/* Data write from buffer */
	hal_ret = HAL_SPI_Transmit(spi->spi_rtos->hspi,
		(void *) data, data_size, transfer_timeout);
	switch(hal_ret)
	{
		case HAL_ERROR:
			ret = SPI_FREERTOS_ERR;
			goto end_of_transaction;
		case HAL_BUSY:
			ret = SPI_FREERTOS_BUSY;
			goto end_of_transaction;
		case HAL_TIMEOUT:
			ret = SPI_FREERTOS_TIMEOUT;
			goto end_of_transaction;
		default:
			break;
	}

	end_of_transaction:
	/* NSS to high - end of transaction */
	spi_freertos_nss_high(spi);
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Read registers through SPI */
spi_freertos_status spi_freertos_read(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_timeout, uint32_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* NSS to low - start of transaction */
	spi_freertos_nss_low(spi);
	
	/* Command data write from buffer */
	if(cmd_size == 0) goto data_stage;
	hal_ret = HAL_SPI_Transmit(spi->spi_rtos->hspi,
		(void *) cmd, cmd_size, transfer_timeout);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	case HAL_TIMEOUT:
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	default:
		break;
	}
	
	data_stage:
	if(data_size == 0) goto end_of_transaction;
	
	/* Data read to buffer */
	hal_ret = HAL_SPI_Receive(spi->spi_rtos->hspi,
		(void *) data, data_size, transfer_timeout);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	case HAL_TIMEOUT:
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	default:
		break;
	}

	end_of_transaction:
	/* NSS to high - end of transaction */
	spi_freertos_nss_high(spi);
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Write registers through SPI using DMA */
spi_freertos_status spi_freertos_write_dma(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_timeout, TickType_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* NSS to low - start of transaction */
	spi_freertos_nss_low(spi);
	
	/* Command data DMA write from buffer */
	if(cmd_size == 0) goto data_stage;
	hal_ret = HAL_SPI_Transmit_DMA(spi->spi_rtos->hspi,
		(void *) cmd, cmd_size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->tx_complete, transfer_timeout)
		== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	data_stage:
	if(data_size == 0) goto end_of_transaction;
	
	/* Data DMA write from buffer */
	hal_ret = HAL_SPI_Transmit_DMA(spi->spi_rtos->hspi,
		(void *) data, data_size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->tx_complete, transfer_timeout)	== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	end_of_transaction:
	/* NSS to high - end of transaction */
	spi_freertos_nss_high(spi);
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Read registers through SPI using DMA */
spi_freertos_status spi_freertos_read_dma(spi_freertos_nss_t* spi,
	const void* cmd,  size_t cmd_size,
	const void* data, size_t data_size,
	TickType_t mutex_timeout, TickType_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* NSS to low - start of transaction */
	spi_freertos_nss_low(spi);
	
	/* Command data DMA write from buffer */
	if(cmd_size == 0) goto data_stage;
	hal_ret = HAL_SPI_Transmit_DMA(spi->spi_rtos->hspi,
		(void *) cmd, cmd_size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->tx_complete, transfer_timeout)
		== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	data_stage:
	if(data_size == 0) goto end_of_transaction;
	
	/* Data DMA read to buffer */
	hal_ret = HAL_SPI_Receive_DMA(spi->spi_rtos->hspi,
		(void *) data, data_size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->rx_complete, transfer_timeout)
		== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	end_of_transaction:
	/* NSS to high - end of transaction */
	spi_freertos_nss_high(spi);
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Write through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_write(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data write from buffer */
	hal_ret = HAL_SPI_Transmit(spi->spi_rtos->hspi,
		(void *) buf, size, transfer_timeout);
	switch(hal_ret)
	{
		case HAL_ERROR:
			ret = SPI_FREERTOS_ERR;
			goto end_of_transaction;
		case HAL_BUSY:
			ret = SPI_FREERTOS_BUSY;
			goto end_of_transaction;
		case HAL_TIMEOUT:
			ret = SPI_FREERTOS_TIMEOUT;
			goto end_of_transaction;
		default:
			break;
	}

	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Read through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_read(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data read to buffer */
	hal_ret = HAL_SPI_Receive(spi->spi_rtos->hspi,
		(void *) buf, size, transfer_timeout);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	case HAL_TIMEOUT:
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	default:
		break;
	}

	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Full-duplex transaction through SPI (slave mode) */
spi_freertos_status spi_freertos_slave_rw(spi_freertos_nss_t* spi,
	const void* txbuf, const void* rxbuf, size_t size,
	TickType_t mutex_timeout, uint32_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data write from buffer */
	hal_ret = HAL_SPI_TransmitReceive(spi->spi_rtos->hspi,
		(void *) txbuf, (void *) rxbuf, size, transfer_timeout);
	switch(hal_ret)
	{
		case HAL_ERROR:
			ret = SPI_FREERTOS_ERR;
			goto end_of_transaction;
		case HAL_BUSY:
			ret = SPI_FREERTOS_BUSY;
			goto end_of_transaction;
		case HAL_TIMEOUT:
			ret = SPI_FREERTOS_TIMEOUT;
			goto end_of_transaction;
		default:
			break;
	}

	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Write through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_write_dma(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data DMA write from buffer */
	hal_ret = HAL_SPI_Transmit_DMA(spi->spi_rtos->hspi, (void *) buf, size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->tx_complete, transfer_timeout)	== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Read through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_read_dma(spi_freertos_nss_t* spi,
	const void* buf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data DMA read to buffer */
	hal_ret = HAL_SPI_Receive_DMA(spi->spi_rtos->hspi, (void *) buf, size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->rx_complete, transfer_timeout)
		== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Full-duplex transaction through SPI using DMA (slave mode) */
spi_freertos_status spi_freertos_slave_rw_dma(spi_freertos_nss_t* spi,
	const void* txbuf, const void* rxbuf, size_t size,
	TickType_t mutex_timeout, TickType_t transfer_timeout)
{
	spi_freertos_status ret = SPI_FREERTOS_OK;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	
	/* Check and change SPI configuration if nessessary */
	if(spi->check_spi_conf_callback != NULL)
		spi->check_spi_conf_callback(spi->spi_rtos->hspi);
	
	/* TODO:: Detect the start of transaction (NSS to low) - TBD */
	/* spi_freertos_nss_low(spi); */
	
	if(size == 0) goto end_of_transaction;
	
	/* Data DMA write from buffer */
	hal_ret = HAL_SPI_TransmitReceive_DMA(spi->spi_rtos->hspi,
		(void *) txbuf, (void *) rxbuf, size);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		goto end_of_transaction;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		goto end_of_transaction;
	default:
		break;
	}
	
	/* Waiting for transfer complete */
	if(xSemaphoreTake(spi->spi_rtos->rx_complete, transfer_timeout)	== pdFALSE)
	{
		ret = SPI_FREERTOS_TIMEOUT;
		goto end_of_transaction;
	}
	
	end_of_transaction:
	/* TODO:: Check the correct end of transaction (NSS to high) - TBD */
	/* spi_freertos_nss_high(spi); */
	
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	
	exit:
	return ret;
}

/* Abort SPI transactions */
spi_freertos_status spi_freertos_abort(spi_freertos_nss_t* spi,
	TickType_t mutex_timeout)
{
	spi_freertos_status ret;
	HAL_StatusTypeDef hal_ret;
	/* Take SPI mutex */
	if(xSemaphoreTake(spi->spi_rtos->mutex, mutex_timeout) == pdFALSE)
	{
		ret = SPI_FREERTOS_BUSY;
		goto exit;
	}
	hal_ret = HAL_SPI_Abort(spi->spi_rtos->hspi);
	switch(hal_ret)
	{
	case HAL_ERROR:
		ret = SPI_FREERTOS_ERR;
		break;
	case HAL_BUSY:
		ret = SPI_FREERTOS_BUSY;
		break;
	default:
		break;
	}
	spi_freertos_nss_high(spi);
	/* Give back SPI mutex */
	xSemaphoreGive(spi->spi_rtos->mutex);
	exit:
	return ret;
}

/* Set NSS pin to high */
inline void spi_freertos_nss_high(spi_freertos_nss_t* spi)
{
	if(spi->nss.port != NULL)
		HAL_GPIO_WritePin(spi->nss.port, spi->nss.pin, GPIO_PIN_SET);
}

/* Set NSS pin to low */
inline void spi_freertos_nss_low(spi_freertos_nss_t* spi)
{
	if(spi->nss.port != NULL)
		HAL_GPIO_WritePin(spi->nss.port, spi->nss.pin, GPIO_PIN_RESET);
}

/* SPI interrupt service routines */

/*
void HAL_SPI_IRQHandler(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi);
*/

/* RX complete */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct spi_rtos_list *item = spi_rtos_list_find_item(hspi);
	if(item == NULL) return;
	xSemaphoreGiveFromISR(item->spi_rtos->rx_complete,
		&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* TX complete */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct spi_rtos_list *item = spi_rtos_list_find_item(hspi);
	if(item == NULL) return;
	xSemaphoreGiveFromISR(item->spi_rtos->tx_complete,
		&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Full-duplex complete - rx_complete semaphore is used*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	struct spi_rtos_list *item = spi_rtos_list_find_item(hspi);
	if(item == NULL) return;
	xSemaphoreGiveFromISR(item->spi_rtos->rx_complete,
		&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
