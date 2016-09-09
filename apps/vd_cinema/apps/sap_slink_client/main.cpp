//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
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
#include <unistd.h>	//	usleep
#include <stdlib.h>	//	atoi

#include "CNX_Uart.h"
#include "NX_UartProtocol.h"
#include <NX_Utils.h>

int main( int argc, char *argv[] )
{
	const char *TestString = "1234567890abcdef";
	CNX_Uart *hUart = new CNX_Uart();
	int32_t written;
	uint32_t frameNumber = 0;

	int32_t count = 1;

	if( argc>1 )
	{
		count = atoi(argv[1]);
	}

	if( !hUart->Init( 2 ) )
	{
		printf("Uart Init Error!!!\n");
		return -1;
	}

	NX_UART_PACKET pkt;

	NX_InitPacket( &pkt );

	pkt.command = 0x12344321;
	pkt.payload = (void *)TestString;
	pkt.payloadSize = strlen(TestString);

	int32_t sendBufSize = NX_GetSendBufferSize( &pkt );
	uint8_t *sendBuffer = (uint8_t *)malloc(sendBufSize);

	int32_t sendSize;

	if( count == -1 )
	{
		count = 0x7fffffff;
	}

	for( int32_t i=0 ; i < count ; i++ )
	{
		pkt.frameNumber = frameNumber;
		sendSize = NX_MakeUartPacket(&pkt, sendBuffer, sendBufSize );
		written = hUart->Write(sendBuffer, sendSize);
		frameNumber++;
	}

	free(sendBuffer);

	delete hUart;
}
