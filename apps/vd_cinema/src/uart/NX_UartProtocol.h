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

#ifndef __NX_UartProtocol_h__
#define __NX_UartProtocol_h__

#define UART_START_CODE		0xCafeFace

#define CMD_BOOT_DONE		0x00000001
#define CMD_ALIVE			0x00000002
#define CMD_MARRIAGE		0x00000101
#define CMD_DIVORCE			0x00000102

struct _NX_UART_PACKET{
	uint32_t startCode;
	int32_t length;
	uint32_t command;
	uint32_t frameNumber;
	uint32_t crc32;
	void * payload;
	int32_t payloadSize;
};

typedef struct _NX_UART_PACKET NX_UART_PACKET;

//	APIs
void NX_InitPacket( NX_UART_PACKET *pPacket );
int32_t NX_GetSendBufferSize( NX_UART_PACKET *pPacket );
int32_t NX_MakeUartPacket( NX_UART_PACKET *pPacket, void *pOutBuf, int32_t outBufSize );
int32_t NX_ParseUartPacket( void *pInBuf, int32_t inBufSize, NX_UART_PACKET *pPacket );
int32_t NX_CheckUartPacket( void *pInBuf, int32_t inBufSize );


//	Debug Functions
void DumpUartPacket(NX_UART_PACKET *pPacket);

#endif	// __NX_UartProtocol_h__
