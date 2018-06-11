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

#include <stdio.h>	// printf
#include <string.h>	// memcpy

#include <ipc_protocol.h>
//
//	Prameters
//		Input  : key, cmd, payload, payloadSize, pOutBuf, outBufSize
//		Output : pOutBuf
//	Return
//		on success, the number of bytes is returned.
//		if pOutBuf is NULL or size is wrong 0 is returned.
//
uint32_t IPC_MakePacket (
	uint32_t key, uint32_t cmd, void *payload, uint32_t payloadSize,
	void *pOutBuf, uint32_t outBufSize )
{
	uint8_t *pBuf = (uint8_t *)pOutBuf;

	//	Check pointer & buffer size
	if( (NULL == pOutBuf) || (payloadSize+10 > outBufSize) )
	{
		return 0;
	}

	//	Write Key Code 4bytes
	*pBuf++ = (( key >> 24) & 0xFF);
	*pBuf++ = (( key >> 16) & 0xFF);
	*pBuf++ = (( key >>  8) & 0xFF);
	*pBuf++ = (( key      ) & 0xFF);

	//	Write Command 2bytes
	*pBuf++ = (( cmd >>  8) & 0xFF);
	*pBuf++ = (( cmd      ) & 0xFF);

	//	Write Length 4bytes
	*pBuf++ = (( payloadSize >> 24) & 0xFF);
	*pBuf++ = (( payloadSize >> 16) & 0xFF);
	*pBuf++ = (( payloadSize >>  8) & 0xFF);
	*pBuf++ = (( payloadSize      ) & 0xFF);

	if( payload != NULL && payloadSize != 0 )
	{
		//	Write Payload
		memcpy( pBuf, payload, payloadSize );
		pBuf += payloadSize;
	}

	return payloadSize+10;
}

//
//	Parameters
//		Input : pInBuf, inBufSize key, cmd, payload, payloadSize
//		Output : key, cmd, payload, payloadSize
//	Return
//		On success, 0 is returned.
//		otherwise, negetive value is returned.
//			-1 : invalid argument.
//			-2 : invalid length or size error.
//
int32_t IPC_ParsePacket (
	void *pInBuf, uint32_t inBufSize,
	uint32_t *key, uint32_t *cmd, void **payload, uint32_t *payloadSize )
{
	uint32_t len;
	uint8_t* pBuf = (uint8_t*)pInBuf;

	if( !key || !pInBuf )
	{
		return -1;
	}

	*key = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	pBuf += 4;

	*cmd = pBuf[0]<<8 | pBuf[1];
	pBuf += 2;

	len  = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	pBuf += 4;

	if ( (len+10) != inBufSize )
	{
		return -2;
	}

	*payloadSize = len;
	*payload = pBuf;

	return 0;
}

//
//	Debug Packet Functions
//
void DumpIpcPacket(void *pData, uint32_t dataSize, int32_t protocol)
{
	if( protocol )
	{
		void *payload;
		uint32_t key, cmd;
		uint32_t payloadSize, ret;
		ret = IPC_ParsePacket( pData, dataSize, &key, &cmd, &payload, &payloadSize );
		if( ret != 0 )
		{
			printf( "DumpIpcPacket : IPC_ParsePacket failed (%d)\n", ret );
		}

		printf("==================================================\n");
		printf("key = 0x%08x\n", key);
		printf("cmd = 0x%08x\n", cmd);
		printf("payloadSize = %d\n", payloadSize);
		printf("payload : \n");
		NX_HexDump( payload, payloadSize );
		printf("==================================================\n");
	}
	else
	{
		NX_HexDump( pData, dataSize );
	}
}
