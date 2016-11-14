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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <NX_IPCServer.h>
#include <NX_IPCClient.h>
#include <NX_IPCCommand.h>

//------------------------------------------------------------------------------
#define TCON_ADDRESS_FROM		0x36
#define TCON_ADDRESS_TO			0x37

//------------------------------------------------------------------------------
static void TconStatus( int32_t iFrom, int32_t iTo )
{
	uint8_t buf = 0x00;
	int32_t size = sizeof(buf);

	for( int32_t i = iFrom; i <= iTo; i++ )
	{
		NX_TCONCommand( i, TCON_CMD_STATUS, &buf, &size );

		printf( "[0x%02X] TCon Status : %s\n", i, 
			(buf == 1) ? "OK" : "NOK" );
	}
}

//------------------------------------------------------------------------------
static void TconDoorTamper( int32_t iFrom, int32_t iTo )
{
	uint8_t buf = 0x00;
	int32_t size = sizeof(buf);

	for( int32_t i = iFrom; i <= iTo; i++ )
	{
		NX_TCONCommand( i, TCON_CMD_DOOR_STATUS, &buf, &size );
		
		printf( "[0x%02X] Door Status : %s\n", i, 
			buf == 0 ? "CLOSE" :
			buf == 1 ? "OPEN" : 
			buf == 2 ? "OPEN -> CLOSE" : "UNKNOWN" );
	}
}

//------------------------------------------------------------------------------
static void TconLedStatus( int32_t iFrom, int32_t iTo )
{
	uint8_t buf = 0x00;
	int32_t size = sizeof(buf);

	for( int32_t i = iFrom; i <= iTo; i++ )
	{
		NX_TCONCommand( i, TCON_CMD_OPEN_POS, &buf, &size );
	}	
}

//------------------------------------------------------------------------------
int32_t main( void )
{

	// TconStatus( TCON_ADDRESS_FROM, TCON_ADDRESS_TO );
	// TconDoorTamper( TCON_ADDRESS_FROM, TCON_ADDRESS_TO );
	// TconLedStatus( TCON_ADDRESS_FROM, TCON_ADDRESS_TO );
	uint8_t buf[2] = {0, 0};
	int32_t size = sizeof(buf);

	NX_TCONCommand( 0x09, TCON_CMD_PATTERN, buf, &size );

	// NX_TCONCommand( 1, TCON_CMD_OPEN, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_OPEN_POS, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_SHORT, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_SHORT_POS, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_DOOR_STATUS_STATUS, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_ON, buf, size );
	// NX_TCONCommand( 1, TCON_CMD_VERSION, buf, size );


	return 0;
}
