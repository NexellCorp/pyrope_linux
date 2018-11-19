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
#include <ctype.h>

#include <NX_TMSClient.h>
#include <gdc_protocol.h>
#include <NX_CinemaCommand.h>
#include <NX_Utils.h>

#define NX_DTAG	"[nx_tms_client]"
#include <NX_DbgMsg.h>

#define MAX_PAYLOAD_SIZE	65535
#define MAX_BUF_SIZE		MAX_PAYLOAD_SIZE + 6	// key(4bytes) + size(2bytes) + payload(65535bytes)

//------------------------------------------------------------------------------
enum {
	NX_TMS_UPLOAD		= 200,
	NX_TMS_DOWNLOAD		= 201,
};

typedef struct NX_QUE_INFO {
	char*	pAddr;
	char*	pQue;
	char*	pMode;
	char*	pColor;
	char* 	pFile;
} NX_QUE_INFO;

static const char* gstQueCommandString[] = {
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//   0 -  15
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",			//  16 -  29
	"All Delete Configuration",										//  30
	"",																//  31
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//  32 -  47
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//  48 -  63
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//  64 -  79
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//  79 -  95
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	//  96 - 111
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 112 - 127
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 128 - 143
	"",	"",	"",	"",	"",	"",											// 144 - 149
	"P2.5_4K_2D",													// 150
	"P2.5_2K_2D",													// 151
	"P2.5_4K_3D",													// 152
	"P2.5_2K_3D",													// 153
	"P3.3_4K_2D",													// 154
	"P3.3_4K_3D",													// 155
	"P3.3_2K_2D",													// 156
	"P3.3_2K_3D",													// 157
	"",	"",															// 158 - 159
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 160 - 175
	"",	"",	"",	"",													// 176 - 179
	"Screen On",													// 180
	"Screen Off",													// 181
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",							// 182 - 191
	"",	"",	"",	"",	"",	"",	"",	"",									// 192 - 199
	"Upload Behavior Data",											// 200
	"Download Behavior Data",										// 201
	"",	"",	"",	"",	"",	"",											// 202 - 207
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 208 - 223
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 224 - 239
	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	"",	// 240 - 255
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
	static char szString[128];
	memset( szString, 0x00, sizeof(szString) );

	snprintf( szString, sizeof(szString), "%d%s%s%s",
		iQue,
		!strlen(gstQueCommandString[iQue]) ? "" : " ( ",
		!strlen(gstQueCommandString[iQue]) ? "" : gstQueCommandString[iQue],
		!strlen(gstQueCommandString[iQue]) ? "" : " ) "
	);
	return szString;
}

//------------------------------------------------------------------------------
static int32_t cbUpload( int32_t iSock, void* pObj )
{
	uint8_t sendBuf[MAX_BUF_SIZE];
	uint8_t recvBuf[MAX_BUF_SIZE];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	NX_QUE_INFO *pInfo = (NX_QUE_INFO*)pObj;

	int32_t iRet = 0;
	if( (pInfo->pFile == NULL) || (0 > (iRet = access(pInfo->pFile, F_OK))) )
	{
		return -1;
	}

	{
		buf[0]   = strtol(pInfo->pQue, NULL, 10);
		iBufSize = 1;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			printf( "Fail, GDC_MakePacket().\n");
			return -1;
		}

		write( iSock, sendBuf, iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );

		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			printf( "Fail, GDC_ParsePacket().\n" );
			return -1;
		}
	}

	{
		FILE *hFile = fopen( pInfo->pFile, "rb" );
		if( NULL == hFile )
			return -1;

		fseek( hFile, 0, SEEK_END );
		iBufSize = ftell( hFile );
		fseek( hFile, 0, SEEK_SET );

		int32_t iReadSize = fread( buf, 1, iBufSize, hFile );
		if( iReadSize != iBufSize )
		{
			fclose( hFile );
			return -1;
		}
		fclose( hFile );

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			printf( "Fail, GDC_MakePacket().\n");
			return -1;
		}

		write( iSock, sendBuf, iSendSize );
		printf(">> Send %d bytes\n", iBufSize);
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );

		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			printf( "Fail, GDC_ParsePacket().\n" );
			return -1;
		}

		/*
		*/
		uint8_t* pRecv = (uint8_t*)pPayload;
		int32_t iResult = ((pRecv[0] << 8) & 0xFF00) + ((pRecv[1] << 0) & 0x00FF);

		printf(">> Received Remote %d bytes\n", iResult);
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbDonwload( int32_t iSock, void* pObj )
{
	uint8_t sendBuf[MAX_BUF_SIZE];
	uint8_t recvBuf[MAX_BUF_SIZE];
	uint8_t buf[MAX_PAYLOAD_SIZE];

	int32_t iSendSize, iRecvSize;
	int32_t iBufSize;

	uint32_t iKey;
	void *pPayload;
	int16_t iPayloadSize;

	NX_QUE_INFO *pInfo = (NX_QUE_INFO*)pObj;

	char szPrefix[128] = { 0x00, };

	if( (pInfo->pColor == NULL) || (
		( toupper( pInfo->pColor[0] ) != (int)'R' ) &&
		( toupper( pInfo->pColor[0] ) != (int)'G' ) &&
		( toupper( pInfo->pColor[0] ) != (int)'B' ) ) )
	{
		return -1;
	}

	{
		buf[0]   = strtol(pInfo->pQue, NULL, 10);
		buf[1]   = strtol(pInfo->pMode, NULL, 10);
		iBufSize = 2;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			printf( "Fail, GDC_MakePacket().\n");
			close( iSock );
			return -1;
		}

		write( iSock, sendBuf, iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );

		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			printf( "Fail, GDC_ParsePacket().\n" );
			close( iSock );
			return -1;
		}

		if( iPayloadSize == 0 )
			return -1;

		memcpy( szPrefix, pPayload, iPayloadSize );
	}

	{
		buf[0]   = strtol(pInfo->pQue, NULL, 10);
		buf[1]   = toupper( pInfo->pColor[0] );
		iBufSize = 2;

		iSendSize = GDC_MakePacket( KEY_GDC(CMD_IMB(IMB_CMD_QUE)), buf, iBufSize, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			printf( "Fail, GDC_MakePacket().\n");
			close( iSock );
			return -1;
		}

		write( iSock, sendBuf, iSendSize );
	}

	{
		iRecvSize = NX_TMSRecvPacket( iSock, recvBuf, sizeof(recvBuf) );

		if( 0 != GDC_ParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
		{
			printf( "Fail, GDC_ParsePacket().\n" );
			close( iSock );
			return -1;
		}

		char szFile[256] = { 0x00, };
		snprintf( szFile, sizeof(szFile), "./Mode%02d_%s_%c.txt",
			(int32_t)strtol(pInfo->pMode, NULL, 10),
			szPrefix,
			toupper( pInfo->pColor[0] )
		);

		FILE* hFile;
		hFile = fopen( szFile, "wb" );
		if( NULL == hFile )
			return -1;

		int32_t iWriteSize = fwrite( pPayload, 1, iPayloadSize, hFile );
		if( iWriteSize != iPayloadSize )
		{
			fclose( hFile );
			return -1;
		}

		fclose( hFile );
	}

	return 0;
}

