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
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>	//
#include <sys/stat.h>	//directory
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_I2C.h>
#include <CNX_EEPRom.h>
#include <CNX_EEPRomDataParser.h>
#include <NX_I2CRegister.h>

#include <NX_Utils.h>

#include <NX_IPCCommand.h>

#include <NX_Version.h>

#define NX_DTAG	"[NX_Command]"
#include <NX_DbgMsg.h>

#include "NX_Command.h"

#define FAKE_DATA			0
#define I2C_DEBUG			0
#define I2C_SEPARATE_BURST	1

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

#define TCON_BURST_RETRY_COUNT			3

#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

#define TCON_EEPROM_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_EEPROM"
// #define TCON_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_USB"
// #define PFPGA_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/PFPGA"

#define TCON_EEPROM_BINARY_FILE			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.bin"

//enum { MAX_LOD_MODULE = 12, MAX_STR_SIZE = 1024 };
#define MAX_LOD_MODULE	12
// #define MAX_STR_SIZE	1024


//------------------------------------------------------------------------------
//Test Pattern Functions
static int32_t	(*m_pTestPatternFunc[6])( CNX_I2C*, uint8_t, uint8_t );

static int32_t TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );

//------------------------------------------------------------------------------
//in TCON_EEPRomRead
static void    MakeDirectory( const char *pDir );




//------------------------------------------------------------------------------
//
//	IMB Commands
//

//------------------------------------------------------------------------------
int32_t IMB_ChangeContents( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize)
{
	UNUSED( cmd );
	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * 4 ) ;
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;


	return 0;
}


//------------------------------------------------------------------------------
//
//	Burst
//

//------------------------------------------------------------------------------
int32_t CMD_BURSTStart( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );
	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * 4 ) ;
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

}

//------------------------------------------------------------------------------
int32_t CMD_BURSTStop( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );
	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * 4 ) ;
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

//goto ERROR;


	return 0;


ERROR:
	return -1;
}


//------------------------------------------------------------------------------
//
//	TCON Commands
//

//------------------------------------------------------------------------------
int32_t	TCON_RegWrite( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1] : msb reg, pBuf[2] : lsb reg, pBuf[3] : msb data, pBuf[4] : lsb data
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * 4 ) ;
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];
	uint8_t msbData	= pBuf[3];
	uint8_t lsbData	= pBuf[4];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	// printf( "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", port, slave, inReg, inData );

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_RegRead( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1] : msb reg, pBuf[2] : lsb reg
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_TCON;
	}

	pTemp[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	pTemp[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	pTemp[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	pTemp[3] = (uint8_t)((iReadData >>  0) & 0xFF);

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
// have to do.....
int32_t	TCON_RegBurstWrite( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	//	Not Implemeation.
	//
	*pResultSize = 1;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t));
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;


	return 0;

// ERROR_TCON:

	// return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_Init( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA1, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA1, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA2, 0x0025 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA2, 0x0025 );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_Status( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iWriteData, iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	iWriteData = (int16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( slave, TCON_REG_CHECK_STATUS, (uint16_t*)&iWriteData, 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_STATUS, iWriteData );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_STATUS );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	if(iWriteData == iReadData)
	{
		pTemp[3] = 0x01;
	}
	else
	{
		pTemp[3] = 0x00;
	}

#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_DoorStatus( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0, 2 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_DOOR_READ )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_DOOR_READ );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = (uint8_t)iReadData;

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 1 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 0 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 0 );
		goto ERROR_TCON;
	}
#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_LvdsStatus( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_LVDS_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LVDS_STATUS );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	if (iReadData == 1080 || iReadData == 2160)
	{
		pTemp[3] = 0x01;
	}else
	{
		pTemp[3] = 0x00;
	}
#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_BootingStatus( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_BOOTING_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_BOOTING_STATUS );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;

	if(iReadData == 0x07E1)
	{
		pTemp[3] = 0x01;
	}else
	{
		pTemp[3] = 0x00;
	}
#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_LedModeNormal( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = 0;
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_PATTERN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_LedModeLod( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = 0;
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	const uint16_t pattern[] = {
		10,		0,		1024,	0,		2160,	4095,	4095,	4095
	};

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(pattern) / sizeof(pattern[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_PATTERN + i, pattern[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_PATTERN + i, pattern[i] );
			goto ERROR_TCON;
		}
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );

		goto ERROR_TCON;
	}

	usleep( 3000000 );	// delay during 3.00sec over ( LOD Scan Time )

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

#endif

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_LedOpenNum( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t)*4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] =  0xFF;
	pTemp[1] =  0xFF;
	pTemp[2] =  0xFF;
	pTemp[3] =  0xFF;

	int32_t iReadData = 0;
	int32_t iErrorNum = 0;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_LOD_100_CHK_DONE ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LOD_100_CHK_DONE );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_ERROR_NUM_OVR ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_NUM_OVR );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		iReadData = i2c.Read( slave, TCON_REG_ERROR_NUM_M1 + i );
		if( 0 > iReadData )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_ERROR_NUM_M1 + i );
			goto ERROR_TCON;
		}

		iErrorNum += iReadData;
	}

	pTemp[0] = (uint8_t)(iErrorNum >> 24) & 0xFF;
	pTemp[1] = (uint8_t)(iErrorNum >> 16) & 0xFF;
	pTemp[2] = (uint8_t)(iErrorNum >>  8) & 0xFF;
	pTemp[3] = (uint8_t)(iErrorNum >>  0) & 0xFF;

	return 0;

