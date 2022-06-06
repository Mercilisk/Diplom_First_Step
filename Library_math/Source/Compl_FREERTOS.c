

#include "Compl_FREERTOS.h"
#include "math.h"
#include "stdlib.h"
/********************************************************************************/
/*								Function Convert 								*/
/********************************************************************************/

compl_t Exp2Algebraic (compl_Exp_form_t *Exp)
{

	compl_t	Algebraic					=	{0, 0};

	Algebraic.Real 						=	Exp->Abs * Compl_Cos( &(Exp -> Arg) );

	Algebraic.Real 						=	Exp->Abs * Compl_Sin( &(Exp -> Arg) );

	return(Algebraic);

}

compl_Exp_form_t Algebraic2Exp(compl_t *Algebraic)
{

	compl_Exp_form_t	Exp 			=	{0, 0};

	Exp.Abs 							=	Compl_Abs( (compl_t *) Algebraic);

	float Arg 							=	(Algebraic->Imag / Algebraic->Real);

	Exp.Arg 							=	Compl_ATan( &Arg );

	return(Exp);

}

/********************************************************************************/
/*						Function Complex Operation 								*/
/********************************************************************************/

compl_t Compl_Sum(compl_t *Item_1, compl_t *Item_2)
{

	compl_t	Sum 						= 	{0, 0};

	Sum.Real							=	Item_1->Real + Item_2->Real;

	Sum.Imag							=	Item_1->Imag + Item_2->Imag;

	return(Sum);

}

compl_t Compl_Product(compl_t *Item_1, compl_t *Item_2)
{
	compl_t Prod 						= 	{0, 0};

	Prod.Real							=	(Item_1->Real * Item_2->Real) - (Item_1->Imag * Item_2->Imag);

	Prod.Imag							=	(Item_1->Real * Item_2->Imag) + (Item_1->Imag * Item_2->Real);

	return(Prod);
}

compl_t fCompl_Product(float *Item_1, compl_t *Item_2)
{
	compl_t Prod 						= 	{0, 0};

	Prod.Real							=	Item_2 -> Real * (*Item_1);

	Prod.Imag							=	Item_2 -> Imag * (*Item_1);

	return(Prod);
	}

compl_t Compl_Division(compl_t *Item_1, compl_t *Item_2)
{
	compl_t Div 						= 	{0, 0};

	Div.Real							=	((Item_1->Real * Item_2->Real) + (Item_1->Imag * Item_2->Imag)) /
											/*--------------------------------------------------------------*/
											   (Item_2->Real * Item_2->Real + Item_2->Imag * Item_2->Imag);

	Div.Imag							=	((Item_1->Real * Item_2->Imag) - (Item_1->Imag * Item_2->Real)) /
											/*--------------------------------------------------------------*/
											   (Item_2->Real * Item_2->Real + Item_2->Imag * Item_2->Imag);

	return(Div);
}

compl_t Compl_Exponentiation(compl_t *Item_1, uint16_t *Indication)
{

	compl_Exp_form_t Exp 				=	Algebraic2Exp(Item_1);

	Exp.Abs 							=	pow(Exp.Abs, *Indication);

	Exp.Arg								=	Exp.Arg * (*Indication);

	return (Exp2Algebraic(&Exp));

}

void Compl_Pairing(compl_t *Item_1)
{

	Item_1 -> Imag = -Item_1 -> Imag;

	return;

}

/********************************************************************************/
/*						Function Trigonometrick Operation 						*/
/********************************************************************************/

float Compl_Sin(float *Arg)
{

	float Sin_Current					=	0;
	float Sin_Previous 					=	1;

	float Eps 							=	1e-3;

	uint32_t Iter 						=	0;

	if (*Arg > M_2PI || *Arg < - M_2PI)
	{
		*Arg  							=	fmod(*Arg, M_2PI);
	}

	while (fabs(Sin_Current - Sin_Previous) > Eps)
	{


		Sin_Previous 					=	Sin_Current;

		Sin_Current 					+=	(Iter % 2 == 0 ? 1: -1) * BinaryPower(*Arg, 2 * Iter + 1) /
											/*-------------------------------------------------------*/
																fact(2 * Iter + 1);

		Iter++;

	}

	return(Sin_Current);

}

float Compl_Cos(float *Arg)
{

	float Cos_Current					=	0;
	float Cos_Previous 					=	1;

	float Eps 							=	1e-3;

	uint32_t Iter 						=	0;

	if (*Arg > M_2PI || *Arg < - M_2PI)
	{
		*Arg  							=	fmod(*Arg, M_2PI);
	}

	while (fabs(Cos_Current - Cos_Previous) > Eps)
	{



		Cos_Previous 					=	Cos_Current;

		Cos_Current 					+=	(Iter % 2 < 1e-10 ? 1: -1) * BinaryPower(*Arg, 2 * Iter) /
											/*---------------------------------------------------*/
															    fact(2 * Iter);
		Iter++;

	}

	return(Cos_Current);

}

float Compl_ATan(float *Arg)
{

	float ATan_Current					=	1;
	float ATan_Previous 				=	0;

	float Eps 							=	1e-3;

	uint32_t Iter 						=	0;

	while (fabs(ATan_Current - ATan_Previous) > Eps)
	{

		if(*Arg > 1)
		{

			*Arg 						=	*Arg - 1;

			continue;

		}
		else if (*Arg < (-1))
		{

			*Arg 						=	*Arg + 1;

			continue;

		}

		ATan_Previous 					=	ATan_Current;

		ATan_Current 					+=	(Iter % 2 == 0 ? 1: -1) * BinaryPower(*Arg, 2 * Iter + 1) /
											/*-------------------------------------------------------*/
															      (2 * Iter + 1);
		Iter++;

	}

	return(ATan_Current);

}

float Compl_Abs(compl_t *Value)
{

	return ( sqrt( pow(Value->Real, 2) + pow(Value->Imag, 2) ) );

}

double fact(uint32_t Value)
{
	double Fact 					    =	1;
	uint16_t Iter 						=	1;

	if (Value == 0)
	{
		return( (uint16_t) 1 );
	}

	while (Iter <= Value)
	{

		Fact 							*=	Iter;

		Iter++;

	}

	return (Fact);

}

float BinaryPower(float Power, uint32_t Exponent)
{
	double Result = 1.0;
	while(Exponent != 0)
	{
		if((Exponent & 1) != 0)
		{
			Result *= Power;
		}
		Power *= Power;
		Exponent >>= 1;
   }
   return ((float) Result);
}
