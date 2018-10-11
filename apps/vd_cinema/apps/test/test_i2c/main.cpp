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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <CNX_I2C.h>

//------------------------------------------------------------------------------
#include <android/log.h>
#define NX_DTAG				"nx_i2c"
#define NX_PRINT_ADB(...)	do {						\
								__android_log_print(	\
									ANDROID_LOG_VERBOSE,\
									NX_DTAG,			\
									__VA_ARGS__			\
								);						\
							} while(0)

#define MAX_DATA_SIZE		1024

//------------------------------------------------------------------------------
static void help( void )
{
	printf(
		"usage: options\n"
		"-p	port i2c-n, default i2c-0 \n"
		"-s	slave id   ( 7bit hex )   \n"
		"-r	address    ( 16bit hex )  \n"
		"-w	write      ( 16bit hex )  \n"
		"-d	display return value      \n"
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t iPort = 0;
	uint8_t iSlave = 0;

	uint16_t iAddr = 0;
	uint16_t iWriteData[MAX_DATA_SIZE] = { 0x0000, };
	int32_t iWriteSize = 0;

	int32_t iTemp = 0;
	int32_t opt;

	int32_t bMsgRet = false;
	int32_t iRet = -1;

	char szData[MAX_DATA_SIZE * 7] = { 0x00, };

	while (-1 != (opt = getopt(argc, argv, "hp:s:r:w:d")))
	{
		switch(opt)
		{
		case 'p':
		{
			iPort = strtol(optarg, NULL, 10);
			break;
		}
		case 's':
		{
			sscanf(optarg, "%x", &iTemp);
			iSlave = iTemp;
			break;
		}
		case 'r':
		{
			sscanf(optarg, "%hx", &iAddr);
			break;
		}
		case 'w':
		{
			char *args = optarg;
			while( NULL != args )
			{
				iWriteData[iWriteSize] = strtol(args, NULL, 16);
				iWriteSize++;

				args = strchr(args, ',');
				if( !args )
					break;

				args++;
			}
			break;
		}
		case 'd':
		{
			bMsgRet = true;
			break;
		}
		case 'h':
		{
			help();
			return 0;
		}
		default:
			break;
		}
	}

	CNX_I2C *pI2CCtrl = new CNX_I2C( iPort );
	if( 0 > pI2CCtrl->Open() )
	{
		printf( "Fail, Open().\n" );
		delete pI2CCtrl;
		return -1;
	}

	if( 0 < iWriteSize )
	{
		iRet = pI2CCtrl->Write( iSlave, iAddr, iWriteData, iWriteSize );

		snprintf( szData, sizeof(szData), "0x%04x", iWriteData[0]);
		for( int32_t i = 1; i < iWriteSize; i++ )
		{
			snprintf( szData+strlen(szData), sizeof(szData)-strlen(szData),
				",0x%04x", iWriteData[i] );
		}

		if( 0 > iRet )
		{
			printf( "Fail, Write().\n" );
		}
		else
		{
			printf( "[wr] i2c-%d, slave: 0x%02x, addr: 0x%04x, data: %s\n", iPort, iSlave, iAddr, szData );
		}
	}
	else
	{
		iRet = pI2CCtrl->Read( iSlave, iAddr );

		snprintf( szData, sizeof(szData), "0x%04x", iRet);

		if( 0 > iRet )
		{
			printf( "Fail, Read().\n");
		}
		else
		{
			printf( "[rd] i2c-%d, slave: 0x%02x, addr: 0x%04x, data: %s\n", iPort, iSlave, iAddr, szData );
		}
	}

	if( bMsgRet )
	{
		NX_PRINT_ADB( "[%s] i2c-%d, slave: 0x%02x, addr: 0x%04x, data: %s\n",
			(0 < iWriteSize) ? "wr" : "rd",
			iPort, iSlave, iAddr, szData
		);

		if( 0 > iRet )
		{
			NX_PRINT_ADB( "Fail, %s().\n", (0 < iWriteSize) ? "Write" : "Read" );
		}

		printf("%d\n", iRet);
	}

	if( pI2CCtrl )
	{
		pI2CCtrl->Close();
		delete pI2CCtrl;
	}

	return 0;
}
