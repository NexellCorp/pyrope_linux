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

#include "ipc_protocol.h"
#include <NX_Utils.h>

//------------------------------------------------------------------------------
//
//	Prameters
//		Input  : iKey, iCmd, pPayload, iPayloadSize, iOutBufSize
//		Output : pOutBuf
//	Return
//		on success, the number of bytes is returned.
//		otherwise, -1 is returned.
//
int32_t IPC_MakePacket (
	uint32_t iKey, uint32_t iCmd, void *pPayload, int32_t iPayloadSize,
	void *pOutBuf, int32_t iOutBufSize )
{
	uint8_t *pBuf = (uint8_t*)pOutBuf;

	//	Check pointer & buffer size
	if( !pOutBuf || iPayloadSize + 8 > iOutBufSize )
	{
		return -1;
	}

	//	Write Key Code
	*pBuf++ = (( iKey >> 24) & 0xFF);
	*pBuf++ = (( iKey >> 16) & 0xFF);
	*pBuf++ = (( iKey >>  8) & 0xFF);
	*pBuf++ = (( iKey      ) & 0xFF);

	//	Write Command
	*pBuf++ = (( iCmd >>  8) & 0xFF);
	*pBuf++ = (( iCmd      ) & 0xFF);

	//	Write Length
	*pBuf++ = (( iPayloadSize >>  8) & 0xFF);
	*pBuf++ = (( iPayloadSize      ) & 0xFF);

	//	Write Payload
	if( pPayload != NULL && 0 < iPayloadSize )
	{
		memcpy( pBuf, pPayload, iPayloadSize );
	}

	return iPayloadSize + 8;
}

//------------------------------------------------------------------------------
//
//	Parameters
//		Input : pInBuf, iInBufSize
//		Output : iKey, iCmd, ppPayload, iPayloadSize
//	Return
//		On success, 0 is returned.
//		otherwise, negetive value is returned.
//			-1 : invalid argument.
//			-2 : invalid length or size error.
//
int32_t IPC_ParsePacket (
	void *pInBuf, int32_t iInBufSize,
	uint32_t *iKey, uint32_t *iCmd, void **ppPayload, int32_t *iPayloadSize )
{
	uint8_t *pBuf = (uint8_t *)pInBuf;
	*ppPayload = NULL;

	//	Check pointer & buffer size
	if( !pInBuf || !iKey || !iCmd || !iPayloadSize )
	{
		return -1;
	}

	*iKey         = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];	pBuf += 4;
	*iCmd         = pBuf[0]<< 8 | pBuf[1];	pBuf += 2;
	*iPayloadSize = pBuf[0]<< 8 | pBuf[1];	pBuf += 2;

	if( *iPayloadSize + 8 != iInBufSize )
	{
		return -2;
	}

	if( 0 != *iPayloadSize )
	{
		*ppPayload = pBuf;
	}

	return 0;
}

//------------------------------------------------------------------------------
//
//	Dump Packet
//
void IPC_DumpPacket( void *pData, int32_t iDataSize )
{
	int32_t iRet;
	void *pPayload;
	uint32_t iKey;
	uint32_t iCmd;
	int32_t iPayloadSize;

	iRet = IPC_ParsePacket( pData, iDataSize, &iKey, &iCmd, &pPayload, &iPayloadSize );
	if( 0 > iRet )
	{
		printf( "Fail, IPC_DumpPacket(). ( %d )\n", iRet );
		return ;
	}

	printf("==================================================\n");
	printf("iKey = 0x%08x\n", iKey);
	printf("iCmd = 0x%04x\n", iCmd);
	printf("iPayloadSize = %d\n", iPayloadSize);
	printf("pPayload : \n");
	NX_HexDump( pPayload, iPayloadSize );
	printf("==================================================\n");
}