ERROR_TCON:

	return -1;;
}

//------------------------------------------------------------------------------
int32_t	TCON_LedOpenPos( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t)*4);
	uint8_t* pTemp = *pResult;
	pTemp[0] =  0xFF;
	pTemp[1] =  0xFF;
	pTemp[2] =  0xFF;
	pTemp[3] =  0xFF;

	int32_t iCoordinateX = 0, iCoordinateY = 0;
	int32_t iErrorOutReady;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	usleep( 50000 );

	iErrorOutReady = i2c.Read( slave, TCON_REG_ERROR_OUT_RDY );
	if( iErrorOutReady )
	{
		iCoordinateX = i2c.Read( slave, TCON_REG_X_COORDINATE );
		iCoordinateY = i2c.Read( slave, TCON_REG_Y_COORDINATE );

		pTemp[0] = (uint8_t)((iCoordinateX >> 8 ) & 0xFF);
		pTemp[1] = (uint8_t)((iCoordinateX >> 0 ) & 0xFF);
		pTemp[2] = (uint8_t)((iCoordinateY >> 8 ) & 0xFF);
		pTemp[3] = (uint8_t)((iCoordinateY >> 0 ) & 0xFF);
	}

	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0001 );
	usleep( 50000 );
	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0000 );

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_TestPattern( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1] : function index, pBuf[2] : pattern index
	//

	m_pTestPatternFunc[0] = &TestPatternDci;
	m_pTestPatternFunc[1] = &TestPatternColorBar;
	m_pTestPatternFunc[2] = &TestPatternFullScreenColor;
	m_pTestPatternFunc[3] = &TestPatternGrayScale;
	m_pTestPatternFunc[4] = &TestPatternDot;
	m_pTestPatternFunc[5] = &TestPatternDiagonal;


	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t funcIndex = pBuf[1];
	uint8_t patternIndex = pBuf[2];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( TCON_CMD_PATTERN_RUN == cmd )
	{
		if( funcIndex != sizeof(m_pTestPatternFunc)/sizeof(m_pTestPatternFunc[0]) )
		{
			m_pTestPatternFunc[funcIndex](&i2c, index, patternIndex);
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0001 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );

				if(iReadData != 0x0001)
				{
					pTemp[0] = 0x00;
					pTemp[1] = 0x00;
					pTemp[2] = 0x00;
					pTemp[3] = 0x00;
				}
			}
#endif
		}
	}
	else
	{
		if( 0 > i2c.Write( slave, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_XYZ_TO_RGB, 0x0000 );
			goto ERROR_TCON;
		}

		if( funcIndex != sizeof(m_pTestPatternFunc)/sizeof(m_pTestPatternFunc[0]) )
		{
			if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_PATTERN, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_PATTERN )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_PATTERN );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );

				if(iReadData != 0x0000)
				{
					pTemp[0] = 0x00;
					pTemp[1] = 0x00;
					pTemp[2] = 0x00;
					pTemp[3] = 0x00;
				}
			}
#endif
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );

				if(iReadData != 0x0000)
				{
					pTemp[0] = 0x00;
					pTemp[1] = 0x00;
					pTemp[2] = 0x00;
					pTemp[3] = 0x00;
				}
			}
