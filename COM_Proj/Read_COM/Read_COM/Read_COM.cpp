// Read_COM.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "windows.h"
#include "math.h"
#include "cobs.h"

/****************************************************************************/
/*							Define Declaration								*/
/****************************************************************************/

#define		TIME_OUT							1000

/****************************************************************************/
/*						Data Type Declaration								*/
/****************************************************************************/
typedef struct
{

	LPCTSTR COM_Port;              
	INT32	Baud_Rate;
	INT32	Length_Buf;

}Port_Settings_TypeDef;

typedef struct
{
              
	INT32	Baud_Rate;
	INT32	Length_Buf;

}test;

/****************************************************************************/
/*						Function Declaration								*/
/****************************************************************************/

UINT8	*Read_COM(HANDLE Port_Handle, size_t Length_Buf);
void	Close_Port(HANDLE Port_Handle);
HANDLE	Open_Port(HANDLE *P_Handle, Port_Settings_TypeDef Port_Settings);

size_t cobs_encode(const UINT8 * input, size_t length,
	UINT8 * output);
	
size_t cobs_decode(const UINT8 * input, size_t length,
	UINT8 * output);

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *FILE_ID;
	char File_Name[]							=	"Read_COM_Propertice.txt";

	/*	Read COM propertice		*/
	Port_Settings_TypeDef	Port_Settings;
	char Port_Name_Char[14]						=	"//./";
	fopen_s(&FILE_ID,(const char *) &File_Name, "r+");
		fgets(&(Port_Name_Char[4]), 6, FILE_ID);
		fscanf_s(FILE_ID, "%d", &(Port_Settings.Baud_Rate));
		fscanf_s(FILE_ID, "%d", &(Port_Settings.Length_Buf));

	INT8 Length_LPCTSTR							=	0;

	for (INT8 Symbol = 0; Symbol < 14; Symbol++)
	{

		if (Port_Name_Char[Symbol] == '\n')
		{

			Length_LPCTSTR						=	Symbol;
			break;

		}
		if (Port_Name_Char[Symbol] == '\0')
		{

			Length_LPCTSTR						=	Symbol;
			break;

		}

	}
	Length_LPCTSTR++;
	char *Port_Name_LPCTSTR						=	(char *) calloc(Length_LPCTSTR, sizeof(char));

	INT8 Symbol;

	for (Symbol = 0; Symbol < Length_LPCTSTR-1; Symbol++)
	{

		*(Port_Name_LPCTSTR + Symbol)			=	Port_Name_Char[Symbol];

	}

	*(Port_Name_LPCTSTR + Symbol)				=	'\0';

	Port_Settings.COM_Port						=	(LPCTSTR) Port_Name_LPCTSTR;
	fclose(FILE_ID);
	/*	Open COM port		*/
	HANDLE Port_Handle;

	while(1)
	{
		Port_Handle								=	Open_Port(&Port_Handle, Port_Settings);
		if (Port_Handle != INVALID_HANDLE_VALUE)
			break;
	}

	size_t cobs_buffer_size = 4 * Port_Settings.Length_Buf + 4 * Port_Settings.Length_Buf/254 + 2;

	UINT8 *Buf									=	(UINT8 *) Read_COM(Port_Handle, cobs_buffer_size);

	UINT8 *Read									=	(UINT8 *) calloc(4 * Port_Settings.Length_Buf, sizeof(UINT8));

	cobs_decode(Buf, cobs_buffer_size, Read);
	if (Buf == NULL)
	{
		return 0;
	}
	free(Buf);
	float *ptrTEST = (float *) Read; 

	FILE *Read_ID;
	char File_Name1[14]							=	{"Read_Data.txt"};
	/*File_Name1[0] = 'R';
	File_Name1[1] = 'e';
	File_Name1[2] = 'a';
	File_Name1[3] = 'd';
	File_Name1[4] = '_';
	File_Name1[5] = 'D';
	File_Name1[6] = 'a';
	File_Name1[7] = 't';
	File_Name1[8] = 'a';
	File_Name1[9] = '.';
	File_Name1[10] = 't';
	File_Name1[11] = 'x';
	File_Name1[12] = 't';
	File_Name1[13] = '\0';*/
	fopen_s(&Read_ID, File_Name1, "w");
	int index = 0;

	for(; index < Port_Settings.Length_Buf; index++)
	{

		fprintf_s(Read_ID, "%f\n", *(ptrTEST + index));

	}
	free(Read);
	fclose(Read_ID);
	Close_Port(Port_Handle);
	free(Port_Name_LPCTSTR);
	return 0;
}

