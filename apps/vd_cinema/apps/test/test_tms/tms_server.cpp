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
#include <unistd.h>

#include <NX_TMSServer.h>

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

	NX_TMSServerStop();
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
static void ProcessCallback( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize, void *pObj )
{
	(void*)pObj;
	*pOutBuf  = 0x01;
	*iOutSize = 1;
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	register_signal();
	NX_TMSServerProcessCallback( &ProcessCallback );
	NX_TMSServerStart();

	while( 1 )
	{
		usleep(1000000);
	}

	NX_TMSServerStop();
	return 0;
}
