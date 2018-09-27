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

#ifndef __GDC_PROTOCOL_H__
#define __GDC_PROTOCOL_H__

#include <stdint.h>

#define MAKE_KEY(C0, C1, C2, C3)	\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )

#define MAKE_LENGTH(C0, C1)	\
		( ((uint16_t)(uint8_t)(C1)      ) |	\
		  ((uint16_t)(uint8_t)(C0) << 8 ) )

#define	KEY_GDC(N)	MAKE_KEY('G','D','C',N)
#define KEY_SEC(N)	MAKE_KEY('S','A','M',N)

//
//	GDC <--> S.AP Communication Packet Format
//
//	Key( 4 bytes ) + Length( 2 bytes ) + Value ( n bytes )
//
//	Description
//		key		: 4 Bytes
//			a. GDC --> SEC : GDC_KEY_VALUE "TMSx"
//			b. SEC --> GDC : SEC_KEY_VALUE "SECx"
//		Length	: 2 Bytes
//		Payload	: N Bytes
//

//	APIs
int32_t GDC_MakePacket(
	uint32_t iKey, void *pPayload, int16_t iPayloadSize,
	void *pOutBuf, int32_t iOutBufSize
);

int32_t GDC_ParsePacket(
	void *pInBuf, int32_t iInBufSize,
	uint32_t *iKey, void **ppPayload, int16_t *iPayloadSize
);

void GDC_DumpPacket( void *pData, int16_t iDataSize, int32_t bProtocol );

#endif	// __GDC_PROTOCOL_H__
