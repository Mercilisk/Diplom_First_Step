#include "FFT_FREERTOS.h"
void FastFurieTransform(compl_t *Furie_Transform, uint32_t Length_Realization)
{
	uint32_t Length_End_Vector				=	Length_Realization;

	uint32_t Number_Elements				=	0;
	uint32_t Number_Vectors					=	1;

	uint32_t Half_Index_Vector, First_Index_Vector;

	compl_t  Exponential;

	compl_t  Assistive_Variable;
	compl_t  Assistive_Sum;

	float Arg_Exp;

	float Quantity_Vectors = 1;

    while(Length_End_Vector > 2)
	{
		Arg_Exp								=	M_2PI / Length_End_Vector * Number_Elements;

		Exponential.Real					=	Compl_Cos(&Arg_Exp);
		Exponential.Imag					=  -Compl_Sin(&Arg_Exp);

		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;
		Assistive_Sum 						=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Product(&Assistive_Sum,
															  &Exponential);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Number_Elements 					=	Number_Elements + 1;

        if (fmod((float) Number_Elements, Length_End_Vector / 2) < 1e-10)
		{
			Number_Elements 				=	0;

            if (Number_Vectors == Quantity_Vectors)
			{    
				Length_End_Vector 			=	Length_End_Vector / 2;

				modff(Length_Realization/Length_End_Vector, &Quantity_Vectors);

				Number_Vectors 				=   0;
			}
			Number_Vectors					=	Number_Vectors + 1;
		}
	}
    
    Exponential.Real                        =	1;
    Exponential.Imag                        =   0;
    // Operation NOT COMPLEX PRODUCT
    while(Length_End_Vector > 1)
	{
		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

        if (Number_Vectors == Quantity_Vectors)
		{
            Length_End_Vector 				=	Length_End_Vector / 2;

            Number_Vectors 					=   0;
		}
        Number_Vectors						=	Number_Vectors + 1;
	}
    Length_End_Vector   =   4;
    uint8_t Flag_Half_Vectors;
    uint32_t Index, Save_Ind, Offset_Vectors;
    while (Length_End_Vector <= Length_Realization)
	{
        Number_Elements                		=   1;
        Number_Vectors                 		=   1;
        Save_Ind                    		=   0;
        Flag_Half_Vectors           		=   0;
        Offset_Vectors 						=	0;

        while (Number_Elements < Length_Realization - 1)
		{
            Index                           =   Number_Elements - Offset_Vectors;

            if (Save_Ind == 0)
			{
                Save_Ind                    =   Number_Elements;
			}

            if (Index < Length_End_Vector / 2)
			{
                Assistive_Variable          =   *(Furie_Transform + Index * 2  + Offset_Vectors);
                *(Furie_Transform + Index * 2  + Offset_Vectors)
											=   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (2 * Number_Elements - Offset_Vectors);
                /*	Средство обхода элементов, которые прошёл алгоритм
                 *   после половины массива*/
                if (Flag_Half_Vectors == 1)
				{
                    (Furie_Transform + Index + Offset_Vectors) -> Blocked = 1;
                    Flag_Half_Vectors       =   0;
				}
			}
            else if (((1 - Length_End_Vector + Index * 2) != Save_Ind - Offset_Vectors) &&
                    (Save_Ind - Offset_Vectors < Length_End_Vector / 2))
			{
                Assistive_Variable          =   *(Furie_Transform + (1 - Length_End_Vector + Index * 2) + 
                													Offset_Vectors);
                *(Furie_Transform + (1 - Length_End_Vector + Index * 2) + 
                					Offset_Vectors)
                                            =   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (1 - Length_End_Vector + Number_Elements * 2 - 
                														 Offset_Vectors);
                Flag_Half_Vectors           =   1;
			}
            else if (Save_Ind - Offset_Vectors < Length_End_Vector / 2)
			{    
                Flag_Half_Vectors           =   1;
                Number_Elements             =   Save_Ind + 2;
                while (Flag_Half_Vectors == 1)
				{
                    if ((Furie_Transform + Number_Elements) -> Blocked == 1)
					{
                        (Furie_Transform + Number_Elements) -> Blocked = 0;
                        Number_Elements     =   Number_Elements + 2;
					}
                    else
					{
                        Flag_Half_Vectors       =   0;
					}
				}
                Save_Ind                    =   Number_Elements;
			}
            else
			{
                Number_Vectors              =   Number_Vectors + 1;
            	Offset_Vectors 				=	(Number_Vectors - 1) * Length_End_Vector;
                Number_Elements             =   Offset_Vectors + 1;
                Save_Ind                    =   0;
                Flag_Half_Vectors           =   0;
                continue;
			}
                
		}

        Length_End_Vector                   =   Length_End_Vector * 2;
	}

}

