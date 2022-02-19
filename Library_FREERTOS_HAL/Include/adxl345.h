#ifndef ADXL345_H
#define ADXL345_H
#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------
  Includes
----------------------------------------------------------------------*/
#include "main.h"
/* HAL */
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "stm32f1xx_hal_gpio.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "semphr.h"
/* Common */
#include "gpio_freertos.h"
#include "spi_freertos.h"

/*----------------------------------------------------------------------
  Defines
----------------------------------------------------------------------*/

/* SPI */
#define ADXL345_SPI_CPHA				SPI_PHASE_2EDGE	     // CPHA=1
#define ADXL345_SPI_CPOL				SPI_POLARITY_HIGH    // CPOL=1
#define ADXL345_SPI_DATASIZE			SPI_DATASIZE_8BIT
#define ADXL345_SPI_DIRECTION			SPI_DIRECTION_2LINES // 4-wire
#define ADXL345_SPI_BIT_ORDER			SPI_FIRSTBIT_MSB

#define ADXL345_SPI_READ				(1<<7)
#define ADXL345_SPI_MB					(1<<6)
#define ADXL345_SPI_ADDR_MASK			(0x3F)

/* register map */
#define ADXL345_REG_DEVID               (0x00) // Device ID
#define ADXL345_REG_THRESH_TAP          (0x1D) // Tap threshold
#define ADXL345_REG_OFSX                (0x1E) // X-axis offset
#define ADXL345_REG_OFSY                (0x1F) // Y-axis offset
#define ADXL345_REG_OFSZ                (0x20) // Z-axis offset
#define ADXL345_REG_DUR                 (0x21) // Tap duration
#define ADXL345_REG_LATENT              (0x22) // Tap latency
#define ADXL345_REG_WINDOW              (0x23) // Tap window
#define ADXL345_REG_THRESH_ACT          (0x24) // Activity threshold
#define ADXL345_REG_THRESH_INACT        (0x25) // Inactivity threshold
#define ADXL345_REG_TIME_INACT          (0x26) // Inactivity time
#define ADXL345_REG_ACT_INACT_CTL       (0x27) // Axis enable control for activity and inactivity detection
#define ADXL345_REG_THRESH_FF           (0x28) // Free-fall threshold
#define ADXL345_REG_TIME_FF             (0x29) // Free-fall time
#define ADXL345_REG_TAP_AXES            (0x2A) // Axis control for single/double tap
#define ADXL345_REG_ACT_TAP_STATUS      (0x2B) // Source for single/double tap
#define ADXL345_REG_BW_RATE             (0x2C) // Data rate and power mode control
#define ADXL345_REG_POWER_CTL           (0x2D) // Power-saving features control
#define ADXL345_REG_INT_ENABLE          (0x2E) // Interrupt enable control
#define ADXL345_REG_INT_MAP             (0x2F) // Interrupt mapping control
#define ADXL345_REG_INT_SOURCE          (0x30) // Source of interrupts
#define ADXL345_REG_DATA_FORMAT         (0x31) // Data format control
#define ADXL345_REG_DATAX0              (0x32) // X-axis data 0
#define ADXL345_REG_DATAX1              (0x33) // X-axis data 1
#define ADXL345_REG_DATAY0              (0x34) // Y-axis data 0
#define ADXL345_REG_DATAY1              (0x35) // Y-axis data 1
#define ADXL345_REG_DATAZ0              (0x36) // Z-axis data 0
#define ADXL345_REG_DATAZ1              (0x37) // Z-axis data 1
#define ADXL345_REG_FIFO_CTL            (0x38) // FIFO control
#define ADXL345_REG_FIFO_STATUS         (0x39) // FIFO status

/* ADXL345_REG_DEVID content */
#define ADXL345_ID						(0xE5)

/* bit fields and masks */
#define ADXL345_MEASURE					(1<<3)
#define ADXL345_SLEEP					(1<<2)
#define ADXL345_INT_INVERT				(1<<5)
#define ADXL345_FIFO_CTL_SAMPLES_MASK	(0x1F)
#define ADXL345_FIFO_STATUS_CNT_MASK	(0x3F)

/* Interrupts (INT_ENABLE, map_to_int2 and INT_SOURCE registers) */
#define ADXL345_INT_DATA_READY			(1<<7)
#define ADXL345_INT_SINGLE_TAP			(1<<6)
#define ADXL345_INT_DOUBLE_TAP			(1<<5)
#define ADXL345_INT_ACTIVITY			(1<<4)
#define ADXL345_INT_INACTIVITY			(1<<3)
#define ADXL345_INT_FREE_FALL			(1<<2)
#define ADXL345_INT_WATERMARK			(1<<1)
#define ADXL345_INT_OVERRUN				(1<<0)