UINT8 *Read_COM(HANDLE Port_Handle, size_t Length_Buf)
{
    DWORD iSize;
	UINT8* sReceivedSymbol = (UINT8*) calloc(Length_Buf, sizeof(UINT8));
	if(sReceivedSymbol == NULL)
	{
		return (NULL);
	}
	int index = 0;
	int status = 0;
	while (1)
	{
		ReadFile(Port_Handle, (sReceivedSymbol + index), 1, &iSize, 0);
		if (iSize > 0)
		{
			if(*(sReceivedSymbol + index) != 0 && status == 0)
			{
				status = 1;
				continue;
			}
			else if(*(sReceivedSymbol + index) == 0 && status == 1)
			{
				status = 2;
				continue;
			}
			else if(*(sReceivedSymbol + index) == 0 || status != 2)
			{
				continue;
			}
			else
			{
			}
			
		}
		else
		{
			return (NULL);
		}
		if(*(sReceivedSymbol + index) == 0)
		{
			break;
		}
		index++;
	}

	return (sReceivedSymbol);

}

void Close_Port(HANDLE Port_Handle)
{

	if(Port_Handle != INVALID_HANDLE_VALUE)
	{

 		CloseHandle(Port_Handle);

        Port_Handle = INVALID_HANDLE_VALUE;

 	}

}

HANDLE Open_Port(HANDLE *P_Handle, Port_Settings_TypeDef Port_Settings)
{

	HANDLE Port_Handle							=	*P_Handle;

	Port_Handle									=	CreateFile(	Port_Settings.COM_Port, 
 																GENERIC_READ,
 																0,
 																NULL,
 																OPEN_EXISTING, 
 																FILE_ATTRIBUTE_NORMAL,
 																NULL
															  );
	
 	if(Port_Handle == INVALID_HANDLE_VALUE) 
	{

		printf_s("Error Port Init");
 		
		return (Port_Handle);

 	}
	
 	//SetCommMask(Port_Handle, EV_RXCHAR);
	SetupComm(Port_Handle, Port_Settings.Length_Buf, 0);

 	COMMTIMEOUTS Comm_TimeOuts;

 	Comm_TimeOuts.ReadIntervalTimeout			= 2;
 	Comm_TimeOuts.ReadTotalTimeoutMultiplier	= 1;
 	Comm_TimeOuts.ReadTotalTimeoutConstant		= 1;

 	if(!SetCommTimeouts(Port_Handle, &Comm_TimeOuts)) 
	{

		printf_s("Error Set TimeOut");

 		Close_Port(Port_Handle);

		return (Port_Handle);

 	}
	
	DCB		Settings;

	memset(&Settings,0,sizeof(Settings));
	Settings.DCBlength							=	sizeof(DCB);

	if (!GetCommState(Port_Handle, &Settings))
	{

		printf_s("Error Get Settings Structure");

		Close_Port(Port_Handle);

		return (Port_Handle);

	}

	Settings.BaudRate							=	Port_Settings.Baud_Rate;
	/*Settings.fBinary							=	true;
	Settings.fParity							=	false;
	Settings.fOutxCtsFlow						=	false;
	Settings.fOutxDsrFlow						=	false;
	Settings.fDtrControl						=	DTR_CONTROL_ENABLE;
	Settings.fNull								=	false;
	Settings.fInX								=	true;
	Settings.fErrorChar							=	false;
	Settings.fNull								=	false;
	Settings.fRtsControl						=	RTS_CONTROL_ENABLE;
	Settings.fAbortOnError						=	false;
	Settings.XonLim								=	1;
 	Settings.XoffLim							=	4 * Port_Settings.Length_Buf;
	Settings.ByteSize							=	8;
	Settings.Parity								=	NOPARITY;
	Settings.StopBits							=	ONESTOPBIT;*/
	/*Settings.XonChar							=	!0;
 	Settings.XoffChar							=	(unsigned char)0xFF;*/
	
 	if(!SetCommState(Port_Handle, &Settings))
	{

 		Close_Port(Port_Handle);

 	}

	return (Port_Handle);

 }


/* Stuffs "length" bytes of data at the location pointed to by
 * "input", writing the output to the location pointed to by
 * "output". Returns the number of bytes written to "output".
 *
 * Remove the "restrict" qualifiers if compiling with a
 * pre-C99 C dialect.
 */
size_t cobs_encode(const UINT8 * input, size_t length,
	UINT8 * output)
{
    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    UINT8 code = 1;

    while(read_index < length)
    {
        if(input[read_index] == 0)
        {
            output[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            output[write_index++] = input[read_index++];
            code++;
            if(code == 0xFF)
            {
                output[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    output[code_index] = code;

    return write_index;
}

/* Unstuffs "length" bytes of data at the location pointed to by
 * "input", writing the output * to the location pointed to by
 * "output". Returns the number of bytes written to "output" if
 * "input" was successfully unstuffed, and 0 if there was an
 * error unstuffing "input".
 *
 * Remove the "restrict" qualifiers if compiling with a
 * pre-C99 C dialect.
 */
size_t cobs_decode(const uint8_t * input, size_t length,
	uint8_t * output)
{
    size_t read_index = 0;
    size_t write_index = 0;
    UINT8 code;
    UINT8 i;

    while(read_index < length)
    {
        code = input[read_index];

        if(read_index + code > length && code != 1)
        {
            return 0;
        }

        read_index++;

        for(i = 1; i < code; i++)
        {
            output[write_index++] = input[read_index++];
        }
        if(code != 0xFF && read_index != length)
        {
            output[write_index++] = '\0';
        }
    }

    return write_index;
}