//------------------------------------------------------------------------------
static void Usage( char* pAppName )
{
	printf(
		"usage: %s [options]\n"
		"  -q [que number]    [M] : que number\n"
		"  -i [ip address]    [O] : ip address (default: 127.0.0.1)\n"
		"  -f [file name]     [O] : behavior file to upload in que #200\n"
		"  -m [mode]          [O] : behavior mode to download in que #201\n"
		"  -c [color type]    [O] : behavior color type to download in que #201\n",
		pAppName
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t iRet;
	int32_t iOpt;

	NX_QUE_INFO info, *pInfo = &info;
	memset( &info, 0x00, sizeof(NX_QUE_INFO) );

	register_signal();

	while (-1 != (iOpt = getopt(argc, argv, "hq:i:f:m:c:")))
	{
		switch( iOpt )
		{
		case 'h':	Usage(argv[0]);						return 0;
		case 'q':	pInfo->pQue   = strdup(optarg);		break;
		case 'i':	pInfo->pAddr  = strdup(optarg);		break;
		case 'f':	pInfo->pFile  = strdup(optarg);		break;
		case 'm':	pInfo->pMode  = strdup(optarg);		break;
		case 'c':	pInfo->pColor = strdup(optarg);		break;
		default:	break;
		}
	}

	if( NULL == pInfo->pQue )
	{
		Usage(argv[0]);
		goto TERMINATE;
	}

	if( NULL == pInfo->pAddr )
	{
		pInfo->pAddr = strdup("127.0.0.1");
	}

	{
		uint8_t szVersion[1024] = { 0x00, };
		uint8_t buf[MAX_PAYLOAD_SIZE];
		int32_t iBufSize;

		iRet = NX_TMSSendCommand( pInfo->pAddr, CMD_PLATFORM(PLATFORM_CMD_TMS_CLIENT_VERSION), buf, &iBufSize );
		if( 0 > iRet )
		{
			printf("Fail, NX_TMSSendCommand().\n");
			goto TERMINATE;
		}

		memcpy( szVersion, buf, iBufSize );
		printf("* TMS Client Version: %s\n", szVersion);
	}

	{
		int32_t iQue = strtol(pInfo->pQue, NULL, 10);

		printf(">> Send TMS Command: %s\n", GetQueCommandString(iQue) );

		if( iQue == NX_TMS_UPLOAD )
		{
			iRet = NX_TMSSendCommand( pInfo->pAddr, (void*)pInfo, &cbUpload );
			if( 0 > iRet )
			{
				printf("Fail, NX_TMSSendCommand().\n");
				goto TERMINATE;
			}
		}
		else if( iQue == NX_TMS_DOWNLOAD )
		{
			iRet = NX_TMSSendCommand( pInfo->pAddr, (void*)pInfo, &cbDonwload );
			if( 0 > iRet )
			{
				printf("Fail, NX_TMSSendCommand().\n");
				goto TERMINATE;
			}
		}
		else
		{
			uint8_t buf[MAX_PAYLOAD_SIZE];
			int32_t iBufSize;

			buf[0] = iQue;
			iBufSize = 1;

			iRet = NX_TMSSendCommand( pInfo->pAddr, CMD_IMB(IMB_CMD_QUE), buf, &iBufSize );
			if( 0 > iRet )
			{
				printf("Fail, NX_TMSSendCommand().\n");
				goto TERMINATE;
			}
		}
	}

TERMINATE:
	if( pInfo->pAddr )	free( pInfo->pAddr );
	if( pInfo->pMode )	free( pInfo->pMode );
	if( pInfo->pColor )	free( pInfo->pColor );
	if( pInfo->pFile )	free( pInfo->pFile );

	return 0;
}
