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
#include <stdlib.h>
#include <unistd.h>

#include <poll.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <net/if.h>

#include <CNX_I2C.h>
#include <CNX_EEPRom.h>
#include <CNX_EEPRomData.h>
#include <NX_Utils.h>
#include <NX_SecureLinkServer.h>

#include "version.h"

//------------------------------------------------------------------------------
#define MAX_TIMEOUT				1000

#define TCON_REG_VALIDATE		0x0170
#define PFPGA_REG_VALIDATE		0x01C2

//------------------------------------------------------------------------------
enum {
	NX_DIAG_TCON_LEFT = 0,
	NX_DIAG_TCON_RIGHT,
	NX_DIAG_PFPGA,
	NX_DIAG_EEPROM,
	NX_DIAG_MARRIAGE_TAMPER,
	NX_DIAG_DOOR_TAMPER,
	NX_DIAG_NETWORK,
	NX_DIAG_MAX,
};

typedef struct NX_DIAG_INFO {
	int32_t iCommand;
	char	szIpAddr[16];
	int32_t iType;
} NX_DIAG_INFO;

//------------------------------------------------------------------------------
static int32_t Accept( int32_t iSock, struct sockaddr *pAddr, socklen_t *pAddrLen )
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = iSock;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
	if( 0 < iRet )
	{
		return accept( iSock, pAddr, pAddrLen );
	}

	return -1;
}

//------------------------------------------------------------------------------
static int32_t Write( int32_t iSock, char *pBuf, int32_t iSize )
{
	return write( iSock, pBuf, iSize );
}

//------------------------------------------------------------------------------
static int32_t Read( int32_t iSock, char *pBuf, int32_t iSize )
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = iSock;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
	if( 0 < iRet )
	{
		return read( iSock, pBuf, iSize );
	}

	return -1;
}

