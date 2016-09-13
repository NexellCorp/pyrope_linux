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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <NX_GDCServer.h>
#include <NX_GDCClient.h>

#include <NX_Utils.h>

#define SERVER_IP_ADDR		"127.0.0.1"

#define SERVER_PRIV_FILE	"./ruby/leaf.key"
#define SERVER_CERT_FILE	"./ruby/leaf.signed.pem"

#define CLIENT_PRIV_FILE	"./client/private_key.pem"
#define CLIENT_CERT_FILE	"./client/certificate.pem"

#define TCP_PORT			43684

//------------------------------------------------------------------------------
static void signal_handler( int32_t signal )
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(signal), signal);

	switch( signal )
	{
		case SIGINT :
			printf("SIGINT..\n"); 	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	NX_MarriageServerStop();
	exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

//------------------------------------------------------------------------------
static int32_t cbEventCallback( void *pObj, int32_t iEventCode, void *pData, int32_t iSize )
{
	switch( iEventCode )
	{
	case EVENT_RECEIVE_CERTIFICATE :
		printf("======================================================================\n");
		printf(">> Receive Ceriticate.\n");
		printf("======================================================================\n");
		printf("%s", (char*)pData);
		printf("======================================================================\n");
		break;
	case EVENT_RECEIVE_PLANE_DATA :
		printf("======================================================================\n");
		printf(">> Receive Plane Data.\n");
		printf("======================================================================\n");
		HexDump( pData, iSize );
		printf("======================================================================\n");
		break;
	case EVENT_RECEIVE_MARRIAGE_OK :
		printf("======================================================================\n");
		printf(">> Receive Marriage OK.\n");
		printf("======================================================================\n");
		break;
	case EVENT_ACK_CERTIFICATE :
		printf("======================================================================\n");
		printf(">> Transfer Ceriticate.\n");
		printf("======================================================================\n");
		HexDump( pData, iSize );
		printf("======================================================================\n");
		break;
	case EVENT_ACK_SIGN_PLANE_TEXT :
		printf("======================================================================\n");
		printf(">> Transfer Sign Data.\n");
		printf("======================================================================\n");
		HexDump( pData, iSize );
		printf("======================================================================\n");
		break;
	case EVENT_ACK_MARRIAGE_OK :
		printf("======================================================================\n");
		printf(">> Transfer Marriage OK.\n");
		printf("======================================================================\n");
		HexDump( pData, iSize );
		printf("======================================================================\n");
		break;
	case ERROR_MAKE_PACKET :
		printf("Error, Make Packet.\n");
		break;
	case ERROR_SIGN_PLANE_TEXT :
		printf("Error, Sign Data.\n");
		break;
	default:
		break;
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t server_main( void )
{
	NX_MarraigeEventCallback( cbEventCallback, NULL );
	NX_MarriageServerStart( TCP_PORT, SERVER_CERT_FILE, SERVER_PRIV_FILE );

	while(1)
	{
		usleep(100000);
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t client_main( void )
{
	NX_MarriageVerify( SERVER_IP_ADDR, TCP_PORT, CLIENT_CERT_FILE, CLIENT_PRIV_FILE, (uint8_t*)"TEST" );
	return 0;
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;
	register_signal();

    if( argc != 2 )
    {
        return 0;
    }

	while( -1 != (opt=getopt(argc, argv, "sc")))
	{
		switch( opt )
		{
		case 's' :
			server_main();
			break;
		case 'c' :
			client_main();
			break;
		default :
			break;
		}
	}

	return 0;
}



