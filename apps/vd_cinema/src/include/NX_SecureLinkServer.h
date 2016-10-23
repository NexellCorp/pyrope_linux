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

#ifndef __NX_SECURELINKSERVER_H__
#define __NX_SECURELINKSERVER_H__

int32_t NX_SLinkServerStart();
void NX_SLinkServerStop();
int32_t NX_SLinkServerGotoSleep();
void NX_SLinkServerRegEventCB( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam );

int32_t NX_SapPowerOn( int32_t on );

#endif	// __NX_SECURELINKSERVER_H__
