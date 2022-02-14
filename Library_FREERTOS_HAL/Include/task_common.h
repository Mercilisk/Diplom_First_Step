/* task_common header file - edit for customize */
#ifndef TASK_COMMON_H
#define TASK_COMMON_H
#ifdef __cplusplus
 extern "C" {
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define ONE_HOUR 60*60*1000
#define ONE_DAY 24*60*60*1000

/* ADXL345 task handle */
#define ADXL345_HTASK 												\
{																	\
	.hadxl =                                                        \
	{                                                               \
		.spi =                                                      \
		{                                                           \
			.spi_rtos = &spi2_rtos,                                 \
			.nss =                                                  \
			{                                                       \
				.port = ADXL_NSS_GPIO_Port,                         \
				.pin = ADXL_NSS_Pin                                 \
			}                                                       \
		},                                                          \
		.int1 =                                                     \
		{                                                           \
			.port = ADXL_INT1_GPIO_Port,                            \
			.pin = ADXL_INT1_Pin,                                   \
		},                                                          \
		.int2 =                                                     \
		{                                                           \
			.port = ADXL_INT2_GPIO_Port,                            \
			.pin = ADXL_INT2_Pin,                                   \
		},                                                          \
		.settings =                                                 \
		{                                                           \
			.data_rate = ADXL345_DATARATE_3200_HZ,					\
			.meas_range = ADXL345_RANGE_16_G,                       \
			.resolution = ADXL345_RESOLUTION_FULL,                  \
			.align = ADXL345_ALIGN_LSB,                             \
			.offset =                                               \
			{                                                       \
				.x = 0,                                             \
				.y = 0,                                             \
				.z = 0                                              \
			},                                                      \
			.int_en = ADXL345_INT_OVERRUN|ADXL345_INT_WATERMARK, 	\
			.map_to_int2 = ADXL345_INT_OVERRUN,  					\
			.fifo_mode = ADXL345_FIFO_FIFO,                         \
			.fifo_trigger = 0, 										\
			.fifo_watermark = 16                        			\
		},                                                          \
		.mutex_timeout = portMAX_DELAY,                             \
		.transfer_timeout = portMAX_DELAY,                          \
	},                                                              \
	.suspend_notify = NULL,                                         \
	.fifo_frame_ptr_queue = NULL,                                   \
	.fifo_frame_qty = 8,                                            \
	.fifo_frame_size = 16*8                                         \
}

/* 3rd orger Butterworth HPF fc=10 Hz + 3rd orger Butterworth LPF fc=1000 Hz */

#define FILTER_NUM_SECTIONS	4
#define FILTER_COEFF                                                \
{                                                                   \
	{                                                               \
		.b0 = 1.0f,                                                 \
		.b1 = -2.0f,                                                \
		.b2 = 1.0f,                                                 \
		.a1 = -1.980175403518735999242039724776986986399f,          \
		.a2 = 0.980557174764986827497637023043353110552f,           \
		.g  = 0.990183144570930706684919186955085024238f            \
	},                                                              \
	{                                                               \
		.b0 = 1.0f,                                                 \
		.b1 = -1.0f,                                                \
		.b2 = 0.0f,                                                 \
		.a1 = -0.980555318909953510519983410631539300084f,          \
		.a2 = 0.0f,                                                 \
		.g  = 0.990277659454976810771142936573596671224f            \
	},                                                              \
	{                                                               \
		.b0 = 1.0f,                                                 \
		.b1 = 2.0f,                                                 \
		.b2 = 1.0f,                                                 \
		.a1 = 0.523528316553325656634854112780885770917f,           \
		.a2 = 0.36804541894530312662325854944356251508f,            \
		.g  = 0.472893433874657154181164742112741805613f            \
	},                                                              \
	{                                                               \
		.b0 = 1.0f,                                                 \
		.b1 = 1.0f,                                                 \
		.b2 = 0.0f,                                                 \
		.a1 = 0.198912367379658006072418174881022423506f,           \
		.a2 = 0.0f,                                                 \
		.g  = 0.599456183689829003036209087440511211753f            \
	}                                                               \
}

#define VIBROV_HTASK                                                \
{                                                                   \
	.dsp =                                                          \
	{                                                               \
		.sample_rate_hz = 3200,									    \
		.frame_size = 16*8,                                         \
		.skip_frames = 8,								            \
		.frame_num = 42,								            \
		.filter_x =                                                 \
		{                                                           \
			.num_sections = FILTER_NUM_SECTIONS,                    \
			.coeff = filter_coeff									\
		},                                                          \
		.filter_y =                                                 \
		{                                                           \
			.num_sections = FILTER_NUM_SECTIONS,                    \
			.coeff = filter_coeff									\
		},                                                          \
		.filter_z =                                                 \
		{                                                           \
			.num_sections = FILTER_NUM_SECTIONS,                    \
			.coeff = filter_coeff									\
		},                                                          \
		.capture = NULL,                                            \
		.convert = NULL,                                            \
	},                                                              \
	.therm =                                                        \
	{                                                               \
		.hds =                                                      \
		{                                                           \
			.ow = &uart2_rtos,                                      \
			.settings =                                             \
			{                                                       \
				.resolution = DS18B20_12BITS,                       \
				.tl = 0,                                            \
				.th = 0,                                            \
			},                                                      \
			.mutex_timeout = 10,                                    \
			.transfer_timeout = 200                                 \
		},                                                          \
		.adc = &adc_rtos,                                           \
		.vref_rank = ADC_INJECTED_RANK_1,                           \
		.temp_rank = ADC_INJECTED_RANK_2,                           \
		.temp_source = TEMPSRC_DS18B20,                             \
		.adc_temp_offset_calib = 0                                  \
	},                                                              \
	.adxl_queue = NULL,                                      		\
	.port = 2,                                                      \
	.period_meas = 120*1000,                                     	\
	.period_meas_rnd = 2000,                                        \
	.amount_join_attempt = 10,										\
 	.period_join_attempt = 5*60*1000,								\
 	.period_join_attempt_rnd = 0x0000007F,							\
 	.amount_join_attempt_1hour = 6,									\
	.amount_join_attempt_24hour = 7,								\
	.every_n_confirm_msg = 30,										\
	.amount_ack_attempt = 5,										\
	.WKUP_delay	= 15000												\
}

#define VIBROV_SYSTEM_HTASK									\
{   														\
 	.rx_queue = NULL,                                     	\
	.port = 5,                                          	\
	.msg_queue_depth = 2,                               	\
	.buf_size = 128											\
}

#define LORAMAC_APP_HTASK 										\
{																\
	.led =                                                      \
	{                                                       	\
		.port = LED_GPIO_Port,									\
		.pin = LED_Pin											\
	},                                                       	\
	.tx_period = 20000,											\
	.tx_period_rnd = 1000										\
}

/* LoRaMAC task handle */
#define LORAMAC_HTASK 											\
{																\
	.iface = 													\
	{															\
		.spi =													\
		{														\
			.spi_rtos = &spi3_rtos,								\
			.nss =												\
			{													\
				.port = SX_NSS_GPIO_Port,						\
				.pin = SX_NSS_Pin								\
			}													\
		},														\
		.dio =													\
		{														\
			{													\
				.port = SX_DIO0_GPIO_Port,						\
				.pin = SX_DIO0_Pin								\
			},													\
			{													\
				.port = SX_DIO1_GPIO_Port,						\
				.pin = SX_DIO1_Pin								\
			},													\
			{													\
				.port = SX_DIO2_GPIO_Port,						\
				.pin = SX_DIO2_Pin								\
			},													\
			{													\
				.port = SX_DIO3_GPIO_Port,						\
				.pin = SX_DIO3_Pin								\
			},													\
			{													\
				.port = SX_DIO4_GPIO_Port,						\
				.pin = SX_DIO4_Pin								\
			},													\
			{													\
				.port = SX_DIO5_GPIO_Port,						\
				.pin = SX_DIO5_Pin								\
			}													\
		},														\
		.nrst =													\
		{														\
			.port = SX_NRST_GPIO_Port,							\
			.pin = SX_NRST_Pin									\
		}														\
	},															\
	.device_class = CLASS_A,									\
	.lmparams = 												\
	{															\
		.Region = LORAMAC_REGION_RU864,							\
		.AdrEnable = LORAMAC_HANDLER_ADR_ON,					\
		.TxDatarate = DR_0,										\
		.PublicNetworkEnable = true,							\
		.DutyCycleEnabled = false,								\
		.DataBufferMaxSize = 242								\
	},															\
	.uplink_msg_type = LORAMAC_HANDLER_UNCONFIRMED_MSG,			\
	.uplink_queue_depth = 1										\
}


/* Common tasks init */
void task_common_init(void);
void peripherial_init(void);
void peripherial_deinit(void);
#ifdef __cplusplus
}
#endif
#endif /* PERITH_COMMON_H */
