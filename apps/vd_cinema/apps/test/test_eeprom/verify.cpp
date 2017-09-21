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
static uint64_t NX_GetTickCount( void )
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	return ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
}

//------------------------------------------------------------------------------
static int32_t NX_InitRandomValue()
{
	srand( (uint32_t)NX_GetTickCount() );
	return 0;
}

//------------------------------------------------------------------------------
static int32_t NX_GetRandomValue( int32_t iStartNum, int32_t iEndNum )
{
	if( iStartNum >= iEndNum )
		return -1;

	return rand() % (iEndNum - iStartNum + 1) + iStartNum;
}

//------------------------------------------------------------------------------
static void NX_HexDump( const void *data, int32_t size, const char *msg, int32_t addr = 0 )
{
	const uint8_t *byte = (const uint8_t *)data;

	printf("%s ( %d bytes ):", msg, size);

	for( int32_t i = 0; i < size; ++i)
	{
		if ((i % 16) == 0)
		{
			printf("\n%04x", i + addr);
		}

		printf(" %02x", byte[i]);
	}
	printf("\n");
}

//------------------------------------------------------------------------------
static void help( void )
{
	printf(
		"usage: options\n"
		"-a eeprom 256 aligned start address (16bit hex) \n"
		"-s eeprom size  ( max 256 )                     \n"
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;
	int32_t iRet = 0;
	int32_t iAddr = -1, iSize = 0;
	uint32_t iTemp = 0;

	while (-1 != (opt = getopt(argc, argv, "ha:s:")))
	{
		switch(opt)
		{
		case 'a':
			sscanf(optarg, "%x", &iAddr);
			break;
		case 's':
			iSize = strtol(optarg, NULL, 10);
			break;
		case 'h':
			help();
			return 0;
		default:
			break;
		}
	}

	if( (0 > iAddr) || (0 >= iSize) || (256 < iSize) )
	{
		printf("Fail, Check Parameter. ( addr: 0x%04X, size: %d )\n", iAddr, iSize );
		return -1;
	}

	if( (iAddr % 256) != 0 )
	{
		printf("Fail, Check Parameter. ( address must be aligned 256, maybe 0x%04X or 0x%04X )\n",
			(iAddr / 256) * 256, ((iAddr + 256) / 256) * 256 );
		return -1;
	}

	CNX_EEPRom eeprom;
	uint8_t *pWriteBuf = (uint8_t*)malloc( iSize );
	uint8_t *pReadBuf = (uint8_t*)malloc( iSize );

	memset( pWriteBuf, 0x00, iSize );
	memset( pReadBuf, 0x00, iSize );

	NX_InitRandomValue();

	for( int32_t i = 0; i < iSize; i++ )
	{
		pWriteBuf[i] = NX_GetRandomValue( 0x00, 0xFF );
	}

	//
	//	Write Random Data
	//
	eeprom.WriteProtection( false );
	eeprom.Write( iAddr, pWriteBuf, iSize );
	eeprom.WriteProtection( true );
	NX_HexDump( pWriteBuf, iSize, "Write", iAddr );

	//
	//	Stable Time
	//
	usleep(1000000);

	//
	//	Read Data
	//
	eeprom.Read( iAddr, pReadBuf, iSize );
	NX_HexDump( pReadBuf, iSize, "Read", iAddr );

	//
	//	Verify
	//
	int32_t bVerify = true;
	for( int32_t i = 0; i < iSize; i++ )
	{
		if( pReadBuf[i] != pWriteBuf[i] )
		{
			printf("[%04X] Write Data: 0x%02X <--> Read Data: 0x%02X\n", iAddr + i, pWriteBuf[i], pReadBuf[i]);
			bVerify = false;
		}
	}

	if( !bVerify )
	{
		printf("Fail, Verify.\n");
	}

	if( pWriteBuf ) free( pWriteBuf );
	if( pReadBuf ) free( pReadBuf );

	return 0;
}
