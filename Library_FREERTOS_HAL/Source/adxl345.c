#include "main.h"
/* HAL from STMCube */
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "stm32f1xx_hal_gpio.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "adxl345.h"
#include "semphr.h"


/*----------------------------------------------------------------------
  Utils
----------------------------------------------------------------------*/
/* convert data to float in g */
float adxl345_convert_float_g(int16_t u)
{
	return (float) u * ADXL345_MG2G_MULTIPLIER;
}
/* convert data to float in m/s^2 */
float adxl345_convert_float_mpss(int16_t u)
{
	return (float) u * ADXL345_MG2G_MULTIPLIER * 9.81f;
}

/*----------------------------------------------------------------------
  High-level functions
----------------------------------------------------------------------*/

/* Init the device*/
adxl345_status adxl345_init(adxl345_handle_t* hadxl)
{
	/* register callback for check SPI configuration */
	spi_freertos_set_check_config_callback(&(hadxl->spi),
		adxl345_spi_check_config);
	
	uint8_t reg;
	adxl345_status ret = ADXL345_OK;
	/* set up NSS into inactive state */
	spi_freertos_nss_high(&(hadxl->spi));
	
	/* Read Device ID */
	ret = adxl345_read_spi(hadxl, ADXL345_REG_DEVID,
		(void *) &reg, sizeof(reg));
	if(ret != ADXL345_OK) return ret;
	if(reg != ADXL345_ID) return ADXL345_NODEV;
	
	/* Disable device operation */
	reg = 0;
	ret = adxl345_write_spi(hadxl, ADXL345_REG_POWER_CTL,
		(void *) &reg, sizeof(reg));
	if(ret != ADXL345_OK) return ret;
	
	/* Clear interrupts */
	reg = 0;
	ret = adxl345_write_spi(hadxl, ADXL345_REG_INT_ENABLE,
		(void *) &reg, sizeof(reg));
	if(ret != ADXL345_OK) return ret;

	/* Write device settings */
	/* Data rate */
	ret = adxl345_write_spi(hadxl, ADXL345_REG_BW_RATE,
		(void *) &(hadxl->settings.data_rate),
		sizeof(hadxl->settings.data_rate));
	if(ret != ADXL345_OK) return ret;
	/* Data format */
	reg = 	hadxl->settings.meas_range | hadxl->settings.align |
			hadxl->settings.resolution;
	ret = adxl345_write_spi(hadxl, ADXL345_REG_DATA_FORMAT,
		(void *) &reg, sizeof(reg));
	if(ret != ADXL345_OK) return ret;
	
	/* Acc offset */
	ret = adxl345_write_spi(hadxl, ADXL345_REG_OFSX,
		(void *) &(hadxl->settings.offset),
		sizeof(hadxl->settings.offset));
	if(ret != ADXL345_OK) return ret;
	
	/* FIFO */
	reg = 	hadxl->settings.fifo_mode | hadxl->settings.fifo_trigger |
		(hadxl->settings.fifo_watermark&ADXL345_FIFO_CTL_SAMPLES_MASK);
	ret = adxl345_write_spi(hadxl, ADXL345_REG_FIFO_CTL,
		(void *) &reg, sizeof(reg));
	if(ret != ADXL345_OK) return ret;
	
	/* Interrupts */
	ret = adxl345_write_spi(hadxl, ADXL345_REG_INT_MAP,
		(void *) &(hadxl->settings.map_to_int2),
		sizeof(hadxl->settings.map_to_int2));
	if(ret != ADXL345_OK) return ret;
	ret = adxl345_write_spi(hadxl, ADXL345_REG_INT_ENABLE,
		(void *) &(hadxl->settings.int_en),
		sizeof(hadxl->settings.int_en));
	if(ret != ADXL345_OK) return ret;
	
	return ret;
}

/* Get the current offsets for all axes (15.6mg/LSB) */
adxl345_status adxl345_get_offset(adxl345_handle_t* hadxl,
	adxl345_acc_offset_t* offset)
{
	return adxl345_read_spi_dma(hadxl, ADXL345_REG_OFSX,
		offset, sizeof(adxl345_acc_offset_t));
}

/* Set the current offsets for all axes (15.6mg/LSB) */
adxl345_status adxl345_set_offset(adxl345_handle_t* hadxl,
	adxl345_acc_offset_t* offset)
{
	return adxl345_write_spi_dma(hadxl, ADXL345_REG_OFSX,
		offset, sizeof(adxl345_acc_offset_t));
}

/* Start acceleration measurements */
adxl345_status adxl345_start(adxl345_handle_t* hadxl)
{
	uint8_t data;
	adxl345_status ret = ADXL345_OK;
	ret = adxl345_read_spi_dma(hadxl, ADXL345_REG_POWER_CTL,
							&data, sizeof(data));
	if(ret != ADXL345_OK) return ret;
	data |= ADXL345_MEASURE;
	ret = adxl345_write_spi_dma(hadxl, ADXL345_REG_POWER_CTL,
							&data, sizeof(data));
	return ret;
}

/* Stop acceleration measurements */
adxl345_status adxl345_stop(adxl345_handle_t* hadxl)
{
	uint8_t data;
	adxl345_status ret = ADXL345_OK;
	ret = adxl345_read_spi_dma(hadxl, ADXL345_REG_POWER_CTL,
							&data, sizeof(data));
	if(ret != ADXL345_OK) return ret;
	data &= (~ADXL345_MEASURE);
	ret = adxl345_write_spi_dma(hadxl, ADXL345_REG_POWER_CTL,
							&data, sizeof(data));
	return ret;
}

