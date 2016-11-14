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

#ifndef __NX_IPCSERVER_H__
#define __NX_IPCSERVER_H__

#include <stdint.h>

#define	IPC_SERVER_FILE		"/data/local/tmp/ipc_server"
#define	MAX_PAYLOAD_SIZE	(128*1024)

int32_t NX_IPCServerStart();
void	NX_IPCServerStop();

#endif	// __NX_IPCSERVER_H__