#endif
		}
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_TargetGamma( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1] .. : data ( num of data : nSize - 1 )
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pBuf + 1;
	uint32_t size	= nSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_TGAM_R_WDATA + (cmd - TCON_CMD_TGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0001 << (cmd - TCON_CMD_TGAM_R);

	uint32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

	for( uint32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[%d] [ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	i, oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize / 4 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	free( pMsbData );
	free( pLsbData );
	return 0;

ERROR_TCON:

	free( pMsbData );
	free( pLsbData );

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_DeviceGamma( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	//
	//	pBuf[0] : index, pBuf[1] .. : data ( num of data : nSize - 1 )
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pBuf + 1;
	uint32_t size	= nSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_DGAM_R_WDATA + (cmd - TCON_CMD_DGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0008 << (cmd - TCON_CMD_DGAM_R);

	uint32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

	for( uint32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize / 4 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	free( pMsbData );
	free( pLsbData );
	return 0;

ERROR_TCON:

	free( pMsbData );
	free( pLsbData );

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_PIXELCorrection( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );

	uint64_t iCurTime = NX_GetTickCount();

	//
	//	pBuf[0] : index, pBuf[1] : module, pBuf[2] .. : data ( num of data : nSize - 1 )
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t module = pBuf[1];
	uint8_t *ptr	= pBuf + 2;
	uint32_t size	= nSize - 2;

	uint32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t ccData[9] = { 0x0000, };
	uint16_t opData[16] = { 0x0000, };

	uint16_t iCheckSum = 0;
	int32_t iRetryCount = 0;
	int32_t bFail;

	for( uint32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(ptr[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(ptr[2 * i + 1] << 0) & 0x00FF);
		iCheckSum = iCheckSum ^ pData[i];
	}

#if 0	// For Debugging.
	uint16_t *pOriData = (uint16_t*)malloc( sizeof(uint16_t) * 64 * 60 * 9 );
	int offset = 0, oriOffset = 0;

	for( int32_t i = 0; i < 64 * 60; i++ )
	{
        pOriData[oriOffset    ] = ((pData[offset + 7]      ) & 0x3FFF);
        pOriData[oriOffset + 1] = ((pData[offset + 6] >>  2) & 0x3FFF);
        pOriData[oriOffset + 2] = ((pData[offset + 6] << 12) & 0x3000) | ((pData[offset + 5] >>  4) & 0x0FFF);
        pOriData[oriOffset + 3] = ((pData[offset + 5] << 10) & 0x3C00) | ((pData[offset + 4] >>  6) & 0x03FF);
        pOriData[oriOffset + 4] = ((pData[offset + 4] <<  8) & 0x3F00) | ((pData[offset + 3] >>  8) & 0x00FF);
        pOriData[oriOffset + 5] = ((pData[offset + 3] <<  6) & 0x3FC0) | ((pData[offset + 2] >> 10) & 0x003F);
        pOriData[oriOffset + 6] = ((pData[offset + 2] <<  4) & 0x3FF0) | ((pData[offset + 1] >> 12) & 0x000F);
        pOriData[oriOffset + 7] = ((pData[offset + 1] <<  2) & 0x3FFC) | ((pData[offset    ] >> 14) & 0x0003);
        pOriData[oriOffset + 8] = ((pData[offset    ]      ) & 0x3FFF);

		printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			pOriData[oriOffset + 0], pOriData[oriOffset + 1], pOriData[oriOffset + 2], pOriData[oriOffset + 3],
			pOriData[oriOffset + 4], pOriData[oriOffset + 5], pOriData[oriOffset + 6], pOriData[oriOffset + 7],
			pOriData[oriOffset + 8] );

        oriOffset += 9;
        offset += 8;
	}

	if( pOriData ) free( pOriData );
#endif

	printf("port(%d), slave(0x%02x), module(%d)\n", port, slave, module);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	// 0. Driver Clock Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Read CC data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module >> 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module >> 1 );
		goto ERROR_TCON;
	}
	usleep(100000);

	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_CC00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_CC00_READ + i );
			goto ERROR_TCON;
		}

		ccData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}

	//
	//	2. Read Optional Data
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		opData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	//
	//	3. Flash Protection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	4. Erase Flash.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(4000000);

	//
	//	5. Write Dot Correction Data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0040) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0040 );
		goto ERROR_TCON;
	}

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 30; i++ )
		{
			if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData + iDataSize / 30 * i, iDataSize / 30) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, TCON_REG_FLASH_WDATA, iDataSize / 30 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_WDATA, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}
		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

	//
	//	6. Write Screen Correction Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_CC00 + i, ccData[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CC00 + i, ccData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(500000);

	//
	//	7. Write Optional Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00 + i, opData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(500000);

	//
	//	8. Flash Protection On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	if(bFail)
	{
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
	}
	else
	{
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x01;
	}

	if( pData ) free( pData );
	return 0;

ERROR_TCON:

	if( pData ) free( pData );

	printf(">>>>> %s() Done : %llu ms\n", __FUNCTION__, NX_GetTickCount() - iCurTime );

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_PIXELCorrectionExtract( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	uint64_t iStartTime = NX_GetTickCount();

	//
	//	pBuf[0] : index, pBuf[1] : module index
	//
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pBuf[1];
	uint16_t nBaseX = module % 4 * 64;
	uint16_t nBaseY = module / 4 * 60;

	int32_t iResultLen = 64 * 60 * 8 * 2;
	uint8_t* tempBuf = (uint8_t*) malloc(sizeof(uint8_t)*iResultLen);

	uint8_t* pTemp = tempBuf;
	memset( pTemp, 0x00, iResultLen * sizeof(uint8_t) );

	// int32_t cnt = 0;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	printf("port(%d), slave(%d), modlue(%d)\n", port, slave, module);

	for( int32_t i = 0; i < 60; i++ )
	{
		for( int32_t j = 0; j < 64; j++ )
		{
			int16_t ccData14[9] = { 0x0000, };
			int16_t ccData16[8] = { 0x0000, };

#if 1
			uint16_t pos_x = j + nBaseX;
			uint16_t pos_y = i + nBaseY;

			// printf("pos_x(%d), pos_y(%d)\n", pos_x, pos_y);

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_X, pos_x ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_X, pos_x );
				goto ERROR_TCON;
			}

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_Y, pos_y ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_Y, pos_y );
				goto ERROR_TCON;
			}

			usleep(50000);

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				if( 0 > (ccData14[k] = i2c.Read( slave, TCON_REG_ADDR_CC_IN_CC00 + k )) )
				{
					NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, slave, TCON_REG_ADDR_CC_IN_CC00 + k );
					goto ERROR_TCON;
				}
				ccData14[k] &= 0x3FFF;
			}
#else
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				ccData14[k] = cnt;		// 0101 0101 0101 0101
				ccData14[k] &= 0x3FFF;	// 0001 0101 0101 0101

				cnt = (cnt + 1) % 0x3FFF;
			}