//------------------------------------------------------------------------------
static int32_t SendRemote( const char *pSockName, const char *pMsg )
{
	int32_t iSock, iLen;
	struct sockaddr_un stAddr;

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		return -1;
	}

	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, pSockName );

	iLen = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(iSock, (struct sockaddr *) &stAddr, iLen))
	{
		close( iSock );
		return -1;
	}

	if( 0 > Write(iSock, (char*)pMsg, strlen(pMsg)) )
	{
		close( iSock );
		return -1;
	}

	close( iSock );
	return 0;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsTconLeft( __attribute__((unused)) void *pObj )
{
	int32_t iPort = 0;
	uint8_t iSlave = 0x09;
	int32_t iWriteData, iReadData = 0x0000;

	CNX_I2C i2c( iPort );
	if( 0 > i2c.Open() )
		goto ERROR;

	iWriteData = (uint16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( iSlave, TCON_REG_VALIDATE, (uint16_t*)&iWriteData, 1 ) )
		goto ERROR;

	if( 0 > (iReadData = i2c.Read( iSlave, TCON_REG_VALIDATE )) )
		goto ERROR;

	printf( "%s\n", (iWriteData == iReadData) ? "PASS" : "FAIL" );
	return 0;

ERROR:
	printf( "FAIL\n" );
	return -1;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsTconRight( __attribute__((unused)) void *pObj )
{
	int32_t iPort = 1;
	uint8_t iSlave = 0x09;
	int32_t iWriteData, iReadData = 0x0000;

	CNX_I2C i2c( iPort );
	if( 0 > i2c.Open() )
		goto ERROR;

	iWriteData = (uint16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( iSlave, TCON_REG_VALIDATE, (uint16_t*)&iWriteData, 1 ) )
		goto ERROR;

	if( 0 > (iReadData = i2c.Read( iSlave, TCON_REG_VALIDATE )) )
		goto ERROR;

	printf( "%s\n", (iWriteData == iReadData) ? "PASS" : "FAIL" );
	return 0;

ERROR:
	printf( "FAIL\n" );
	return -1;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsPfpga( __attribute__((unused)) void *pObj )
{
	int32_t iPort = 2;
	uint8_t iSlave = 0x0A;
	int32_t iWriteData, iReadData = 0x0000;

	CNX_I2C i2c( iPort );
	if( 0 > i2c.Open() )
		goto ERROR;

	iWriteData = (uint16_t)NX_GetRandomValue( 0x0000, 0xFFFF );
	if( 0 > i2c.Write( iSlave, PFPGA_REG_VALIDATE, (uint16_t*)&iWriteData, 1 ) ) {
		printf("0x%02X, 0x%04X\n", iSlave, PFPGA_REG_VALIDATE);
		goto ERROR;
	}

	if( 0 > (iReadData = i2c.Read( iSlave, PFPGA_REG_VALIDATE )) )
		goto ERROR;

	printf( "%s\n", (iWriteData == iReadData) ? "PASS" : "FAIL" );
	return 0;

ERROR:
	printf( "FAIL\n" );
	return -1;
}

//------------------------------------------------------------------------------
#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

static int32_t DiagnosticsEEPRom( void *pObj )
{
	NX_DIAG_INFO *pInfo = (NX_DIAG_INFO*)pObj;

	CNX_EEPRom eeprom;
	CNX_EEPRomData parser;
	TCON_EEPROM_INFO *pTconInfo = NULL;

	int32_t iAddr = 0, iReadSize;
	int32_t iRemainSize;
	uint8_t *pPtr;

	uint8_t buf[TCON_EEPROM_DATA_SIZE];
	uint8_t version[TCON_EEPROM_MAX_VERSION_SIZE];

	if( 0 > pInfo->iType )
		goto ERROR;

	if( 0 == pInfo->iType )
	{
		iAddr       = 0;
		pPtr        = buf;
		iRemainSize = TCON_EEPROM_DATA_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
				goto ERROR;

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			// fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_DATA_SIZE);
			// fflush(stdout);
		}
		// printf("\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_DATA_SIZE);
	}
	else
	{
		iAddr       = 0;
		pPtr        = buf;
		iRemainSize = TCON_EEPROM_VERSION_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
				goto ERROR;

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			// fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_DATA_SIZE);
			// fflush(stdout);
		}
		// printf("\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_VERSION_SIZE);
	}

	if( 0 > parser.Init( buf, TCON_EEPROM_VERSION_SIZE ) )
		goto ERROR;

	if( 0 > parser.ParseVersion( &pTconInfo ) )
		goto ERROR;

	memcpy( version, pTconInfo->version, TCON_EEPROM_MAX_VERSION_SIZE );
	printf("%s\n", version);

	parser.Deinit();
	return 0;

ERROR:
	parser.Deinit();

	printf("FAIL\n");
	return -1;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsMarriageTamper( __attribute__((unused)) void *pObj )
{
	const char *pSockName	= "cinema.secure";
	const char *pResult		= "Marriage";

	int32_t bPass = false;

	int32_t iSock, iLen, iClnSock = -1;
	struct sockaddr_un stAddr;
	char buf[128];

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
		goto ERROR;

	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, pSockName );
	iLen = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > bind(iSock, (struct sockaddr *)&stAddr, iLen) )
		goto ERROR;

	if( 0 > listen(iSock, 5) )
		goto ERROR;

	iClnSock = Accept( iSock, (struct sockaddr*)&stAddr, (socklen_t*)&iLen );
	if( 0 > iClnSock )
		goto ERROR;

	memset( buf, 0x00, sizeof(buf) );
	if( 0 > Read( iClnSock, buf, sizeof(buf) ) )
		goto ERROR;

	if( !strncmp( buf, pResult, strlen(pResult)) )
		bPass = true;

	printf("%s\n", bPass ? "PASS" : "FAIL");
	return 0;

