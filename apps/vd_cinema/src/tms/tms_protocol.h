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

#ifndef __TMS_PROTOCOL_H__
#define __TMS_PROTOCOL_H__

#include <NX_Utils.h>

#ifndef MAKE_KEY_VALUE
#define MAKE_KEY_VALUE(C0, C1, C2, C3)		\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )
#endif

#define	TMS_KEY_VALUE	MAKE_KEY_VALUE('G','D','C',' ')	// GDC --> SAM Key Value
#define SEC_KEY_VALUE	MAKE_KEY_VALUE('S','A','M',' ')	// SAM --> GDC Key Value

#define TMS_GET_LENGTH(C0, C1) \
		( ((uint32_t)(uint8_t)(C1)      ) |	\
		  ((uint32_t)(uint8_t)(C0) << 8 ) )
#define TMS_GET_COMMAND(C0, C1) \
		( ((uint32_t)(uint8_t)(C1)      ) |	\
		  ((uint32_t)(uint8_t)(C0) << 8 ) )

//
//	TMS <--> N.AP Coomunication Packet Format
//
//	Key       ( 4 Bytes )
//	Length    ( 4 Bytes )
//	Command   ( 4 Bytes )
//	Payload   ( n Bytes )
//
//	Description
//		KeyValue:
//			a. TMS --> SEC : TMS_KEY_VALUE "TMS "
//			b. SEC --> TMS : SEC_KEY_VALUE "SEC "
//		Length :
//			Command ( 4 bytes ) + Payload ( n Bytes )

//	APIs
int32_t TMS_MakePacket (
	uint32_t key, uint32_t cmd, void *payload, int32_t payloadSize,
	void *pOutBuf, int32_t outBufSize );

int32_t TMS_ParsePacket (
	void *pInBuf, int32_t inBufSize,
	uint32_t *key, uint32_t *cmd, void **payload, int32_t *playloadSize );

//	Debug Functions
void DumpTmsPacket(void *pData, int32_t dataSize, int32_t protocol);

#endif	// __TMS_PROTOCOL_H__
