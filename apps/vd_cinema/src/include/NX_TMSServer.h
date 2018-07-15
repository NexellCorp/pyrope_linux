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

#ifndef __NX_TMSSERVER_H__
#define __NX_TMSSERVER_H__

#include <stdint.h>

#define TMS_SERVER_PORT		7434

int32_t NX_TMSServerStart();
void	NX_TMSServerStop();
void	NX_TMSServerProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj = NULL );

#endif	// __NX_TMSSERVER_H__