#endif

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData14[0], ccData14[1], ccData14[2], ccData14[3], ccData14[4],
			// 	ccData14[5], ccData14[6], ccData14[7], ccData14[8] );

			//
			//	Convert Data ( 14bit x 9EA --> 16bit x 8EA )
			//
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				ccData16[0] = ((ccData14[7] & 0x0003) << 14) | ((ccData14[8] & 0x3FFF) >>  0);
				ccData16[1] = ((ccData14[6] & 0x000F) << 12) | ((ccData14[7] & 0x3FFC) >>  2);
				ccData16[2] = ((ccData14[5] & 0x003F) << 10) | ((ccData14[6] & 0x3FF0) >>  4);
				ccData16[3] = ((ccData14[4] & 0x00FF) <<  8) | ((ccData14[5] & 0x3FC0) >>  6);
				ccData16[4] = ((ccData14[3] & 0x03FF) <<  6) | ((ccData14[4] & 0x3F00) >>  8);
				ccData16[5] = ((ccData14[2] & 0x0FFF) <<  4) | ((ccData14[3] & 0x3C00) >> 10);
				ccData16[6] = ((ccData14[1] & 0x3FFF) <<  2) | ((ccData14[2] & 0x3000) >> 12);
				ccData16[7] =                                  ((ccData14[0] & 0x3FFF) >>  0);
			}

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData16[0], ccData16[1], ccData16[2], ccData16[3],
			// 	ccData16[4], ccData16[5], ccData16[6], ccData16[7] );

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				pTemp[k * 2    ] = (uint8_t)((ccData16[k] >> 8) & 0xFF);
				pTemp[k * 2 + 1] = (uint8_t)((ccData16[k] >> 0) & 0xFF);
			}
			pTemp += 16;

			//	Print Progress Debugging
			fprintf( stdout, "> %4d / %4d ( %3d %% )\r",
				i * 64 + j + 1, 60 * 64,
				(int)((float)((i * 64) + j + 1) / (float)(64 * 60) * (float)100) );
			fflush( stdout );
		}
	}


	printf( "\n>> DotCorrection Extract Done. ( %llu mSec )\n", NX_GetTickCount() - iStartTime );

	*pResultSize = iResultLen;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t)*iResultLen);
	memcpy(*pResult , tempBuf , *pResultSize );

	free(tempBuf);
	return 0;

ERROR_TCON:

	printf( "\n>> DotCorrection Extract Done. ( %llu mSec )\n", NX_GetTickCount() - iStartTime );
	free(tempBuf);

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;


	return -1;
}

//------------------------------------------------------------------------------
#define IS_TCON_MODULE_TOP(module)		(((module / 4) == 0) ? true : false)
#define IS_TCON_MODULE_BOTTOM(module)	(((module / 4) == 5) ? true : false)
#define IS_TCON_MODULE_LEFT(module)		(((module % 4) == 0) ? true : false)
#define IS_TCON_MODULE_RIGHT(module)	(((module % 4) == 3) ? true : false)

#define TCON_MODULE_TOP			0
#define TCON_MODULE_BOTTOM		1
#define TCON_MODULE_LEFT		2
#define TCON_MODULE_RIGHT		3

