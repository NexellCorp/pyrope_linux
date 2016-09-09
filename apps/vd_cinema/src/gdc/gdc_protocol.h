//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Samsung Co. All Rights Reserved
//	Samsung Co. Proprietary & Confidential
//
//	SAMSUNG INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
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

#ifndef __GDC_PROTOCOL_H__
#define __GDC_PROTOCOL_H__

#include <stdint.h>

//
//	GDC <--> S.AP Communication Packet Format
//
//	Key( 4 bytes ) + Length( 2 bytes ) + Value ( n bytes )
//
//	Description
//		KeyValue:
//			a. GDC --> SEC : GDC_KEY_VALUE "TMSx"
//			b. SEC --> GDC : SEC_KEY_VALUE "SECx"
//		Length :
//			Command ( 2 bytes ) + Payload ( n Bytes )
//
#ifndef MAKE_KEY_4BYTE
#define MAKE_KEY_4BYTE(C0, C1, C2, C3)		\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )
#endif

#ifndef MAKE_LENGTH_2BYTE
#define MAKE_LENGTH_2BYTE(C0, C1)			\
		( ((uint16_t)(uint8_t)(C1)      ) |	\
		  ((uint16_t)(uint8_t)(C0) << 8 ) )
#endif

#define	GDC_KEY(N)		MAKE_KEY_4BYTE('G','D','C',N)
#define SEC_KEY(N)		MAKE_KEY_4BYTE('S','E','C',N)

//
//	APIs
//
int32_t GDC_MakePacket( uint32_t key, void *payload, int16_t payloadSize, void *pOutBuf, int32_t outBufSize );
int32_t GDC_ParsePacket( void *pInBuf, int32_t inBufSize, uint32_t *key, void **payload, int16_t *playloadSize );

void	GDC_DumpPacket( void *pData, int16_t dataSize );

#endif	// __GDC_PROTOCOL_H__
