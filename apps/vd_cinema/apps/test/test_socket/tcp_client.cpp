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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <SockUtils.h>
#include <tms_protocol.h>
#include <NX_Utils.h>

#define	SVR_PORT_NO			6060
#define	MAX_PAYLOAD_SIZE	4096

#define PAYLOAD "I am a boy"

int32_t main( void )
{
	int32_t clntSock;
	char buff[MAX_PAYLOAD_SIZE+12];
	uint32_t loopCount = 10000;
	uint32_t cmd = 0;
	int32_t sendSize;
	char payload[1500];
	int32_t totalSendSize = 0;

	clntSock = TCP_Connect("127.0.0.1", 6060);
	if( -1 == clntSock)
	{
		printf( "Error : socket \n");
		exit( 1);
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd++, payload, sizeof(payload), buff, sizeof(buff) );
	write( clntSock, buff, sendSize );      // +1: NULL까지 포함해서 전송
	close( clntSock);

   return 0;
}
