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
#include <stdint.h>
#include <string.h>		//	memcpy
#include <NX_UartProtocol.h>
#include <crc32.h>
#include <NX_Utils.h>

int32_t NX_GetSendBufferSize( NX_UART_PACKET *pPacket )
{
	if( !pPacket )
		return -1;
	return pPacket->payloadSize + 12;
}

void NX_InitPacket( NX_UART_PACKET *pPacket )
{
	memset( pPacket, 0, sizeof(NX_UART_PACKET) );
}
//
//	Data Foramt:
//
//		Name         Length     Description
//
//		Start Code   4 bytes
//		Length       4 bytes    Command + Frame Number + Playload + CRC32's Size
//		Command      4 bytes
//		Frame Num    4 bytes
//		Payload      n bytes
//		CRC32        4 bytes    Command + Frame Number + Payload
//

int32_t NX_MakeUartPacket( NX_UART_PACKET *pPacket, void *pOutBuf, int32_t outBufSize )
{
	uint8_t *pBuf = (uint8_t *)pOutBuf;
	uint32_t length, crc32;
	uint8_t *pCrcStart;
	if( !pPacket || !pPacket->payload )

	//	Size Check
	if( (pPacket->payloadSize+12) > outBufSize )
	{
		return -1;
	}

	length = 4 /* Command */ + 4 /* Frame Number */ + pPacket->payloadSize /* payload */ + 4 /* CRC */;

	//	Write Start Code
	*pBuf++ = (( UART_START_CODE >> 24) & 0xFF);
	*pBuf++ = (( UART_START_CODE >> 16) & 0xFF);
	*pBuf++ = (( UART_START_CODE >>  8) & 0xFF);
	*pBuf++ = (( UART_START_CODE      ) & 0xFF);

	//	Write Length
	*pBuf++ = (( length >> 24) & 0xFF);
	*pBuf++ = (( length >> 16) & 0xFF);
	*pBuf++ = (( length >>  8) & 0xFF);
	*pBuf++ = (( length      ) & 0xFF);

	//	Write Command
	pCrcStart = pBuf;
	*pBuf++ = (( pPacket->command >> 24) & 0xFF);
	*pBuf++ = (( pPacket->command >> 16) & 0xFF);
	*pBuf++ = (( pPacket->command >>  8) & 0xFF);
	*pBuf++ = (( pPacket->command      ) & 0xFF);

	//	FrameCount
	*pBuf++ = (( pPacket->frameNumber >> 24) & 0xFF);
	*pBuf++ = (( pPacket->frameNumber >> 16) & 0xFF);
	*pBuf++ = (( pPacket->frameNumber >>  8) & 0xFF);
	*pBuf++ = (( pPacket->frameNumber      ) & 0xFF);

	//	Write Payload
	memcpy( pBuf, pPacket->payload, pPacket->payloadSize );
	pBuf += pPacket->payloadSize;

	//	Calculate CRC
	crc32 = 0;
	crc32 = calc_crc32( crc32, (const uint8_t*)pCrcStart, length-4 );
	//printf("%7d = 0x%08x\n", pPacket->frameNumber, crc32);

	//	write CRC
	*pBuf++ = (( crc32 >> 24) & 0xFF);
	*pBuf++ = (( crc32 >> 16) & 0xFF);
	*pBuf++ = (( crc32 >>  8) & 0xFF);
	*pBuf   = (( crc32      ) & 0xFF);

	return length + 8;
}

//
//	pInBuf : Length + Command + Frame Number + Payload + CRC32
//
int32_t NX_ParseUartPacket( void *pInBuf, int32_t inBufSize, NX_UART_PACKET *pPacket )
{
	uint8_t *pBuf = (uint8_t *)pInBuf;

	//	length
	pPacket->length = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	if( pPacket->length+4 > inBufSize )
		return -1;
	pBuf += 4;

	//	Command
	pPacket->command = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	pBuf += 4;

	//	Frame Number
	pPacket->frameNumber = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	pBuf += 4;

	//	Payload
	pPacket->payload = pBuf;
	pPacket->payloadSize = pPacket->length - 12;
	pBuf += pPacket->length - 12;

	//	CRC32
	pPacket->crc32 = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	return 0;
}


//
//	pInBuf : Command + Frame Number + Payload + CRC
//	inBufSize : Size of command to crc32.
//
int32_t NX_CheckUartPacket( void *pInBuf, int32_t inBufSize )
{
	//	Calculate CRC
	uint8_t *pBuf = (uint8_t *)pInBuf;
	uint32_t crc32 = 0, readCrc32;

	//	Calculate CRC32 Command to Payload.
	crc32 = calc_crc32( crc32, (const uint8_t *)pBuf, inBufSize-4 );
	pBuf += inBufSize-4;

	//	Read CRC Information
	readCrc32 = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	if( crc32 != readCrc32 )
	{
		printf("read CRC32 = 0x%08x, Calc CRC32 = 0x%08x\n", readCrc32, crc32 );
		return -1;
	}

	return 0;
}




//
//	Debug Functions
//
void DumpUartPacket(NX_UART_PACKET *pPacket)
{
	printf("===================================================\n");
	printf("length      = %d\n", pPacket->length );
	printf("command     = 0x%08x\n", pPacket->command );
	printf("frameNumber = %d\n", pPacket->frameNumber );
	printf("crc32       = 0x%08x\n", pPacket->crc32 );
	printf("payload : \n");
	HexDump( pPacket->payload, pPacket->payloadSize );
	printf("===================================================\n");
}
