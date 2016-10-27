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

static void help( void )
{
	printf(
		"usage: options\n"
		"-p	port i2c-n, default i2c-0 \n"
		"-r range, default 0 - 127"
	);
}

int32_t main( int32_t argc, char *argv[] )
{
	int32_t iPort = 0;
	int32_t iFrom = 0, iTo = 127;
	int32_t iFound = 0;
	int32_t opt;

	while (-1 != (opt = getopt(argc, argv, "hp:r:")))
	{
		switch(opt)
		{
		case 'p':
			iPort = strtol(optarg, NULL, 10);
			break;
		case 'r':
			optind--;
			if( argc > optind ) sscanf(argv[optind++], "%d", &iFrom);
			if( argc > optind ) sscanf(argv[optind++], "%d", &iTo);
			break;
		case 'h':
			help();
			return 0;
		default:
			break;
		}
	}

	CNX_I2C *pI2CCtrl = new CNX_I2C( iPort );
	if( 0 > pI2CCtrl->Open() )
	{
		printf("Fail, Open().\n");
		delete pI2CCtrl;
		return -1;
	}

	printf("i2c-%d valid slave address [0x%02X-0x%02X]\n :", iPort, iFrom, iTo);

	for( int32_t i = iFrom; i < iTo; i++ )
	{
		int32_t iRet = pI2CCtrl->Read( i, 0x0000 );
		if( 0 > iRet )
			continue;

		iFound++;
		printf(" 0x%02X", i);
	}

	printf("\nFind %d EA\n", iFound);

	if( pI2CCtrl )
	{
		pI2CCtrl->Close();
		delete pI2CCtrl;
	}

	return 0;
}
