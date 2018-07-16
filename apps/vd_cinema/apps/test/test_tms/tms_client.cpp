//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <NX_TMSClient.h>
#include <NX_IPCCommand.h>

#define MAX_PAYLOAD_SIZE	65533

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
static int32_t IMB_QueCommand( int32_t iMode )
{
	uint8_t buf[MAX_PAYLOAD_SIZE];
	int32_t iBufSize;

	buf[0]   = iMode;
	iBufSize = 1;

	return NX_TMSSendCommand( "127.0.0.1", GDC_COMMAND(CMD_TYPE_IMB, IMB_CMD_QUE), buf, &iBufSize );
}

//------------------------------------------------------------------------------
static const char* GetQueCommandString( int32_t iMode )
{
	if( 150 == iMode )		return "4K2D( 150 )";
	else if( 151 == iMode ) return "2K2D( 151 )";
	else if( 152 == iMode ) return "4K3D( 152 )";
	else if( 153 == iMode ) return "2K3D( 153 )";

	return "";
}


//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	register_signal();

	if( argc < 2 )
	{
		printf("Usage : %s [mode number]\n", argv[0]);
		return -1;
	}

	char *pResult = NULL;
	int32_t iValue = strtol(argv[1], &pResult, 10);

	printf(">> Send QueCommand: %s\n", (iValue < 150) ? argv[1] : GetQueCommandString(iValue));

	IMB_QueCommand( iValue );
	
	return 0;
}