//------------------------------------------------------------------------------
int32_t	TCON_WhiteSeamRead( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t iSeamValue[4] = { 16384, 16384, 16384, 16384 };

	uint16_t iTempValue[4];
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	//	2. Read top seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_TOP] = iReadData & 0x7FFF;

		if( (iTempValue[TCON_MODULE_TOP] != 16384) &&
			(iTempValue[TCON_MODULE_TOP] <= 31744) &&
			(iTempValue[TCON_MODULE_TOP] >= 1024 ) )
		{
			if( iSeamValue[TCON_MODULE_TOP] == iTempValue[TCON_MODULE_TOP] )
			{
				iSeamValue[TCON_MODULE_TOP] = iTempValue[TCON_MODULE_TOP];
				break;
			}
			else
			{
				iSeamValue[TCON_MODULE_TOP] = iTempValue[TCON_MODULE_TOP];
			}
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] );
		goto ERROR_TCON;
	}

	//
	//	3. Read bottom seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_BOTTOM(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA01_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA01_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_BOTTOM] = iReadData & 0x7FFF;

		if( (iTempValue[TCON_MODULE_BOTTOM] != 16384) &&
			(iTempValue[TCON_MODULE_BOTTOM] <= 31744) &&
			(iTempValue[TCON_MODULE_BOTTOM] >= 1024 ) )
		{
			if( iSeamValue[TCON_MODULE_BOTTOM] == iTempValue[TCON_MODULE_BOTTOM] )
			{
				iSeamValue[TCON_MODULE_BOTTOM] = iTempValue[TCON_MODULE_BOTTOM];
				break;
			}
			else
			{
				iSeamValue[TCON_MODULE_BOTTOM] = iTempValue[TCON_MODULE_BOTTOM];
			}
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] );
		goto ERROR_TCON;
	}

	//
	//	4. Read left seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_LEFT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA02_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_LEFT] = iReadData & 0x7FFF;

		if( (iTempValue[TCON_MODULE_LEFT] != 16384) &&
			(iTempValue[TCON_MODULE_LEFT] <= 31744) &&
			(iTempValue[TCON_MODULE_LEFT] >= 1024 ) )
		{
			if( iSeamValue[TCON_MODULE_LEFT] == iTempValue[TCON_MODULE_LEFT] )
			{
				iSeamValue[TCON_MODULE_LEFT] = iTempValue[TCON_MODULE_LEFT];
				break;
			}
			else
			{
				iSeamValue[TCON_MODULE_LEFT] = iTempValue[TCON_MODULE_LEFT];
			}
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] );
		goto ERROR_TCON;
	}

	//
	//	5. Read white seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_RIGHT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA03_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_RIGHT] = iReadData & 0x7FFF;

		if( (iTempValue[TCON_MODULE_RIGHT] != 16384) &&
			(iTempValue[TCON_MODULE_RIGHT] <= 31744) &&
			(iTempValue[TCON_MODULE_RIGHT] >= 1024 ) )
		{
			if( iSeamValue[TCON_MODULE_RIGHT] == iTempValue[TCON_MODULE_RIGHT] )
			{
				iSeamValue[TCON_MODULE_RIGHT] = iTempValue[TCON_MODULE_RIGHT];
				break;
			}
			else
			{
				iSeamValue[TCON_MODULE_RIGHT] = iTempValue[TCON_MODULE_RIGHT];
			}
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] );
		goto ERROR_TCON;
	}


	pTemp[0] = (uint32_t)iSeamValue[0];
	pTemp[1] = (uint32_t)iSeamValue[1];
	pTemp[2] = (uint32_t)iSeamValue[2];
	pTemp[3] = (uint32_t)iSeamValue[3];

	return 0;
ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_WhiteSeamWrite( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1:2] : top, pBuf[3:4] : bottom, pBuf[5:6] : left, pBuf[7:8] : right
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t opData[24][16]	= { 0x0000, };
	uint16_t iSeamValue[4];

	for( int32_t i = 0; i < 4; i++ )
	{
		iSeamValue[i] = ((int16_t)(pBuf[i*2+1] << 8) & 0xFF00) + (int16_t)pBuf[i*2+2];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	//	2. Read Optional Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	2-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	2-2. Read Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00_READ + i );
				goto ERROR_TCON;
			}

			opData[module][i] = (uint16_t)(iReadData & 0x0000FFFF);
		}
	}

	//
	//	3. Modify Seam Value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) && !IS_TCON_MODULE_BOTTOM(module) &&
			!IS_TCON_MODULE_LEFT(module) && !IS_TCON_MODULE_RIGHT(module) )
		{
			opData[module][TCON_MODULE_TOP]		= 16384;
			opData[module][TCON_MODULE_BOTTOM]	= 16384;
			opData[module][TCON_MODULE_LEFT]	= 16384;
			opData[module][TCON_MODULE_RIGHT]	= 16384;
		}

		if( IS_TCON_MODULE_TOP(module) )
		{
			opData[module][TCON_MODULE_TOP]    = iSeamValue[TCON_MODULE_TOP];
			opData[module][TCON_MODULE_BOTTOM] = 16384;

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			else								opData[module][TCON_MODULE_LEFT] = 16384;

			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
			else								opData[module][TCON_MODULE_RIGHT] = 16384;
		}

		if( IS_TCON_MODULE_BOTTOM(module) )
		{
			opData[module][TCON_MODULE_BOTTOM]	= iSeamValue[TCON_MODULE_BOTTOM];
			opData[module][TCON_MODULE_TOP]		= 16384;

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			else								opData[module][TCON_MODULE_LEFT] = 16384;

			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
			else								opData[module][TCON_MODULE_RIGHT] = 16384;
		}

		if( IS_TCON_MODULE_LEFT(module) )
		{
			opData[module][TCON_MODULE_LEFT]	= iSeamValue[TCON_MODULE_LEFT];
			opData[module][TCON_MODULE_RIGHT]	= 16384;

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			else								opData[module][TCON_MODULE_TOP] = 16384;

			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
			else								opData[module][TCON_MODULE_BOTTOM] = 16384;
		}

		if( IS_TCON_MODULE_RIGHT(module) )
		{
			opData[module][TCON_MODULE_RIGHT]	= iSeamValue[TCON_MODULE_RIGHT];
			opData[module][TCON_MODULE_LEFT]	= 16384;

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			else								opData[module][TCON_MODULE_TOP] = 16384;

			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
			else								opData[module][TCON_MODULE_BOTTOM] = 16384;
		}
	}

	//
	//	4. Write Flash Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	4-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	4-2. Flash Protection Off.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
			goto ERROR_TCON;
		}

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		//
		//	4-3. Write Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] );
				goto ERROR_TCON;
			}
		}

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
			goto ERROR_TCON;
		}

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(500000);

		//
		//	4-4. Flash Protection On.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
			goto ERROR_TCON;
		}

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);
	}

	//
	//	5. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
// uint8_t* CNX_I2CController::TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t *pBuf, uint32_t nSize )
// {
// 	UNUSED( cmd );
// 	UNUSED( pBuf );
// 	UNUSED( nSize );

// 	//
// 	//	pBuf[0] : index, pBuf[1] : module,
// 	//

// 	return 0;
// }

//------------------------------------------------------------------------------
int32_t	TCON_OptionalData( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index, pBuf[1] : module
	//

	*pResultSize = 32;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t)*32 );
	uint8_t* pTemp = *pResult;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pBuf[1];

	uint16_t opData[16] = { 0x0000, };

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	2. Pluse Generation for Flash Read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	//	3. Read Optional Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		opData[i] = (uint16_t)(iReadData & 0x0000FFFF);
		usleep(50000);
	}

	//
	//	4. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	Print Debug Message
	//
	printf(">> Optional Data Information : index(%d), module(%d)\n", index, module);

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		printf(" -. REG_F_LED_DATA%02d_READ : 0x%04x\n", i, opData[i]);
	}

	//
	//
	//
	for(int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		pTemp[i * 2 + 0] = (uint8_t)((opData[i] >> 8) & 0xFF);
		pTemp[i * 2 + 1] = (uint8_t)((opData[i] >> 0) & 0xFF);
	}

	return 0;

