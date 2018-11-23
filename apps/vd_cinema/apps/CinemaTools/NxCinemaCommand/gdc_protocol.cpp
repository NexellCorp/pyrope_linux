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

#include "stdafx.h"
#include "gdc_protocol.h"

//------------------------------------------------------------------------------
//
//	Prameters
//		Input  : iKey, pPayload, iPayloadSize, iOutBufSize
//		Output : pOutBuf
//	Return
//		on success, the number of bytes is returned.
//		otherwise, -1 is returned.
//
int32_t GDC_MakePacket(
	uint32_t iKey, void *pPayload, int16_t iPayloadSize,
	void *pOutBuf, int32_t iOutBufSize )
{
	uint8_t *pBuf = (uint8_t*)pOutBuf;

	//	Check pointer & buffer size
	if( !pOutBuf || iPayloadSize + 6 > iOutBufSize )
	{
		return -1;
	}

	//	Write Key Code
	*pBuf++ = (( iKey >> 24) & 0xFF);
	*pBuf++ = (( iKey >> 16) & 0xFF);
	*pBuf++ = (( iKey >>  8) & 0xFF);
	*pBuf++ = (( iKey      ) & 0xFF);

	//	Write Length
	*pBuf++ = (( iPayloadSize >> 8) & 0xFF);
	*pBuf++ = (( iPayloadSize     ) & 0xFF);

	if( pPayload != NULL && iPayloadSize != 0 )
	{
		//	Write Payload
		memcpy( pBuf, pPayload, iPayloadSize );
	}

	return iPayloadSize+6;
}

//------------------------------------------------------------------------------
//
//	Parameters
//		Input : pInBuf, iInBufSize
//		Output : iKey, ppPayload, iPayloadSize
//	Return
//		On success, 0 is returned.
//		otherwise, negetive value is returned.
//			-1 : invalid argument.
//			-2 : invalid length or size error.
//
int32_t GDC_ParsePacket(
	void *pInBuf, int32_t iInBufSize,
	uint32_t *iKey, void **ppPayload, int16_t *iPayloadSize )
{
	uint8_t *pBuf = (uint8_t*)pInBuf;
	*ppPayload = NULL;

	//	Check pointer & buffer size
	if( !pInBuf || !iKey || !iPayloadSize )
	{
		return -1;
	}

	*iKey         = pBuf[0]<<24 | pBuf[1]<<16 | pBuf[2]<<8 | pBuf[3];	pBuf += 4;
	*iPayloadSize = pBuf[0]<< 8 | pBuf[1];	pBuf += 2;

	if( *iPayloadSize + 6 != iInBufSize )
	{
		return -2;
	}

	if( 0 != *iPayloadSize )
	{
		*ppPayload = pBuf;
	}

	return 0;
}