void SignalFastFurieTransform(float *Signal, compl_t *Furie_Transform, uint32_t Length_Realization)
{
	uint32_t Length_End_Vector				=	Length_Realization;

	uint32_t Number_Elements				=	0;
	uint32_t Number_Vectors					=	1;

	uint32_t Half_Index_Vector, First_Index_Vector;

	compl_t  Exponential;

	compl_t  Assistive_Variable;
	compl_t  Assistive_Sum;
	float    Assistive_fSum;

	float Arg_Exp;

	float Quantity_Vectors = 1;

	while(fmod((float) Number_Elements, Length_End_Vector / 2) < 1e-10)
	{
		Arg_Exp								=	M_2PI / Length_End_Vector * Number_Elements;

		Exponential.Real					=	Compl_Cos(&Arg_Exp);
		Exponential.Imag					=  -Compl_Sin(&Arg_Exp);

		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_fSum 						=	*(Signal + First_Index_Vector) -
												*(Signal + Half_Index_Vector);

		*(Furie_Transform + Half_Index_Vector)
											=	fCompl_Product(&Assistive_fSum,
															   &Exponential);

		(Furie_Transform + Half_Index_Vector) -> Blocked 	= 	0;

		(Furie_Transform + First_Index_Vector) -> Real
											=	*(Signal + First_Index_Vector) +
												*(Signal + Half_Index_Vector);

		(Furie_Transform + First_Index_Vector) -> Imag  	=	0;
		(Furie_Transform + First_Index_Vector) -> Blocked 	= 	0;

		Number_Elements 					=	Number_Elements + 1;

	}

	Length_End_Vector 						=	Length_End_Vector / 2;

	modff(Length_Realization/Length_End_Vector, &Quantity_Vectors);

    while(Length_End_Vector > 2)
	{
		Arg_Exp								=	M_2PI / Length_End_Vector * Number_Elements;

		Exponential.Real					=	Compl_Cos(&Arg_Exp);
		Exponential.Imag					=  -Compl_Sin(&Arg_Exp);

		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;
		Assistive_Sum 						=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Product(&Assistive_Sum,
															  &Exponential);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Number_Elements 					=	Number_Elements + 1;

        if (fmod((float) Number_Elements, Length_End_Vector / 2) < 1e-10)
		{
			Number_Elements 				=	0;

            if (Number_Vectors == Quantity_Vectors)
			{
				Length_End_Vector 			=	Length_End_Vector / 2;

				modff(Length_Realization/Length_End_Vector, &Quantity_Vectors);

				Number_Vectors 				=   0;
			}
			Number_Vectors					=	Number_Vectors + 1;
		}
	}

    Exponential.Real                        =	1;
    Exponential.Imag                        =   0;
    // Operation NOT COMPLEX PRODUCT
    while(Length_End_Vector > 1)
	{
		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

        if (Number_Vectors == Quantity_Vectors)
		{
            Length_End_Vector 				=	Length_End_Vector / 2;

            Number_Vectors 					=   0;
		}
        Number_Vectors						=	Number_Vectors + 1;
	}
    Length_End_Vector   =   4;
    uint8_t Flag_Half_Vectors;
    uint32_t Index, Save_Ind, Offset_Vectors;
    while (Length_End_Vector <= Length_Realization)
	{
        Number_Elements                		=   1;
        Number_Vectors                 		=   1;
        Save_Ind                    		=   0;
        Flag_Half_Vectors           		=   0;
        Offset_Vectors 						=	0;

        while (Number_Elements < Length_Realization - 1)
		{
            Index                           =   Number_Elements - Offset_Vectors;

            if (Save_Ind == 0)
			{
                Save_Ind                    =   Number_Elements;
			}

            if (Index < Length_End_Vector / 2)
			{
                Assistive_Variable          =   *(Furie_Transform + Index * 2  + Offset_Vectors);
                *(Furie_Transform + Index * 2  + Offset_Vectors)
											=   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (2 * Number_Elements - Offset_Vectors);
                /*	Средство обхода элементов, которые прошёл алгоритм
                 *   после половины массива*/
                if (Flag_Half_Vectors == 1)
				{
                    (Furie_Transform + Index + Offset_Vectors) -> Blocked = 1;
                    Flag_Half_Vectors       =   0;
				}
			}
            else if (((1 - Length_End_Vector + Index * 2) != Save_Ind - Offset_Vectors) &&
                    (Save_Ind - Offset_Vectors < Length_End_Vector / 2))
			{
                Assistive_Variable          =   *(Furie_Transform + (1 - Length_End_Vector + Index * 2) +
                													Offset_Vectors);
                *(Furie_Transform + (1 - Length_End_Vector + Index * 2) +
                					Offset_Vectors)
                                            =   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (1 - Length_End_Vector + Number_Elements * 2 -
                														 Offset_Vectors);
                Flag_Half_Vectors           =   1;
			}
            else if (Save_Ind - Offset_Vectors < Length_End_Vector / 2)
			{
                Flag_Half_Vectors           =   1;
                Number_Elements             =   Save_Ind + 2;
                while (Flag_Half_Vectors == 1)
				{
                    if ((Furie_Transform + Number_Elements) -> Blocked == 1)
					{
                        (Furie_Transform + Number_Elements) -> Blocked = 0;
                        Number_Elements     =   Number_Elements + 2;
					}
                    else
					{
                        Flag_Half_Vectors       =   0;
					}
				}
                Save_Ind                    =   Number_Elements;
			}
            else
			{
                Number_Vectors              =   Number_Vectors + 1;
            	Offset_Vectors 				=	(Number_Vectors - 1) * Length_End_Vector;
                Number_Elements             =   Offset_Vectors + 1;
                Save_Ind                    =   0;
                Flag_Half_Vectors           =   0;
                continue;
			}

		}

        Length_End_Vector                   =   Length_End_Vector * 2;
	}

}