#define ADXL345_MG2G_MULTIPLIER			(0.004)  // 4mg per LSB

/* Flush samples error theshold */
#define ADXL345_FLUSH_THESHOLD			64U

/*----------------------------------------------------------------------
  Data type declarations
----------------------------------------------------------------------*/

/* Used with register 0x2C (ADXL345_REG_BW_RATE) to set bandwidth */
typedef enum
{
    ADXL345_DATARATE_3200_HZ = 0b1111, // 1600Hz Bandwidth   140µA IDD
    ADXL345_DATARATE_1600_HZ = 0b1110, //  800Hz Bandwidth    90µA IDD
    ADXL345_DATARATE_800_HZ  = 0b1101, //  400Hz Bandwidth   140µA IDD
    ADXL345_DATARATE_400_HZ  = 0b1100, //  200Hz Bandwidth   140µA IDD
    ADXL345_DATARATE_200_HZ  = 0b1011, //  100Hz Bandwidth   140µA IDD
    ADXL345_DATARATE_100_HZ  = 0b1010, //   50Hz Bandwidth   140µA IDD
    ADXL345_DATARATE_50_HZ   = 0b1001, //   25Hz Bandwidth    90µA IDD
    ADXL345_DATARATE_25_HZ   = 0b1000, // 12.5Hz Bandwidth    60µA IDD
    ADXL345_DATARATE_12_5_HZ = 0b0111, // 6.25Hz Bandwidth    50µA IDD
    ADXL345_DATARATE_6_25_HZ = 0b0110, // 3.13Hz Bandwidth    45µA IDD
    ADXL345_DATARATE_3_13_HZ = 0b0101, // 1.56Hz Bandwidth    40µA IDD
    ADXL345_DATARATE_1_56_HZ = 0b0100, // 0.78Hz Bandwidth    34µA IDD
    ADXL345_DATARATE_0_78_HZ = 0b0011, // 0.39Hz Bandwidth    23µA IDD
    ADXL345_DATARATE_0_39_HZ = 0b0010, // 0.20Hz Bandwidth    23µA IDD
    ADXL345_DATARATE_0_20_HZ = 0b0001, // 0.10Hz Bandwidth    23µA IDD
    ADXL345_DATARATE_0_10_HZ = 0b0000  // 0.05Hz Bandwidth    23µA IDD (default value)
} adxl345_data_rate_t;

/* Used with register 0x31 (ADXL345_REG_DATA_FORMAT) to set g range */
typedef enum
{
    ADXL345_RANGE_16_G = 0b11,   // ±16g
    ADXL345_RANGE_8_G  = 0b10,   // ±8g
    ADXL345_RANGE_4_G  = 0b01,   // ±4g
    ADXL345_RANGE_2_G  = 0b00    // ±2g (default value)
} adxl345_range_t;

/* Used with register 0x31 (ADXL345_REG_DATA_FORMAT) to set data align */
typedef enum
{
    ADXL345_ALIGN_LSB = 0,     // right-justified (LSB) mode with sign extention
    ADXL345_ALIGN_MSB = (1<<2) // left-justified (MSB) mode
} adxl345_align_t;

/* Used with register 0x31 (ADXL345_REG_DATA_FORMAT) to set data align */
typedef enum
{
    ADXL345_RESOLUTION_REDUCED = 0,     // 10-bit mode
    ADXL345_RESOLUTION_FULL    = (1<<3) // 13-bit mode (4mg/LSB)
} adxl345_resolution_t;

/* acc offset structure */
typedef struct __packed
{
	int8_t		x;
	int8_t		y;
	int8_t		z;
} adxl345_acc_offset_t;

/* Used with register 0x38 (ADXL345_FIFO_CTL) to set FIFO mode */
typedef enum
{
    ADXL345_FIFO_BYPASS		= (0b00U<<6), // FIFO is bypassed
    ADXL345_FIFO_FIFO		= (0b01U<<6), // FIFO mode
    ADXL345_FIFO_STREAM		= (0b10U<<6), // Stream mode
    ADXL345_FIFO_TRIGGER	= (0b11U<<6)  // Trigger mode
} adxl345_fifo_mode_t;

