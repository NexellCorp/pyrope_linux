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

#include <NX_TMSClient.h>
#include <NX_CinemaCommand.h>

#define MAX_PAYLOAD_SIZE	65533

//------------------------------------------------------------------------------
enum {
	NX_TMS_QUE			= 150,
	NX_TMS_P25_4K_2D	= 150,
	NX_TMS_P25_2K_2D,
	NX_TMS_P25_4K_3D,
	NX_TMS_P25_2K_3D,
	NX_TMS_P33_4K_2D,
	NX_TMS_P33_4K_3D,
	NX_TMS_P33_2K_2D,
	NX_TMS_P33_2K_3D,
	NX_TMS_QUE_MAX,
};

static const char* gstQueCommandString[] = {
	"P2.5_4K_2D (150)",
	"P2.5_2K_2D (151)",
	"P2.5_4K_3D (152)",
	"P2.5_2K_3D (153)",
	"P3.3_4K_2D (154)",
	"P3.3_4K_3D (155)",
	"P3.3_2K_2D (156)",
	"P3.3_2K_3D (157)",
};

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
static const char* GetQueCommandString( int32_t iQue )
{
	if( iQue < NX_TMS_QUE || iQue >= NX_TMS_QUE_MAX )
		return "";

	return gstQueCommandString[iQue-NX_TMS_QUE];
}

//------------------------------------------------------------------------------
static void Usage( char* pAppName )
{
	printf(
		"usage: %s [options]\n"
		"  -m [mode number]   [M] : mode number\n"
		"  -i [ip address]    [O] : ip address (default: 127.0.0.1)\n",
		pAppName
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t iRet;
	int32_t iOpt;
	char *pAddr = NULL, *pMode = NULL;

	register_signal();

	while (-1 != (iOpt = getopt(argc, argv, "hi:m:")))
	{
		switch( iOpt )
		{
		case 'i':	pAddr = strdup(optarg);		break;
		case 'm':	pMode = strdup(optarg);		break;
		default:	break;
		}
	}

	if( NULL == pAddr )
	{
		pAddr = strdup("127.0.0.1");
	}

	if( NULL == pMode )
	{
		Usage(argv[0]);
		goto TERMINATE;
	}

	{
		uint8_t szVersion[1024] = { 0x00, };
		uint8_t buf[MAX_PAYLOAD_SIZE];
		int32_t iBufSize;

		iRet = NX_TMSSendCommand( pAddr, GDC_COMMAND(CMD_TYPE_PLATFORM, PLATFORM_CMD_TMS_CLIENT_VERSION), buf, &iBufSize );
		if( 0 > iRet )
		{
			printf("Fail, NX_TMSSendCommand().\n");
			goto TERMINATE;
		}

		memcpy( szVersion, buf, iBufSize );
		printf("* TMS Client Version: %s\n", szVersion);
	}

	{
		int32_t iMode = strtol(pMode, NULL, 10);
		uint8_t buf[MAX_PAYLOAD_SIZE];
		int32_t iBufSize;

		buf[0] = iMode;
		iBufSize = 1;

		printf(">> Send TMS Command: %s\n", (iMode < 150) ? pMode : GetQueCommandString(iMode));

		iRet = NX_TMSSendCommand( pAddr, GDC_COMMAND(CMD_TYPE_IMB, IMB_CMD_QUE), buf, &iBufSize );
		if( 0 > iRet )
		{
			printf("Fail, NX_TMSSendCommand().\n");
			goto TERMINATE;
		}
	}

TERMINATE:
	if( pAddr ) free( pAddr );
	if( pMode ) free( pMode );

	return 0;
}
