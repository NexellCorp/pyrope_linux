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

#ifndef __NX_IPCCLIENT_H__
#define __NX_IPCCLIENT_H__

#include <NX_IPCCommand.h>

int32_t NX_TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_IPCCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );

#endif	// __NX_IPCCLIENT_H__
