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
		"-i input binary file for writing.  ( mandatory )\n"
		"-o output binary file for reading. ( optional ) \n"
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt, bFail = false;
	FILE *pInFile = NULL, *pOutFile = NULL;

	while (-1 != (opt = getopt(argc, argv, "hi:o:")))
	{
		switch(opt)
		{
		case 'i':
			pInFile = fopen( optarg, "rb" );
			break;
		case 'o':
			pOutFile = fopen( optarg, "wb" );
			break;
		case 'h':
			help();
			return 0;
		default:
			break;
		}
	}

	if( pInFile == NULL )
	{
		printf("Fail, Invalid input file.\n");
		return -1;
	}

	//
	//	Calculate File Size.
	//
	int32_t iSize;
	fseek( pInFile, 0, SEEK_END );
	iSize = ftell( pInFile );
	rewind( pInFile );

	CNX_EEPRom eeprom;
	int32_t iRemainSize, iWriteSize, iReadSize, iTotalSize;
	int32_t iAddr;
	uint8_t *pBuf, *pPtr;

	pBuf = (uint8_t*)malloc( iSize );
	iTotalSize = fread( pBuf, 1, iSize, pInFile );

	if( iTotalSize != iSize )
	{
		printf("Fail, fread()\n");
		goto ERROR;
	}

	//
	//	Writing Operation
	//
	iRemainSize = iTotalSize;
	iAddr       = 0;
	pPtr        = pBuf;

	eeprom.WriteProtection( false );

	while( iRemainSize > 0 )
	{
		if( iRemainSize > 256 ) iWriteSize = 256;
		else iWriteSize = iRemainSize;

		if( 0 > eeprom.Write( iAddr, pPtr, iWriteSize ) )
		{
			printf("Fail, Write().\n");
			eeprom.WriteProtection( true );
			goto ERROR;
		}

		iAddr       += iWriteSize;
		pPtr		+= iWriteSize;
		iRemainSize -= iWriteSize;

		fprintf(stdout, "EEPRom Write. ( 0x%08X / 0x%08x )\r", iAddr, iSize);
		fflush(stdout);
	}

	eeprom.WriteProtection( true );
	printf("\nEEPRom Write Done. ( %d bytes )\n", iAddr);

	//
	//	Reading EEPRom & File Write Operation
	//
	if( pOutFile )
	{
		iRemainSize = iTotalSize;
		iAddr       = 0;
		pPtr        = pBuf;

		while( iRemainSize > 0 )
		{
			if( iRemainSize > 256 ) iReadSize = 256;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				printf("Fail, Read().\n");
				goto ERROR;
			}

			fwrite( pPtr, 1, iReadSize, pOutFile );

			iAddr       += iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read & File Write. ( 0x%08X / 0x%08x )\r", iAddr, iSize);
			fflush(stdout);
		}

		printf("\nEEPRom Read & File Write Done. ( %d bytes )\n", iSize);
	}

ERROR:
	if( pBuf )		free( pBuf );
	if( pOutFile )	fclose( pOutFile );
	if( pInFile )	fclose( pInFile );

	return 0;
}