ERROR_TCON:

	*pResultSize = 4;
	*pResult = (uint8_t*) realloc( *pResult , sizeof(uint8_t) * 4 );
	pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_SwReset( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;

ERROR_TCON:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_EEPRomRead( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	MakeDirectory( TCON_EEPROM_RESULT_PATH );

	*pResultSize = TCON_EEPROM_DATA_SIZE;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * (*pResultSize) );
	uint8_t* pTemp = *pResult;

	int32_t iAddr = 0, iReadSize;
	int32_t iRemainSize;

	uint8_t *pInBuf = (uint8_t*)malloc( TCON_EEPROM_DATA_SIZE );
	uint8_t *pPtr;

	int32_t iRet;
	CNX_EEPRom eeprom;
	CNX_EEPRomDataParser parser;
	TCON_EEPROM_INFO *pInfo = NULL;
	uint8_t version[TCON_EEPROM_MAX_VERSION_SIZE];

	FILE *pFile = NULL;
	int32_t bUpdate = true;

	//
	//	0. Check Version
	//
	if( !access( TCON_EEPROM_BINARY_FILE, F_OK) )
	{
		iAddr       = 0;
		pPtr        = pInBuf;
		iRemainSize = TCON_EEPROM_VERSION_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n" );
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_VERSION_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}

		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_VERSION_SIZE );

		//
		//	0-1. EEPROM Version Parsing.
		//
		iRet = parser.Init( pInBuf, TCON_EEPROM_VERSION_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		memcpy( version, pInfo->version, TCON_EEPROM_MAX_VERSION_SIZE );
		parser.Deinit();

		//
		//	0-2. Binary Version Parsing.
		//
		parser.Init( TCON_EEPROM_BINARY_FILE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		// NX_HexDump( version, sizeof(version) );
		// NX_HexDump( pInfo->version, sizeof(version) );

		if( !strcmp( (char*)pInfo->version, (char*)version ) )
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version match.\n");
			bUpdate = false;
		}
		else
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version missmatch.\n");
		}

		parser.Deinit();
	}
bUpdate = true;
	if( bUpdate )
	{
		//
		//	1. Read EEPRom Data.
		//
		iAddr       = 0;
		pPtr        = pInBuf;
		iRemainSize = TCON_EEPROM_DATA_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n");
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_DATA_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}

		//get result.....
		memcpy( *pResult , pInBuf , (*pResultSize) );
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_DATA_SIZE );

		//
		//	2. Make Binary File.
		//
		pFile = fopen( TCON_EEPROM_BINARY_FILE, "wb" );
		if( NULL == pFile )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Make Binary File. ( %s )\n", TCON_EEPROM_BINARY_FILE );
			goto ERROR_TCON;
		}

		fwrite( pInBuf, 1, TCON_EEPROM_DATA_SIZE, pFile );
		fclose( pFile );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s )\n", TCON_EEPROM_BINARY_FILE );

		//
		//	3. Make T_REG_EEPROM.txt
		//
		iRet = parser.Init( pInBuf, TCON_EEPROM_DATA_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.Parse( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Parse(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		parser.WriteTconInfo( pInfo, TCON_EEPROM_RESULT_PATH );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s/T_REG_EEPROM.txt )\n", TCON_EEPROM_RESULT_PATH );

		sync();

		//pTemp[0] = 0x01;		// Update Successful.
	}
	else
	{
		//pTemp[0] = 0x00;		// Update is not needed.
	}

	//NxDbgMsg( NX_DBG_ERR, "%s(). ( pTemp = 0x%02X )\n", __FUNCTION__, pTemp );

	parser.Deinit();

	//dont need???
	free(pInBuf);
	return 0;

ERROR_TCON:
	parser.Deinit();

	*pResultSize = 4;
	*pResult = (uint8_t*) realloc( *pResult , sizeof(uint8_t) * 4 );
	pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

//dont need???
	free(pInBuf);
	return -1;
}

//------------------------------------------------------------------------------
int32_t	TCON_Version( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : index
	//

	*pResultSize = 8;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t)*8 );
	uint8_t* pTemp = *pResult;
	for(int32_t i = 0 ; i < 8 ; i++)
	{
		pTemp[i] = 0xFF;
	}

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	int32_t iModeName, iTime;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iModeName = i2c.Read( slave, TCON_REG_FPGA_MODE_NAME )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_MODE_NAME );
		goto ERROR_TCON;
	}

	if( 0 > (iTime = i2c.Read( slave, TCON_REG_FPGA_TIME1 )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_TIME1 );
		goto ERROR_TCON;
	}

	pTemp[0] = (uint8_t)((iModeName >> 24) & 0xFF);
	pTemp[1] = (uint8_t)((iModeName >> 16) & 0xFF);
	pTemp[2] = (uint8_t)((iModeName >>  8) & 0xFF);
	pTemp[3] = (uint8_t)((iModeName >>  0) & 0xFF);

	pTemp[4] = (uint8_t)((iTime >> 24) & 0xFF);
	pTemp[5] = (uint8_t)((iTime >> 16) & 0xFF);
	pTemp[6] = (uint8_t)((iTime >>  8) & 0xFF);
	pTemp[7] = (uint8_t)((iTime >>  0) & 0xFF);

	return 0;

ERROR_TCON:
	*pResultSize = 4;
	*pResult = (uint8_t*) realloc( *pResult , sizeof(uint8_t) * 4 );
	pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;
	return -1;
}


