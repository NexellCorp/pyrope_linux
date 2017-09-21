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
static void NX_HexDump( const void *data, int32_t size, const char *msg )
{
	const uint8_t *byte = (const uint8_t *)data;

	printf("%s ( %d bytes ):", msg, size);

	for( int32_t i = 0; i < size; ++i)
	{
		if ((i % 16) == 0)
		{
			printf("\n%04x", i);
		}

		printf(" %02x", byte[i]);
	}
	printf("\n");
}

//------------------------------------------------------------------------------
int32_t main()
{
	CNX_EEPRom eeprom;
	uint8_t iReadBuf[512];
	memset( iReadBuf, 0x00, sizeof(iReadBuf) );

	iReadBuf[0] = eeprom.Read( 0x0000 );

	for( int i = 1; i < (int32_t)sizeof(iReadBuf); i++ )
	{
		iReadBuf[i] = eeprom.Read();
	}

	NX_HexDump( iReadBuf, sizeof(iReadBuf), "Data: ");

	return 0;
}
