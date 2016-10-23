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
#include <errno.h>

#include "SockUtils.h"

#define  BUFF_SIZE   1024
#define  FILE_SERVER "/data/local/tmp/local_test_server"

//------------------------------------------------------------------------------
int32_t main( void )
{
	int32_t server_socket;
	int32_t client_socket;
	int32_t client_addr_size;
	int32_t read_len;

	struct sockaddr_un client_addr;

	char buff_rcv[BUFF_SIZE+5];
	char buff_snd[BUFF_SIZE+5];

	server_socket  = LS_Open( FILE_SERVER );
	if( server_socket < 0 )
	{
		printf( "Error : server socket \n");
		exit( 1);
	}

	while( 1 )
	{
		if( -1 == listen(server_socket, 5) )
		{
			printf( "Error : listen (err = %d)\n", errno);
			exit( 1);
		}

		client_addr_size = sizeof( client_addr );
		client_socket    = accept( server_socket, (struct sockaddr*)&client_addr, &client_addr_size );

		if ( -1 == client_socket )
		{
			printf( "Error : accept (err = %d)\n", errno);
			exit( 1);
		}

		read_len = read ( client_socket, buff_rcv, BUFF_SIZE );
		printf( "receive: %dn", read_len);

		sprintf( buff_snd, "%d : %s", strlen( buff_rcv), buff_rcv );
		write( client_socket, buff_snd, strlen( buff_snd)+1 );
		close( client_socket );
	}
	return 0;
}
