

#ifndef INCLUDE_COMPL_FREERTOS_H_
#define INCLUDE_COMPL_FREERTOS_H_

#include "math.h"
#include "stdint.h"

#define M_2PI						(float) (2 * M_PI)

/*	Data Type Declaration	*/
typedef struct
{

	float 					Real;
	float 					Imag;
	uint8_t					Blocked;

}compl_t;

typedef struct
{

	float 					Abs;
	float 					Arg;

}compl_Exp_form_t;

/********************************************************************************/
/*								Function Convert 								*/
/********************************************************************************/

compl_t Exp2Algebraic (compl_Exp_form_t *Exp);

compl_Exp_form_t Algebraic2Exp(compl_t *Algebraic);

/********************************************************************************/
/*						Function Complex Operation 								*/
/********************************************************************************/

compl_t Compl_Sum(compl_t *Item_1, compl_t *Item_2);

compl_t Compl_Product(compl_t *Item_1, compl_t *Item_2);

compl_t fCompl_Product(float *Item_1, compl_t *Item_2);

compl_t Compl_Division(compl_t *Item_1, compl_t *Item_2);

compl_t Compl_Exponentiation(compl_t *Item_1, uint16_t *Indication);

void Compl_Pairing(compl_t *Item_1);

/********************************************************************************/
/*						Function Trigonometrick Operation 						*/
/********************************************************************************/

float Compl_Sin(float *Arg);

float Compl_Cos(float *Arg);

float Compl_ATan(float *Value);

float Compl_Abs(compl_t *Value);

/********************************************************************************/
/*						      Mathematics Function 						        */
/********************************************************************************/

double fact(uint32_t Value);

float BinaryPower(float Power, uint32_t Exponent);

#endif /* INCLUDE_COMPL_FREERTOS_H_ */
