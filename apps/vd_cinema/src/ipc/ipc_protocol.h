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

#include <NX_Utils.h>

#ifndef MAKE_KEY_VALUE
#define MAKE_KEY_VALUE(C0, C1, C2, C3)		\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )
#endif

#define	NXP_KEY_VALUE	MAKE_KEY_VALUE('N','X','P',0x00)	// GDC --> SAM Key Value
#define SEC_KEY_VALUE	MAKE_KEY_VALUE('S','A','M',0x00)	// SAM --> GDC Key Value

#define IPC_GET_LENGTH(C0, C1, C2, C3)		\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )

#define IPC_GET_COMMAND(C0, C1) \
		( ((uint32_t)(uint8_t)(C1)      ) |	\
		  ((uint32_t)(uint8_t)(C0) << 8 ) )

//
//	NXP <--> N.AP Coomunication Packet Format
//
//	Key       ( 4 Bytes )
//	Command   ( 2 Bytes )
//	Length    ( 4 Bytes )
//	Payload   ( n Bytes )
//
//	Description
//		KeyValue:
//			a. NXP --> SEC : TMS_KEY_VALUE "NXP "
//			b. SEC --> NXP : SEC_KEY_VALUE "SEC "
//		Length :
//			Payload ( n Bytes )

//	APIs
uint32_t IPC_MakePacket (
	uint32_t key, uint32_t cmd, void *payload, uint32_t payloadSize,
	void *pOutBuf, uint32_t outBufSize );

int32_t IPC_ParsePacket (
	void *pInBuf, uint32_t inBufSize,
	uint32_t *key, uint32_t *cmd, void **payload, uint32_t *payloadSize );

//	Debug Functions
void DumpIpcPacket(void *pData, uint32_t dataSize, int32_t protocol);

#endif	// __IPC_PROTOCOL_H__
