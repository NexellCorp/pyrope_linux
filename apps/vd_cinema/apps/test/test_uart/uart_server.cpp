//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <CNX_Uart.h>
#include <crc32.h>
#include <NX_Utils.h>

#include <NX_UartProtocol.h>
#include <CNX_GpioControl.h>

#define MAX_BUFFER_SIZE		1024

//------------------------------------------------------------------------------
int32_t ReadData(CNX_Uart *hUart, uint8_t *pBuf, int32_t size)
{
	int32_t readSize, totalSize=0;
	do
	{
		readSize = hUart->Read( pBuf, size );
		if( readSize < 0 )
			return -1;

		size -= readSize;
		pBuf += readSize;
		totalSize += readSize;
		printf("readSize = %d\n", readSize);
	}while(size > 0);
	return totalSize;
}

//------------------------------------------------------------------------------
int main( void )
{
	CNX_Uart *hUart = new CNX_Uart();
	uint8_t buf[4096], tmp;
	int32_t readSize;
	uint32_t startCode=0;
	uint32_t frameCount;
	int32_t length;
	uint8_t testbuf[4] = {0, 0, 0, 0};

	NX_UART_PACKET rcvPacket;

	// CNX_GpioControl gpioD12;
	// gpioD12.Init( GPIOD12 );
	// gpioD12.SetDirection( GPIO_DIRECTION_OUT );
	// gpioD12.SetValue( 0 );

	if( !hUart->Init( 0 ) )
	{
		printf("Uart Init Error!!!\n");
		goto ERROR;
	}

	while(1)
	{
		memset( buf, 0, sizeof(buf));
		startCode = 0;

		//
		//	Find Start Code
		//
		do{
			readSize = ReadData( hUart, &tmp, 1 );
			if( readSize != 1 )
			{
				printf("Read Error!!!\n");
				goto ERROR;
			}
			startCode = (startCode<<8) | tmp;
			if( startCode == UART_START_CODE )
			{
				break;
			}

		}while(1);

		//	Read Length
		readSize = ReadData( hUart, buf, 4 );
		if( readSize != 4 )
		{
			printf("Read Length Error!!!\n");
			break;
		}
		length = buf[0]<<24 | buf[1]<<8 | buf[2]<<8 | buf[3];

		printf("length = %d\n", length);

		if( length < 0 || length > 4096 )
		{
			continue;
		}

		//	Read Remained Data
		readSize = ReadData( hUart, buf+4, length );
		if( readSize != length )
		{
			printf("Read Error!!!\n");
			break;
		}

		//	Check Data Sanity
		if( 0 != NX_CheckUartPacket( buf+4, length ) )
		{
			printf("~~~~~~~~~~~~~~ Packet Error !!!!!\n");
			//DumpUartPacket(&rcvPacket);
			HexDump( buf, length+4 );
		}
		HexDump( buf, length+4 );

		//	Parsing Uart Command
		if( 0 != NX_ParseUartPacket( buf, length +4, &rcvPacket ) )
		{
			//DumpUartPacket(&rcvPacket);
			printf("Packet Parsing Error");
		}
	}

	delete hUart;
	return 0;

ERROR :
	delete hUart;
	return -1;

}