void SignalFastFurieTransformFREERTOS(float *Signal, compl_t *Furie_Transform, uint32_t Length_Realization,
	SemaphoreHandle_t Status_Furie_Ready)
{
	uint32_t Length_End_Vector				=	Length_Realization;

	uint32_t Number_Elements				=	0;
	uint32_t Number_Vectors					=	1;

	uint32_t Half_Index_Vector, First_Index_Vector;

	compl_t  Exponential;

	compl_t  Assistive_Variable;
	compl_t  Assistive_Sum;
	float    Assistive_fSum;

	float Arg_Exp;

	float Quantity_Vectors = 1;

	while(fmod((float) Number_Elements, Length_End_Vector / 2) < 1e-10)
	{
		Arg_Exp								=	M_2PI / Length_End_Vector * Number_Elements;

		Exponential.Real					=	Compl_Cos(&Arg_Exp);
		Exponential.Imag					=  -Compl_Sin(&Arg_Exp);

		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_fSum 						=	*(Signal + First_Index_Vector) -
												*(Signal + Half_Index_Vector);

		*(Furie_Transform + Half_Index_Vector)
											=	fCompl_Product(&Assistive_fSum,
															   &Exponential);

		(Furie_Transform + Half_Index_Vector) -> Blocked 	= 	0;

		(Furie_Transform + First_Index_Vector) -> Real
											=	*(Signal + First_Index_Vector) +
												*(Signal + Half_Index_Vector);

		(Furie_Transform + First_Index_Vector) -> Imag  	=	0;
		(Furie_Transform + First_Index_Vector) -> Blocked 	= 	0;

		Number_Elements 					=	Number_Elements + 1;

	}

	xSemaphoreGive(Status_Furie_Ready);

	Length_End_Vector 						=	Length_End_Vector / 2;

	modff(Length_Realization/Length_End_Vector, &Quantity_Vectors);

    while(Length_End_Vector > 2)
	{
		Arg_Exp								=	M_2PI / Length_End_Vector * Number_Elements;

		Exponential.Real					=	Compl_Cos(&Arg_Exp);
		Exponential.Imag					=  -Compl_Sin(&Arg_Exp);

		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;
		Assistive_Sum 						=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Product(&Assistive_Sum,
															  &Exponential);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Number_Elements 					=	Number_Elements + 1;

        if (fmod((float) Number_Elements, Length_End_Vector / 2) < 1e-10)
		{
			Number_Elements 				=	0;

            if (Number_Vectors == Quantity_Vectors)
			{
				Length_End_Vector 			=	Length_End_Vector / 2;

				modff(Length_Realization/Length_End_Vector, &Quantity_Vectors);

				Number_Vectors 				=   0;
			}
			Number_Vectors					=	Number_Vectors + 1;
		}
	}

    Exponential.Real                        =	1;
    Exponential.Imag                        =   0;
    // Operation NOT COMPLEX PRODUCT
    while(Length_End_Vector > 1)
	{
		First_Index_Vector					=	Number_Elements + (Number_Vectors - 1) * Length_End_Vector;
		Half_Index_Vector 					=	First_Index_Vector + Length_End_Vector / 2;

		Assistive_Variable.Real				=  -(Furie_Transform + Half_Index_Vector) -> Real;
		Assistive_Variable.Imag				=  -(Furie_Transform + Half_Index_Vector) -> Imag;

		*(Furie_Transform + Half_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

		Assistive_Variable.Real				=  -Assistive_Variable.Real;
		Assistive_Variable.Imag				=  -Assistive_Variable.Imag;

		*(Furie_Transform + First_Index_Vector)
											=	Compl_Sum((Furie_Transform + First_Index_Vector),
														  &Assistive_Variable);

        if (Number_Vectors == Quantity_Vectors)
		{
            Length_End_Vector 				=	Length_End_Vector / 2;

            Number_Vectors 					=   0;
		}
        Number_Vectors						=	Number_Vectors + 1;
	}
    Length_End_Vector   =   4;
    uint8_t Flag_Half_Vectors;
    uint32_t Index, Save_Ind, Offset_Vectors;
    while (Length_End_Vector <= Length_Realization)
	{
        Number_Elements                		=   1;
        Number_Vectors                 		=   1;
        Save_Ind                    		=   0;
        Flag_Half_Vectors           		=   0;
        Offset_Vectors 						=	0;

        while (Number_Elements < Length_Realization - 1)
		{
            Index                           =   Number_Elements - Offset_Vectors;

            if (Save_Ind == 0)
			{
                Save_Ind                    =   Number_Elements;
			}

            if (Index < Length_End_Vector / 2)
			{
                Assistive_Variable          =   *(Furie_Transform + Index * 2  + Offset_Vectors);
                *(Furie_Transform + Index * 2  + Offset_Vectors)
											=   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (2 * Number_Elements - Offset_Vectors);
                /*	Средство обхода элементов, которые прошёл алгоритм
                 *   после половины массива*/
                if (Flag_Half_Vectors == 1)
				{
                    (Furie_Transform + Index + Offset_Vectors) -> Blocked = 1;
                    Flag_Half_Vectors       =   0;
				}
			}
            else if (((1 - Length_End_Vector + Index * 2) != Save_Ind - Offset_Vectors) &&
                    (Save_Ind - Offset_Vectors < Length_End_Vector / 2))
			{
                Assistive_Variable          =   *(Furie_Transform + (1 - Length_End_Vector + Index * 2) +
                													Offset_Vectors);
                *(Furie_Transform + (1 - Length_End_Vector + Index * 2) +
                					Offset_Vectors)
                                            =   *(Furie_Transform + Save_Ind);
                *(Furie_Transform + Save_Ind)
											=   Assistive_Variable;
                Number_Elements             =   (1 - Length_End_Vector + Number_Elements * 2 -
                														 Offset_Vectors);
                Flag_Half_Vectors           =   1;
			}
            else if (Save_Ind - Offset_Vectors < Length_End_Vector / 2)
			{
                Flag_Half_Vectors           =   1;
                Number_Elements             =   Save_Ind + 2;
                while (Flag_Half_Vectors == 1)
				{
                    if ((Furie_Transform + Number_Elements) -> Blocked == 1)
					{
                        (Furie_Transform + Number_Elements) -> Blocked = 0;
                        Number_Elements     =   Number_Elements + 2;
					}
                    else
					{
                        Flag_Half_Vectors       =   0;
					}
				}
                Save_Ind                    =   Number_Elements;
			}
            else
			{
                Number_Vectors              =   Number_Vectors + 1;
            	Offset_Vectors 				=	(Number_Vectors - 1) * Length_End_Vector;
                Number_Elements             =   Offset_Vectors + 1;
                Save_Ind                    =   0;
                Flag_Half_Vectors           =   0;
                continue;
			}

		}

        Length_End_Vector                   =   Length_End_Vector * 2;
	}

}

void InvertFastFurieTransform(compl_t *Furie_Transform2Invert_Furie_Transform, uint32_t Length_Realization)
{

	for(uint32_t Index_Furie = 0; Index_Furie < Length_Realization; Index_Furie++)
	{
		Compl_Pairing((Furie_Transform2Invert_Furie_Transform + Index_Furie));
	}

	FastFurieTransform(Furie_Transform2Invert_Furie_Transform, Length_Realization);

	for(uint32_t Index_Furie = 0; Index_Furie < Length_Realization; Index_Furie++)
	{
		Compl_Pairing((Furie_Transform2Invert_Furie_Transform + Index_Furie));
		Furie_Transform2Invert_Furie_Transform -> Real
							=	Furie_Transform2Invert_Furie_Transform -> Real / Length_Realization;
		Furie_Transform2Invert_Furie_Transform -> Imag
							=	Furie_Transform2Invert_Furie_Transform -> Imag / Length_Realization;
	}

}

void Hilbert(float *Signal, compl_t *Signal2Hilbert, uint32_t Length_Realization, SemaphoreHandle_t Status_Furie_Ready)
{

	SignalFastFurieTransformFREERTOS(Signal, Signal2Hilbert, Length_Realization,
			Status_Furie_Ready);

	for(uint32_t Index_2Half_SH = 0; Index_2Half_SH < Length_Realization / 2; Index_2Half_SH++)
	{
		(Signal2Hilbert + Index_2Half_SH) -> Real  = 2 * (Signal2Hilbert + Index_2Half_SH) -> Real;
		(Signal2Hilbert + Index_2Half_SH) -> Imag  = 2 * (Signal2Hilbert + Index_2Half_SH) -> Imag;
		(Signal2Hilbert + Index_2Half_SH + Length_Realization/2) -> Real = 0;
		(Signal2Hilbert + Index_2Half_SH + Length_Realization/2) -> Imag = 0;
	}

	InvertFastFurieTransform(Signal2Hilbert, Length_Realization);

}

void Envelope(float *Envelope, float *Signal, compl_t *Signal2Hilbert, uint32_t Length_Realization, SemaphoreHandle_t Status_Furie_Ready)
{

	Hilbert(Signal, Signal2Hilbert, Length_Realization, Status_Furie_Ready);

	for(uint32_t Index = 0; Index < Length_Realization; Index++)
	{
		*(Envelope + Index) = Compl_Abs((Signal2Hilbert + Index));

		(Signal2Hilbert + Index) -> Imag = 0;		//	Refrash Signal
	}

}


