//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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

#include <stdint.h>

//
//	TMS <--> N.AP Communication Packet Format
//
//	Key( 4 bytes ) + Command( 2 bytes ) + Length( 4 bytes ) + Value ( n bytes )
//
//	Description
//		KeyValue:
//			a. TMS --> N.AP : GDC_KEY_VALUE "GDC "
//			b. N.AP --> GDC : SEC_KEY_VALUE "SAM "
//		Length :
//			Payload ( n Bytes )
//
#ifndef MAKE_LENGTH_4BYTE
#define MAKE_LENGTH_4BYTE(C0, C1, C2, C3)		\
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

#define	TMS_KEY		MAKE_LENGTH_4BYTE('G','D','C',0x00)
#define SEC_KEY		MAKE_LENGTH_4BYTE('S','A','M',0x00)

//
//	APIs
//
uint32_t TMS_MakePacket( uint32_t key, uint32_t cmd, void *payload, uint32_t payloadSize, void *pOutBuf, uint32_t outBufSize );

int32_t TMS_ParsePacket( void *pInBuf, uint32_t inBufSize, uint32_t *key, uint32_t* cmd, void **payload, uint32_t *payloadSize );

void	TMS_DumpPacket( void *pData, uint32_t dataSize , int32_t protocol);

#endif	// __TMS_PROTOCOL_H__