//------------------------------------------------------------------------------
//
//	PFPGA Commands
//

//------------------------------------------------------------------------------
int32_t	PFPGA_RegWrite( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : msb reg, pBuf[1] : lsb reg, pBuf[2] : msb data, pBuf[3] : lsb data
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t)*4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pBuf[0];
	uint8_t lsbReg	= pBuf[1];
	uint8_t msbData	= pBuf[2];
	uint8_t lsbData	= pBuf[3];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	return 0;
ERROR_PFPGA:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	PFPGA_RegRead( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : msb reg, pBuf[1] : lsb reg
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pBuf[0];
	uint8_t lsbReg	= pBuf[1];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_PFPGA;
	}

	pTemp[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	pTemp[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	pTemp[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	pTemp[3] = (uint8_t)((iReadData >>  0) & 0xFF);

	return 0;

ERROR_PFPGA:

	return -1;
}

//------------------------------------------------------------------------------
// uint8_t* CNX_I2CController::PFPGA_RegBurstWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, uint32_t nSize )
// {
// 	UNUSED( cmd );
// 	UNUSED( pBuf );
// 	UNUSED( nSize );

// 	//
// 	//	Not Implemetation
// 	//

// 	uint8_t pResult = 0xFF;
// 	int32_t sendSize;

// ERROR_PFPGA:
// 	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &pResult, sizeof(pResult), m_SendBuf, sizeof(m_SendBuf) );

// 	write( fd, m_SendBuf, sendSize );
// 	// NX_HexDump( m_SendBuf, sendSize );

// 	return 0;
// }

//------------------------------------------------------------------------------
int32_t	PFPGA_Status( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	//	pBuf : not used
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_CHECK_STATUS );
		goto ERROR_PFPGA;
	}

	if(PFPGA_VAL_CHECK_STATUS == iReadData)
	{
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x01;
	}
	else
	{
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x00;
	}

#endif

	return 0;
ERROR_PFPGA:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	PFPGA_UniformityData( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	//
	//	pBuf[0] - : data
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t *data	= pBuf;
	uint32_t size	= nSize;

	uint32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	for( uint32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(data[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(data[2 * i + 1] << 0) & 0x00FF);
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0080) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0080 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, PFPGA_REG_NUC_WDATA, iDataSize / 4 );
			goto ERROR_PFPGA;
		}
	}
#else
	if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, PFPGA_REG_NUC_WDATA, iDataSize );
		goto ERROR_PFPGA;
	}
#endif

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

	free( pData );
	return 0;

ERROR_PFPGA:

	free( pData );

	return -1;
}

//------------------------------------------------------------------------------
int32_t	PFPGA_Mute( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( nSize );

	//
	//	pBuf[0] : data
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;


#if FAKE_DATA
	pTemp[0] = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t data	= pBuf[0];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_MUTE, data ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_MUTE, data );
		goto ERROR_PFPGA;
	}

	pTemp[0] = 0x00;
	pTemp[1] = 0x00;
	pTemp[2] = 0x00;
	pTemp[3] = 0x01;

#endif

	return 0;

ERROR_PFPGA:

	return -1;
}

//------------------------------------------------------------------------------
int32_t	PFPGA_Version( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	//	pBuf : not used
	//

	*pResultSize = 4;
	*pResult = (uint8_t*) malloc(sizeof(uint8_t) * 4 );
	uint8_t* pTemp = *pResult;
	pTemp[0] = 0xFF;
	pTemp[1] = 0xFF;
	pTemp[2] = 0xFF;
	pTemp[3] = 0xFF;


	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_PF_VERSION )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_PF_VERSION );
		goto ERROR_PFPGA;
	}

	pTemp[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	pTemp[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	pTemp[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	pTemp[3] = (uint8_t)((iReadData >>  0) & 0xFF);

	return 0;

ERROR_PFPGA:

	return -1;
}



//------------------------------------------------------------------------------
//
//	Platform Commands
//

//------------------------------------------------------------------------------
//dont use I2C
int32_t	PLAT_NapVersion( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	// pBuf : not used
	//

	*pResultSize = strlen(NX_VERSION_NAP) + 1;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*pResultSize) );
	memset( *pResult, 0x00, *pResultSize );
	memcpy( *pResult,  (uint8_t*)NX_VERSION_NAP, strlen(NX_VERSION_NAP) );

	return 0;
}

//------------------------------------------------------------------------------
//dont use I2C
int32_t	PLAT_SapVersion( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	// pBuf : not used
	//

	*pResultSize = strlen(NX_VERSION_SAP) + 1;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*pResultSize) );
	memset( *pResult, 0x00, *pResultSize );
	memcpy( *pResult,  (uint8_t*)NX_VERSION_SAP, strlen(NX_VERSION_SAP) );

	return 0;
}

//------------------------------------------------------------------------------
//dont use I2C
int32_t	PLAT_IpcServerVersion( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	// pBuf : not used
	//

	*pResultSize = strlen(NX_VERSION_IPC_SERVER) + 1;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*pResultSize) );
	memset( *pResult, 0x00, *pResultSize );
	memcpy( *pResult,  (uint8_t*)NX_VERSION_IPC_SERVER, strlen(NX_VERSION_IPC_SERVER) );

	return 0;
}

