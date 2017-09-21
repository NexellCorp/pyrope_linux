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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <CNX_EEPRom.h>
#include <CNX_GpioControl.h>

#define EEPROM_DEVICE			"/dev/i2c-8"
#define EEPROM_SLAVE			0x50
#define EEPROM_WRITE_CONTROL	GPIOC31			// Write Protection

//------------------------------------------------------------------------------
CNX_EEPRom::CNX_EEPRom()
	: m_hDev( -1 )
{
	m_hDev = open( EEPROM_DEVICE, O_RDWR, 644 );
}

//------------------------------------------------------------------------------
CNX_EEPRom::~CNX_EEPRom()
{
	if( 0 < m_hDev )
		close( m_hDev );
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Read()
{
	int32_t iRet = 0;
	uint8_t data = 0x00;

	iRet = RawRead( EEPROM_SLAVE, &data, sizeof(data) );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	return 0x000000FF & data;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Read( uint32_t iAddr )
{
	int32_t iRet = 0;
	uint8_t slave;
	uint8_t addr[2] = { 0x00, };
	uint8_t data = 0x00;

	if( !IsValidAddress(iAddr) )
	{
		printf("Fail, Invalid Address. ( 0x%08X )\n", iAddr );
		return -1;
	}

	slave   = EEPROM_SLAVE | ((iAddr >> 16) & 0x01);
	addr[0] = (iAddr >> 8) & 0xFF;
	addr[1] = (iAddr >> 0) & 0xFF;

	iRet = RawWrite( slave, addr, sizeof(addr) / sizeof(addr[0]) );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	iRet = RawRead( slave, &data, sizeof(data) );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	return 0x000000FF & data;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Read( uint8_t *pBuf, int32_t iSize )
{
	int32_t iRet = 0;

	iRet = RawRead( EEPROM_SLAVE, pBuf, iSize );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Read( uint32_t iAddr, uint8_t *pBuf, int32_t iSize )
{
	int32_t iRet = 0;
	uint8_t slave;
	uint8_t addr[2] = { 0x00, };

	if( !IsValidAddress(iAddr) )
	{
		printf("Fail, Invalid Address. ( 0x%08X )\n", iAddr );
		return -1;
	}

	slave   = EEPROM_SLAVE | ((iAddr >> 16) & 0x01);
	addr[0] = (iAddr >> 8) & 0xFF;
	addr[1] = (iAddr >> 0) & 0xFF;

	iRet = RawWrite( slave, addr, sizeof(addr) / sizeof(addr[0]) );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	iRet = RawRead( slave, pBuf, iSize );
	if( 0 > iRet )
	{
		printf("Fail, Read().\n");
		return iRet;
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Write( uint32_t iAddr, uint8_t iBuf )
{
	int32_t iRet = 0;
	uint8_t slave;
	uint8_t raw[3] = { 0x00, };

	if( !IsValidAddress(iAddr) )
	{
		printf("Fail, Invalid Address. ( 0x%08X )\n", iAddr );
		return -1;
	}

	slave  = EEPROM_SLAVE | ((iAddr >> 16) & 0x01);
	raw[0] = (iAddr >> 8) & 0xFF;
	raw[1] = (iAddr >> 0) & 0xFF;
	raw[2] = iBuf;

	iRet = RawWrite( slave, raw, sizeof(raw) / sizeof(raw[0]) );
	if( 0 > iRet )
	{
		printf("Fail, Write().\n");
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::Write( uint32_t iAddr, uint8_t *pBuf, int32_t iSize )
{
	int32_t iRet = 0;
	uint8_t slave;

	if( !IsValidAddress(iAddr) )
	{
		printf("Fail, Invalid Address. ( 0x%08X )\n", iAddr );
		return -1;
	}

	if( (iSize <= 0) || (iSize > MAX_PAGE_SIZE) )
	{
		printf("Fail, Invalid Size. ( Cur: %d, Max : %d )\n", iSize, MAX_PAGE_SIZE );
		return -1;
	}

	int32_t iRawSize = 2 + iSize;
	uint8_t *pRaw = (uint8_t*)malloc( iRawSize );

	slave   = EEPROM_SLAVE | ((iAddr >> 16) & 0x01);
	pRaw[0] = (iAddr >> 8) & 0xFF;
	pRaw[1] = (iAddr >> 0) & 0xFF;
	memcpy( pRaw + 2, pBuf, iSize );

	iRet = RawWrite( slave, pRaw, iRawSize );
	if( 0 > iRet )
	{
		printf("Fail, Write().\n");
	}

	if( pRaw ) free( pRaw );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::WriteProtection( int32_t bEnable )
{
	CNX_GpioControl gpio;
	gpio.Init( EEPROM_WRITE_CONTROL );
	gpio.SetDirection( GPIO_DIRECTION_OUT );
	gpio.SetValue( bEnable );
	gpio.Deinit();
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::RawWrite( uint8_t iSlave, uint8_t *pBuf, int32_t iSize )
{
	if( !IsValidFd() )
		return -1;

	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[1];

	rdwr.msgs = msgs;
	rdwr.nmsgs = 1;

	rdwr.msgs[0].addr  = iSlave;
	rdwr.msgs[0].flags = !I2C_M_RD;
	rdwr.msgs[0].len   = iSize;
	rdwr.msgs[0].buf   = pBuf;

	return ioctl(m_hDev, I2C_RDWR, &rdwr);
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::RawRead( uint8_t iSlave, uint8_t *pBuf, int32_t iSize )
{
	if( !IsValidFd() )
		return -1;

	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[1];

	rdwr.msgs = msgs;
	rdwr.nmsgs = 1;

	rdwr.msgs[0].addr  = iSlave;
	rdwr.msgs[0].flags = I2C_M_RD;
	rdwr.msgs[0].len   = iSize;
	rdwr.msgs[0].buf   = pBuf;

	return ioctl(m_hDev, I2C_RDWR, &rdwr);

}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::IsValidFd( void )
{
	if( 0 == (fcntl(m_hDev, F_GETFL) != -1 || errno != EBADF) )
	{
		printf("fail, file descriptor. ( %d )\n", m_hDev);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_EEPRom::IsValidAddress( uint32_t iAddr )
{
	return (iAddr <= 0x1FFFF) ? true : false;
}
