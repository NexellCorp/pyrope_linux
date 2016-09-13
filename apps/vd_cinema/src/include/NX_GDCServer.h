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

#ifndef __NX_MARRIAGESERVER_H__
#define __NX_MARRIAGESERVER_H__

#include <stdint.h>

enum {
	EVENT_RECEIVE_CERTIFICATE	= 0x1001,
	EVENT_RECEIVE_PLANE_DATA	= 0x1002,
	EVENT_RECEIVE_MARRIAGE_OK	= 0x1003,

	EVENT_ACK_CERTIFICATE		= 0x2001,
	EVENT_ACK_SIGN_PLANE_TEXT	= 0x2002,
	EVENT_ACK_MARRIAGE_OK		= 0x2003,

	ERROR_MAKE_PACKET			= 0xF001,
	ERROR_SIGN_PLANE_TEXT		= 0xF002,
};

int32_t	NX_MarriageServerStart( int32_t iPort,  const char *pCertFile, const char *pPrivFile );
void	NX_MarriageServerStop();

void	NX_MarraigeEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam );

#endif	// __NX_MARRIAGESERVER_H__
