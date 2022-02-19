#include "FreeRTOS.h"
#include "exti_freertos.h"
#include "task.h"

/* External interrupt callbacks list */


/* Append the item at the end of list */
static exti_freertos_list_t* exti_freertos_list_append(uint16_t pin,
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken));
/* Delete the item from list */
static exti_freertos_list_t* exti_freertos_list_delete(uint16_t pin);


/* register handler on EXTI GPIO pin */
BaseType_t exti_freertos_register(uint16_t pin,
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken))
{
	if(exti_freertos_find_item(pin) != NULL)
		return pdFALSE;
	taskENTER_CRITICAL();
	exti_freertos_list_append(pin, handler);
	taskEXIT_CRITICAL();
	return pdTRUE;
}

/* unregister handler on EXTI GPIO pin */
void exti_freertos_unregister(uint16_t pin)
{
	taskENTER_CRITICAL();
	exti_freertos_list_delete(pin);
	taskEXIT_CRITICAL();
}

/* EXTI ISR */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	exti_freertos_list_t *service = exti_freertos_find_item(GPIO_Pin);
	if(service == NULL) return;
	service->handler(GPIO_Pin, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static exti_freertos_list_t* exti_freertos_list_begin = NULL;

/* Append the item at the end of list */
static exti_freertos_list_t* exti_freertos_list_append(uint16_t pin,
	void (*handler)(uint16_t pin, BaseType_t* pxHigherPriorityTaskWoken))
{
	/* creating item */
	exti_freertos_list_t* new_item = (exti_freertos_list_t*)
		pvPortMalloc(sizeof(exti_freertos_list_t));
	new_item->pin = pin;
	new_item->handler = handler;
	new_item->next = NULL;
	/* if list is empty, create and return the begin of list */
	if(exti_freertos_list_begin == NULL)
		exti_freertos_list_begin = new_item;
	/* else find end of list and append item at the end of list */
	else
	{
		exti_freertos_list_t *item = exti_freertos_list_begin;
		while(item->next != NULL)
			item = item->next;
		item->next = new_item;
	}
	return new_item;
}

/* Delete the item from list */
static exti_freertos_list_t* exti_freertos_list_delete(uint16_t pin)
{
	exti_freertos_list_t *item = exti_freertos_list_begin, *tmp;
	/* find list item */
	while(item != NULL)
	{
		/* if item found, delete and replace next item pointer */
		if(item->next->pin == pin)
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

/* Find item in list with mached pin */
exti_freertos_list_t* exti_freertos_find_item(uint16_t pin)
{
	exti_freertos_list_t *item = exti_freertos_list_begin;
	while(item != NULL)
	{
		if(item->pin == pin)
			break;
		item = item->next;
	}
	return item;
}