/* Get the acceleration data for all axes */
adxl345_status adxl345_get_data(adxl345_handle_t* hadxl,
	adxl345_acc_data_t* data)
{
	return adxl345_read_spi_dma(hadxl, ADXL345_REG_DATAX0,
		data, sizeof(adxl345_acc_data_t));
}

/* Flush internal FIFO */
adxl345_status adxl345_fifo_flush(adxl345_handle_t* hadxl)
{
	adxl345_acc_data_t tmp;
	uint8_t sample_cnt = 0;
	while(adxl345_get_int_src(hadxl) & ADXL345_INT_DATA_READY)
	{
		if(sample_cnt++ > ADXL345_FLUSH_THESHOLD)
			return ADXL345_ERR;
		adxl345_get_data(hadxl, &tmp);
	}
	return ADXL345_OK;
}

/*----------------------------------------------------------------------
  Middle-level functions
----------------------------------------------------------------------*/

/* Write the device register */
void adxl345_write_reg(adxl345_handle_t* hadxl, uint8_t reg,
	uint8_t value)
{
	adxl345_write_spi(hadxl, reg, (void *) &value, sizeof(value));
}

/* Read the device register */
uint8_t adxl345_read_reg(adxl345_handle_t* hadxl, uint8_t reg)
{
	uint8_t value;
	adxl345_read_spi(hadxl, reg, (void *) &value, sizeof(value));
	return value;
}

/* Read the device ID register */
uint8_t adxl345_get_id(adxl345_handle_t* hadxl)
{
	uint8_t id;
	adxl345_read_spi(hadxl, ADXL345_REG_DEVID,
		(void *) &id, sizeof(id));
	return id;
}

/* Read the INT_SOURCE register */
uint8_t adxl345_get_int_src(adxl345_handle_t* hadxl)
{
	uint8_t reg;
	adxl345_read_spi(hadxl, ADXL345_REG_INT_SOURCE,
		(void *) &reg, sizeof(reg));
	return reg;
}

/* Get FIFO count */
uint8_t adxl345_get_fifo_cnt(adxl345_handle_t* hadxl)
{
	uint8_t reg;
	adxl345_read_spi(hadxl, ADXL345_REG_FIFO_STATUS,
		(void *) &reg, sizeof(reg));
	return (reg & ADXL345_FIFO_STATUS_CNT_MASK);
}

/*----------------------------------------------------------------------
  Low-level functions
----------------------------------------------------------------------*/

/* Write registers through SPI */
adxl345_status adxl345_write_spi(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size)
{
	/* Command - write multiple bytes */
	uint8_t cmd = ADXL345_SPI_MB | (addr & ADXL345_SPI_ADDR_MASK);
	
	return (adxl345_status) spi_freertos_write(&(hadxl->spi), &cmd, sizeof(cmd),
		buf, size, hadxl->mutex_timeout, (uint32_t) hadxl->transfer_timeout);
}

/* Read registers through SPI */
adxl345_status adxl345_read_spi(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size)
{
	/* Command - read multiple bytes */
	uint8_t cmd = ADXL345_SPI_READ |
				ADXL345_SPI_MB | (addr&ADXL345_SPI_ADDR_MASK);
	
	return (adxl345_status) spi_freertos_read(&(hadxl->spi), &cmd, sizeof(cmd),
		buf, size, hadxl->mutex_timeout, (uint32_t) hadxl->transfer_timeout);
}

/* Write registers through SPI using DMA */
adxl345_status adxl345_write_spi_dma(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size)
{
	/* Command - write multiple bytes */
	uint8_t cmd = ADXL345_SPI_MB | (addr & ADXL345_SPI_ADDR_MASK);
	
	return (adxl345_status) spi_freertos_write_dma(&(hadxl->spi),
		&cmd, sizeof(cmd), buf, size,
		hadxl->mutex_timeout, hadxl->transfer_timeout);
}

/* Read registers through SPI using DMA */
adxl345_status adxl345_read_spi_dma(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size)
{
	uint8_t cmd = ADXL345_SPI_READ |
				ADXL345_SPI_MB | (addr&ADXL345_SPI_ADDR_MASK);
	
	return (adxl345_status) spi_freertos_read_dma(&(hadxl->spi),
		&cmd, sizeof(cmd), buf, size,
		hadxl->mutex_timeout, hadxl->transfer_timeout);
}

/* Check and change SPI configuration */
void adxl345_spi_check_config(SPI_HandleTypeDef* hspi)
{
	/* Check configuration */
	if( (hspi->Init.CLKPhase    != ADXL345_SPI_CPHA)       ||
		(hspi->Init.CLKPolarity != ADXL345_SPI_CPOL)       ||
		(hspi->Init.DataSize    != ADXL345_SPI_DATASIZE)   ||
		(hspi->Init.Direction   != ADXL345_SPI_DIRECTION)  ||
		(hspi->Init.FirstBit    != ADXL345_SPI_BIT_ORDER))
	{
		hspi->Init.CLKPhase     = ADXL345_SPI_CPHA;
		hspi->Init.CLKPolarity  = ADXL345_SPI_CPOL;
		hspi->Init.DataSize     = ADXL345_SPI_DATASIZE;
		hspi->Init.Direction    = ADXL345_SPI_DIRECTION;
		hspi->Init.FirstBit     = ADXL345_SPI_BIT_ORDER;
		if (HAL_SPI_Init(hspi) != HAL_OK)
		{
			Error_Handler();
		}
	}
}
