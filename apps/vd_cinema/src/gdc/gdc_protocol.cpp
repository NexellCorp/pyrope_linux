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
#include <ctype.h>

#include "gdc_protocol.h"
#include <NX_Utils.h>

//------------------------------------------------------------------------------
//
//	Prameters
//		Input  : key, payload, payloadSize, outBufSize
//		Output : pOutBuf
//	Return
//		on success, the number of bytes is returned.
//		otherwise, -1 is returned.
//
int32_t GDC_MakePacket ( uint32_t key, void *payload, int16_t payloadSize, void *pOutBuf, int32_t outBufSize )
{
	uint8_t *pBuf = (uint8_t *)pOutBuf;

	//	Check pointer & buffer size
	if( !pOutBuf || payloadSize+6 >= outBufSize )
	{
		return -1;
	}

	//	Write Key Code
	*pBuf++ = (( key >> 24) & 0xFF);
	*pBuf++ = (( key >> 16) & 0xFF);
	*pBuf++ = (( key >>  8) & 0xFF);
	*pBuf++ = (( key      ) & 0xFF);

	//	Write Length
	*pBuf++ = (( payloadSize >> 8) & 0xFF);
	*pBuf++ = (( payloadSize     ) & 0xFF);

	if( payload != NULL && payloadSize != 0 )
	{
		//	Write Payload
		memcpy( pBuf, payload, payloadSize );
		pBuf += payloadSize;
	}

	return payloadSize+6;
}

//------------------------------------------------------------------------------
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
int32_t GDC_ParsePacket ( void *pInBuf, int32_t inBufSize, uint32_t *key, void **payload, int16_t *playloadSize )
{
	int32_t len;
	uint8_t *pBuf = (uint8_t *)pInBuf;
	if( !key || !pInBuf )
	{
		return -1;
	}

	*key = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];
	pBuf += 4;
	len  = pBuf[0]<<8  | pBuf[1];
	pBuf += 2;

	if ( (len+6) != inBufSize )
	{
		return -2;
	}

	*playloadSize = len;
	*payload = pBuf;

	return 0;
}

//------------------------------------------------------------------------------
void GDC_DumpPacket(void *pData, int16_t dataSize, int32_t protocol)
{
	if( protocol )
	{
		int32_t ret;
		void *payload;
		uint32_t key;
		int16_t payloadSize;
		ret = GDC_ParsePacket( pData, dataSize, &key, &payload, &payloadSize );
		if( ret != 0 )
		{
			printf( "DumpGdcPacket : GDC_ParsePacket failed (%d)\n", ret );
		}

		printf("==================================================\n");
		printf("key = 0x%08x\n", key);
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
