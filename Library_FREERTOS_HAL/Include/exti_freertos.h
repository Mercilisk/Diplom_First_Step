#ifndef EXTI_FREERTOS_H
#define EXTI_FREERTOS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "FreeRTOS.h"

struct exti_freertos_list_t
{
	uint16_t pin;
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken);
	struct exti_freertos_list	*next;
};

typedef struct
{
	uint16_t pin;
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken);
	struct exti_freertos_list_t	*next;
}exti_freertos_list_t;


/*----------------------------------------------------------------------
  Functions
----------------------------------------------------------------------*/

/* register handler on EXTI GPIO pin */
BaseType_t exti_freertos_register(uint16_t pin,
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken));

/* unregister handler on EXTI GPIO pin */
void exti_freertos_unregister(uint16_t pin);

/* Find item in list with mached port */
exti_freertos_list_t* exti_freertos_find_item(uint16_t pin);

/* External interrupt callbacks */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif
#endif /* EXTI_FREERTOS_H */
