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

#ifndef __IPC_PROTOCOL_H__
#define __IPC_PROTOCOL_H__

#include <stdint.h>

#define MAKE_KEY(C0, C1, C2, C3)	\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )

#define MAKE_COMMAND(C0, C1)	\
		( ((uint32_t)(uint8_t)(C1)      ) |	\
		  ((uint32_t)(uint8_t)(C0) << 8 ) )

#define MAKE_LENGTH(C0, C1)		\
		( ((uint16_t)(uint8_t)(C1)      ) |	\
		  ((uint16_t)(uint8_t)(C0) << 8 ) )

#define	KEY_NXP		MAKE_KEY('N','X','P',' ')		// NXP --> SAM Key Value
#define KEY_SEC		MAKE_KEY('S','A','M',' ')		// SAM --> NXP Key Value

//
//	NXP <--> N.AP Communication Packet Format
//
//	Key( 4 Bytes ) + Command( 2 Bytes ) + Length( 2 Bytes ) + Payload( n Bytes )
//
//	Description
//		Key		: 4Bytes
//			a. NXP --> SEC : TMS_KEY_VALUE "NXP "
//			b. SEC --> NXP : SEC_KEY_VALUE "SEC "
//		Command	: 2 Bytes
//		Length	: 2 Bytes
//		Payload : N Bytes

//	APIs
int32_t IPC_MakePacket (
	uint32_t iKey, uint32_t iCmd, void *pPayload, int32_t iPayloadSize,
	void *pOutBuf, int32_t iOutBufSize
);

int32_t IPC_ParsePacket (
	void *pInBuf, int32_t iInBufSize,
	uint32_t *iKey, uint32_t *iCmd, void **ppPayload, int32_t *iPlayloadSize
);

void IPC_DumpPacket( void *pData, int32_t iDataSize, int32_t bProtocol );

#endif	// __IPC_PROTOCOL_H__
