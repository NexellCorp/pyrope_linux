//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
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

#include <CNX_EEPRom.h>

//------------------------------------------------------------------------------
static void help( void )
{
	printf(
		"usage: options\n"
		"-r eeprom address (17bit hex) \n"
		"-w eeprom data (8bit hex)     \n"
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;
	int32_t iRet = 0;
	int32_t iAddr = -1;
	uint8_t iWriteData = 0;
	uint16_t iTemp = 0;
	int32_t bWrite = false;

	while (-1 != (opt = getopt(argc, argv, "hr:w:")))
	{
		switch(opt)
		{
		case 'r':
			sscanf(optarg, "%x", &iAddr);
			break;
		case 'w':
			sscanf(optarg, "%hx", &iTemp);
			iWriteData = (uint8_t)iTemp;
			bWrite = true;
			break;
		case 'h':
			help();
			return 0;
		default:
			break;
		}
	}

	int32_t iReadData = 0;
	CNX_EEPRom eeprom;

	if( bWrite )
	{
		if( 0 <= iAddr )
		{
			eeprom.WriteProtection( false );
			if( 0 > eeprom.Write( iAddr, iWriteData ) )
			{
				printf("Fail, Write().\n");
				eeprom.WriteProtection( true );
				return -1;
			}

			printf("[wr] addr: 0x%05X, data: 0x%02X\n", (uint32_t)iAddr, iWriteData );
			eeprom.WriteProtection( true );
		}
	}
	else
	{
		if( 0 > iAddr )
		{
			if( 0 > (iReadData = eeprom.Read()) )
			{
				printf("Fail, Read().\n");
				return -1;
			}

			printf("[rd] addr: current, data: 0x%02X\n", iReadData);
		}
		else
		{
			if( 0 > (iReadData = eeprom.Read(iAddr)) )
			{
				printf("Fail, Read().\n");
				return -1;
			}

			printf("[rd] addr: 0x%05X, data: 0x%02X\n", (uint32_t)iAddr, iReadData);
		}
	}

	return 0;
}