//------------------------------------------------------------------------------
//dont use I2C
int32_t	PLAT_IpcClientVersion( uint32_t cmd, uint8_t *pBuf, uint32_t nSize , uint8_t** pResult , uint32_t* pResultSize)
{
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( nSize );

	//
	// pBuf : not used
	//

	*pResultSize = strlen(NX_VERSION_IPC_CLIENT) + 1;
	*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*pResultSize) );
	memset( *pResult, 0x00, *pResultSize );
	memcpy( *pResult,  (uint8_t*)NX_VERSION_IPC_CLIENT, strlen(NX_VERSION_IPC_CLIENT) );

	return 0;
}





//------------------------------------------------------------------------------
//
//	Test Pattern Functions
//

//------------------------------------------------------------------------------
static int32_t TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	2901,	2171,	100		},	//	Red-1
		{	10,		0,		1024,	0,		2160,	2417,	3493,	1222	},	//	Green-1
		{	10,		0,		1024,	0,		2160,	2014,	1416,	3816	},	//	Blue-1
		{	10,		0,		1024,	0,		2160,	2911,	3618,	3890	},	//	Cyan-1
		{	10,		0,		1024,	0,		2160,	3289,	2421,	3814	},	//	Magenta-1
		{	10,		0,		1024,	0,		2160,	3494,	3853,	1221	},	//	Yellow-1
		{	10,		0,		1024,	0,		2160,	2738,	2171,	1233	},	//	Red-2
		{	10,		0,		1024,	0,		2160,	2767,	3493,	2325	},	//	Green-2
		{	10,		0,		1024,	0,		2160,	1800,	1416,	3203	},	//	Blue-2
		{	10,		0,		1024,	0,		2160,	3085,	3590,	3756	},	//	Cyan-2
		{	10,		0,		1024,	0,		2160,	3062,	2421,	3497	},	//	Magenta-2
		{	10,		0,		1024,	0,		2160,	3461,	3777,	2065	},	//	Yellow-2
		{	10,		0,		1024,	0,		2160,	3883,	3960,	4092	},	//	White-1
		{	10,		0,		1024,	0,		2160,	3794,	3960,	3890	},	//	White-2
		{	10,		0,		1024,	0,		2160,	3893,	3960,	3838	},	//	White-3
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1024,	0,		2160,	4095,	4095,	4095	},	// 6 Color Bar
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	White	100%
		{	10,		0,		1024,	0,		2160,	3685,	3685,	3685	},	//	Gray	90%
		{	10,		0,		1024,	0,		2160,	3276,	2948,	2948	},	//	Gray	80%
		{	10,		0,		1024,	0,		2160,	2866,	2866,	2866	},	//	Gray	70%
		{	10,		0,		1024,	0,		2160,	2457,	2457,	2457	},	//	Gray	60%
		{	10,		0,		1024,	0,		2160,	2047,	2047,	2047	},	//	Gray	50%
		{	10,		0,		1024,	0,		2160,	1638,	1638,	1638	},	//	Gray	40%
		{	10,		0,		1024,	0,		2160,	1228,	1228,	1228	},	//	Gray	30%
		{	10,		0,		1024,	0,		2160,	819,	819,	819		},	//	Gray	20%
		{	10,		0,		1024,	0,		2160,	409,	409,	409		},	//	Gray	1c0%
		{	10,		0,		1024,	0,		2160,	0,		0,		0		},	//	Black	0%
		{	10,		0,		1024,	0,		2160,	4095,	0,		0		},	//	Red		100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	0		},	//	Green	100%
		{	10,		0,		1024,	0,		2160,	0,		0,		4095	},	//	Blue	100%
		{	10,		0,		1024,	0,		2160,	4095,	0,		4095	},	//	Magenta	100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	4095	},	//	Cyan	100%
		{	10,		0,		1024,	0,		2160,	4095,	4095,	0		},	//	Yellow 	00%
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28
		{	2,		0,		1024,	0,		2160	},							//	Gray 16-Step
		{	3,		0,		1024,	0,		2160	},							//	Gray 32-Step
		{	4,		0,		1024,	0,		2160	},							//	Gray 64-Step
		{	5,		0,		1024,	0,		2160	},							//	Gray 128-Step
		{	6,		0,		1024,	0,		2160	},							//	Gray 256-Step
		{	7,		0,		1024,	0,		2160	},							//	Gray 512-Step
		{	9,		7,		1024,	0,		2160	},							//	Gray 2048-Step
		{	9,		1,		1024,	0,		2160	},							//	Red 2048-Step
		{	9,		2,		1024,	0,		2160	},							//	Green 2048-Step
		{	9,		4,		1024,	0,		2160	},							//	Blue 2048-Step
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	17,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	104,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}


//------------------------------------------------------------------------------
static void MakeDirectory( const char *pDir )
{
	char buf[1024];
	char *pBuf = buf;

	memcpy( buf, pDir, sizeof(buf) -1 );
	buf[sizeof(buf)-1] = 0x00;

	while( *pBuf )
	{
		if( '/' == *pBuf )
		{
			*pBuf = 0x00;
			if( 0 != access( buf, F_OK ) && (pBuf != buf) )
			{
				printf("Make Directory. ( %s )\n", buf);
				mkdir( buf, 0777 );
			}
			*pBuf = '/';
		}
		pBuf++;
	}

	if( 0 != access( buf, F_OK) )
	{
		printf("Make Directory. ( %s )\n", buf);
		mkdir( buf, 0777 );
	}
}
