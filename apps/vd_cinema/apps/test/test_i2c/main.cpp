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
		"-s	slave id (7bit hex)       \n"
		"-a	address (16bit hex)       \n"
		"-w	write (16bit hex)         \n"
	);
}

int32_t main( int32_t argc, char *argv[] )
{
	int32_t iPort = 0;
	uint8_t iSlave = 0;

	uint16_t iAddr = 0, iWriteData = 0;
	int32_t bWrite = false;

	int32_t iTemp = 0;
	int32_t opt;

	while (-1 != (opt = getopt(argc, argv, "hp:s:a:w:")))
	{
		switch(opt)
		{
		case 'p':
			iPort = strtol(optarg, NULL, 10);
			break;
		case 's':
			sscanf(optarg, "%x", &iTemp);
			iSlave = iTemp;
			break;
		case 'a':
			sscanf(optarg, "%hx", &iAddr);
			break;
		case 'w':
			sscanf(optarg, "%hx", &iWriteData);
			bWrite = true;
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

	if( bWrite )
	{
		if( 0 > pI2CCtrl->Write( iSlave, iAddr, &iWriteData, 1 ) )
		{
			printf("Fail, Write().\n");
		}
	}
	else
	{
		int32_t iReadData = pI2CCtrl->Read( iSlave, iAddr );

		if( 0 > iReadData )
		{
			printf("Fail, Read().\n");
		}
		else
		{
			printf("read data = 0x%04x\n", iReadData );
		}
	}

	if( pI2CCtrl )
	{
		pI2CCtrl->Close();
		delete pI2CCtrl;
	}

	return 0;
}
