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

#include <tms_protocol.h>
//
//	Prameters
//		Input  : key, cmd, payload, payloadSize, outBufSize
//		Output : pOutBuf
//	Return
//		on success, the number of bytes is returned.
//		otherwise, -1 is returned.
//
int32_t TMS_MakePacket (
	uint32_t key, uint32_t cmd, void *payload, int32_t payloadSize,
	void *pOutBuf, int32_t outBufSize )
{
	uint8_t *pBuf = (uint8_t *)pOutBuf;
	uint32_t length, crc32;
	uint8_t *pCrcStart;

	//	Check pointer & buffer size
	if( !pOutBuf || payloadSize+8 >= outBufSize )
	{
		return -1;
	}

	length = 4 /* Command */ + payloadSize /* payload */;

	//	Write Key Code
	*pBuf++ = (( key >> 24) & 0xFF);
	*pBuf++ = (( key >> 16) & 0xFF);
	*pBuf++ = (( key >>  8) & 0xFF);
	*pBuf++ = (( key      ) & 0xFF);

	//	Write Length
	*pBuf++ = (( length >> 24) & 0xFF);
	*pBuf++ = (( length >> 16) & 0xFF);
	*pBuf++ = (( length >>  8) & 0xFF);
	*pBuf++ = (( length      ) & 0xFF);

	//	Write Command
	pCrcStart = pBuf;
	*pBuf++ = (( cmd >> 24) & 0xFF);
	*pBuf++ = (( cmd >> 16) & 0xFF);
	*pBuf++ = (( cmd >>  8) & 0xFF);
	*pBuf++ = (( cmd      ) & 0xFF);

	//	Write Payload
	if( payloadSize > 0 )
	{
		memcpy( pBuf, payload, payloadSize );
		pBuf += payloadSize;
	}

	return length + 8;
}

//
//	Parameters
//		Input : pInBuf, inBufSize
//		Output : inBufSize
//	Return
//		On success, 0 is returned.
//		otherwise, negetive value is returned.
//			-1 : invalid argument.
//			-2 : invalid length or size error.
//
int32_t TMS_ParsePacket (
	void *pInBuf, int32_t inBufSize,
	uint32_t *key, uint32_t *cmd, void **payload, int32_t *playloadSize )
{
	int32_t len;
	uint8_t *pBuf = (uint8_t *)pInBuf;
	if( !key || !cmd || !pInBuf )
	{
		return -1;	//	Invalid argument
	}

	*key = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];	pBuf += 4;
	len  = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];	pBuf += 4;
	*cmd = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];	pBuf += 4;

	if ( (len+8) != inBufSize )
	{
		return -2;
	}

	*playloadSize = len - 4;
	*payload = pBuf;

	return 0;
}

//
//	Debug Packet Functions
//
void DumpTmsPacket(void *pData, int32_t dataSize, int32_t protocol)
{
	if( protocol )
	{
		void *payload;
		uint32_t key, cmd;
		int32_t payloadSize, ret;
		ret = TMS_ParsePacket( pData, dataSize, &key, &cmd, &payload, &payloadSize );
		if( ret != 0 )
		{
			printf( "DumpTmsPacket : TMS_ParsePacket failed (%d)\n", ret );
		}

		printf("==================================================\n");
		printf("key = 0x%08x\n", key);
		printf("cmd = 0x%08x\n", cmd);
		printf("payloadSize = %d\n", payloadSize);
		printf("payload : \n");
		HexDump( payload, payloadSize );
		printf("==================================================\n");
	}
	else
	{
		HexDump( pData, dataSize );
	}
}