ERROR:
	printf("FAIL\n");

	if( 0 < iClnSock )
	{
		close( iClnSock );
		iClnSock = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsDoorTamper( __attribute__((unused)) void *pObj )
{
	const char *pSockName	= "cinema.tamper";
	const char *pResult		= "Error DoorTamper";

	int32_t bPass = false;

	int32_t iSock, iLen, iClnSock = -1;
	struct sockaddr_un stAddr;
	char buf[128];

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
		goto ERROR;

	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, pSockName );
	iLen = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > bind(iSock, (struct sockaddr *)&stAddr, iLen) )
		goto ERROR;

	if( 0 > listen(iSock, 5) )
		goto ERROR;

	iClnSock = Accept( iSock, (struct sockaddr*)&stAddr, (socklen_t*)&iLen );
	if( 0 > iClnSock )
		goto ERROR;

	memset( buf, 0x00, sizeof(buf) );
	if( 0 > Read( iClnSock, buf, sizeof(buf) ) )
		goto ERROR;

	if( !strncmp( buf, pResult, strlen(pResult)) )
		bPass = true;

	printf("%s\n", bPass ? "PASS" : "FAIL");
	return 0;

ERROR:
	printf("FAIL\n");

	if( 0 < iClnSock )
	{
		close( iClnSock );
		iClnSock = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
static int32_t DiagnosticsNetwork( void *pObj )
{
	NX_DIAG_INFO *pInfo = (NX_DIAG_INFO*)pObj;
	int32_t iRet = -1;

	if( 0 == strlen(pInfo->szIpAddr) )
		goto ERROR;

	iRet = ping( pInfo->szIpAddr );

ERROR:
	printf("%s\n", !iRet ? "PASS" : "FAIL");
	return iRet;
}

//------------------------------------------------------------------------------
static void Usage( const char *pAppName )
{
	printf("Usage : %s [option]                 \n", pAppName);
	printf("                                              \n");
	printf(" Options                                      \n");
	printf("   -v           : Version Information.        \n");
	printf("   -c [command] : Command Type.               \n");
	printf("            0   : I2C0 ( TCON Left )          \n");
	printf("            1   : I2C1 ( TCON Right )         \n");
	printf("            2   : I2C2 ( P.FPGA )             \n");
	printf("            3   : I2C8 ( EEPROM )             \n");
	printf("            4   : Marriage Tamper Detector    \n");
	printf("            5   : Door Tamper Detector        \n");
	printf("            6   : Network                     \n");
	printf("   -i [ip addr] : IP Address for Network Test.\n");
	printf("   -t [type]    : Type for EEPRom Test        \n");
	printf("                                              \n");
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	NX_DIAG_INFO info;

	int32_t iOpt;
	int32_t (*pDiagnosticsFunc[NX_DIAG_MAX])( void *pObj );

	pDiagnosticsFunc[0] = &DiagnosticsTconLeft;
	pDiagnosticsFunc[1] = &DiagnosticsTconRight;
	pDiagnosticsFunc[2] = &DiagnosticsPfpga;
	pDiagnosticsFunc[3] = &DiagnosticsEEPRom;
	pDiagnosticsFunc[4] = &DiagnosticsMarriageTamper;
	pDiagnosticsFunc[5] = &DiagnosticsDoorTamper;
	pDiagnosticsFunc[6] = &DiagnosticsNetwork;

	memset( &info, 0x00, sizeof(info) );
	info.iCommand = -1;
	info.iType    = -1;

	while( -1 != (iOpt = getopt(argc, argv, "hvc:i:t:")) )
	{
		switch( iOpt )
		{
		case 'h':
			Usage(argv[0]);
			return 0;
		case 'v':
			printf("%s\n", NX_CINEMA_DIAGNOSTICS_VERSION);
			return 0;
		case 'c':
			info.iCommand = atoi(optarg);
			break;
		case 'i':
			snprintf( info.szIpAddr, sizeof(info.szIpAddr), "%s", optarg );
			break;
		case 't':
			info.iType = atoi(optarg);
			break;
		default	:
			break;
		}
	}

	if( 0 > info.iCommand )
		return -1;

	pDiagnosticsFunc[info.iCommand]( (void*)&info );
	return 0;
}
