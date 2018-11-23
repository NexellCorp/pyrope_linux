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

#ifndef __NX_TMSCLIENT_H__
#define __NX_TMSCLIENT_H__

#include <stdint.h>

#define MAX_PAYLOAD_SIZE			65535
#define MAX_BUF_SIZE				MAX_PAYLOAD_SIZE + 6

#define GDC_COMMAND( type, cmd )	(type | (cmd & 0x00FF))
#define CMD_TYPE_IMB				0xC0								//	1100 0000
#define CMD_IMB( cmd )				GDC_COMMAND( CMD_TYPE_IMB, cmd )
#define IMB_CMD_QUE					0x0C20								//	IMB QUE Command	// 0xC0 | 0x20

int32_t GDC_MakePacket( uint32_t iKey, void *pPayload, int16_t iPayloadSize, void *pOutBuf, int32_t iOutBufSize );
int32_t GDC_ParsePacket( void *pInBuf, int32_t iInBufSize, uint32_t *iKey, void **ppPayload, int16_t *iPayloadSize );

int32_t NX_TMSRecvPacket( int32_t fd, uint8_t *pBuf, int32_t iSize );
int32_t NX_TMSSendPacket( int32_t fd, uint8_t *pBuf, int32_t iSize );

int32_t NX_TMSSendCommand( const char *pIpAddr, int32_t iCmd, uint8_t *pBuf, int32_t *iSize );
int32_t NX_TMSSendCommand( const char *pIpAddr, void *pObj, int32_t (*cbProcess)(int32_t iSock, void *pObj) );

#endif	// __NX_TMSCLIENT_H__