/* Used with register 0x38 (ADXL345_FIFO_CTL) to set FIFO trigger bit */
typedef enum
{
    ADXL345_FIFO_TRIG_INT1	= 0, 		// trigger event in INT1
    ADXL345_FIFO_TRIG_INT2	= (1<<5)	// trigger event in INT2
} adxl345_fifo_trigger_t;

/* settings structure */
typedef struct __packed
{
	/* Data rate */
	adxl345_data_rate_t		data_rate;
	/* Data format */
	adxl345_range_t			meas_range;
	adxl345_align_t			align;
	adxl345_resolution_t	resolution;
	/* Acc offset */
	adxl345_acc_offset_t	offset;
	/* Interrupts*/
	uint8_t					int_en;
	uint8_t					map_to_int2;
	/* FIFO*/
	adxl345_fifo_mode_t		fifo_mode;
	adxl345_fifo_trigger_t	fifo_trigger;
	uint8_t					fifo_watermark;
} adxl345_settings_t;

/* handle structure */
typedef struct __packed
{
	/* SPI */
	spi_freertos_nss_t	spi;
	/* EXTI GPIO */
	gpio_freertos_t		int1;
	gpio_freertos_t		int2;
	/* settings */
	adxl345_settings_t	settings;
	/* Timeouts */
	TickType_t			mutex_timeout;
	TickType_t			transfer_timeout;
} adxl345_handle_t;

/* acc data structure */
typedef struct __packed
{
	int16_t		x;
	int16_t		y;
	int16_t		z;
} adxl345_acc_data_t;

/* Status codes */
typedef enum
{
	ADXL345_OK					= 0x00U,	// Success
	ADXL345_ERR					= 0x01U,	// Error
	ADXL345_NODEV				= 0x02U,	// No such device
	ADXL345_BUSY				= 0x03U,	// Device is busy
	ADXL345_TIMEOUT				= 0x04U		// Timeout
} adxl345_status;

/*----------------------------------------------------------------------
  Utils
----------------------------------------------------------------------*/
/* convert data to float in g */
float adxl345_convert_float_g(int16_t u);
/* convert data to float in m/s^2 */
float adxl345_convert_float_mpss(int16_t u);

/*----------------------------------------------------------------------
  High-level functions
----------------------------------------------------------------------*/

/* Init the device*/
adxl345_status adxl345_init(adxl345_handle_t* hadxl);

/* Get the current offsets for all axes (15.6mg/LSB) */
adxl345_status adxl345_get_offset(adxl345_handle_t* hadxl,
	adxl345_acc_offset_t* offset);

/* Set the current offsets for all axes (15.6mg/LSB) */
adxl345_status adxl345_set_offset(adxl345_handle_t* hadxl,
	adxl345_acc_offset_t* offset);

/* Start acceleration measurements */
adxl345_status adxl345_start(adxl345_handle_t* hadxl);

/* Stop acceleration measurements */
adxl345_status adxl345_stop(adxl345_handle_t* hadxl);

/* Get the acceleration data for all axes */
adxl345_status adxl345_get_data(adxl345_handle_t* hadxl,
	adxl345_acc_data_t* data);

/* Flush internal FIFO */
adxl345_status adxl345_fifo_flush(adxl345_handle_t* hadxl);

/*----------------------------------------------------------------------
  Middle-level functions
----------------------------------------------------------------------*/

/* Write the device register */
void adxl345_write_reg(adxl345_handle_t* hadxl, uint8_t reg,
	uint8_t value);

/* Read the device register */
uint8_t adxl345_read_reg(adxl345_handle_t* hadxl, uint8_t reg);

/* Read the device ID register */
uint8_t adxl345_get_id(adxl345_handle_t* hadxl);

/* Read the INT_SOURCE register */
uint8_t adxl345_get_int_src(adxl345_handle_t* hadxl);

/* Get FIFO count */
uint8_t adxl345_get_fifo_cnt(adxl345_handle_t* hadxl);

/*----------------------------------------------------------------------
  Low-level functions
----------------------------------------------------------------------*/

/* Write registers through SPI */
adxl345_status adxl345_write_spi(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size);

/* Read registers through SPI */
adxl345_status adxl345_read_spi(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size);

/* Write registers through SPI using DMA */
adxl345_status adxl345_write_spi_dma(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size);

/* Read registers through SPI using DMA */
adxl345_status adxl345_read_spi_dma(adxl345_handle_t* hadxl,
	uint8_t addr, void* buf, size_t size);

void adxl345_spi_check_config(SPI_HandleTypeDef* hspi);

#ifdef __cplusplus
}
#endif
#endif /* ADXL345_H */
