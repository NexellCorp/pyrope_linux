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
#include <stdio.h>

#define NX_DTAG	"[NX_DbgMsg]"
#include "NX_DbgMsg.h"

uint32_t gNxDebugLevel = NX_DBG_DEBUG;		// NX_DBG_WARN

//------------------------------------------------------------------------------
void NxChgDebugLevel( uint32_t level )
{
	NxDbgMsg( NX_DBG_VBS, "%s : Change debug level %d to %d.\n", __FUNCTION__, gNxDebugLevel, level );
	gNxDebugLevel = level;
}

uint32_t NxGetDebugLevel(void)
{
	return gNxDebugLevel;
}
