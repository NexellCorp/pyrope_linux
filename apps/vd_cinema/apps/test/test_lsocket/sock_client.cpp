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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>

#include "SockUtils.h"

#define  BUFF_SIZE   1024
#define  FILE_SERVER "/data/local/tmp/ipc_server"

//------------------------------------------------------------------------------
int main( int /*argc*/, char **argv )
{
	int   client_socket;
	char   buff[BUFF_SIZE+5];
	int loopCount = 10000;

	while ( --loopCount > 0 ){
		client_socket = LS_Connect(FILE_SERVER);
		if( -1 == client_socket)
		{
			printf( "Error : socket (%s)\n", FILE_SERVER);
			exit( 1);
		}
		write( client_socket, argv[1], strlen( argv[1])+1);      // +1: NULL까지 포함해서 전송
		//read ( client_socket, buff, BUFF_SIZE);
		printf( "%4d : %s\n", loopCount, buff);
		close( client_socket);
		usleep(10000);
	}

   return 0;
}
