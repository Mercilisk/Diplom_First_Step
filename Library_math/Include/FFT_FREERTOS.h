#ifndef FFT_FREERTOS_H_
#define FFT_FREERTOS_H_

#include "Compl_FREERTOS.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"

void FastFurieTransform(compl_t *Signal2Furie_Transform, uint32_t Length_Realization);

void SignalFastFurieTransform(float *Signal, compl_t *Furie_Transform, uint32_t Length_Realization);

void SignalFastFurieTransformFREERTOS(float *Signal, compl_t *Furie_Transform, uint32_t Length_Realization,
	SemaphoreHandle_t Status_Furie_Ready);

void InvertFastFurieTransform(compl_t *Furie_Transform2Invert_Furie_Transform, uint32_t Length_Realization);

void Hilbert(float *Signal, compl_t *Signal2Hilbert, uint32_t Length_Realization, SemaphoreHandle_t Status_Furie_Ready);

void Envelope(float *Envelope, float *Signal, compl_t *Signal2Hilbert, uint32_t Length_Realization, SemaphoreHandle_t Status_Furie_Ready);

#endif /* FFT_FREERTOS_H_ */
