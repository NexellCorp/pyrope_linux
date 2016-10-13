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

#include <CNX_I2C.h>

//------------------------------------------------------------------------------
CNX_I2C::CNX_I2C( int32_t iPort )
	: m_hDev( -1 )
	, m_iPort( iPort )
{
}

//------------------------------------------------------------------------------
CNX_I2C::~CNX_I2C()
{
	Close();
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::Open( void )
{
	char devName[64];
	sprintf( devName, "/dev/i2c-%d", m_iPort );
	m_hDev = open( devName, O_RDWR, 644 );

	if( 0 > m_hDev )
	{
		printf("fail, open device. ( %s )\n", devName);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
void CNX_I2C::Close( void )
{
	if( 0 < m_hDev && IsValidFd() )
	{
		close( m_hDev );
		m_hDev = -1;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::Write( uint8_t iSlave )
{
	int32_t iRet = 0;

	iRet = RawWrite( iSlave, NULL, 0 );
	if( 0 > iRet )
	{
		printf("fail, [wr] i2c-%d, slave:0x%02x, addr:NULL, size:0\n", m_iPort, iSlave );
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::Write( uint8_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize )
{
	int32_t iRet = 0;
	int32_t iRawSize = 2 + iSize * 2;
	uint8_t *pRawData = (uint8_t*)malloc( iRawSize );

	pRawData[0] = (iAddr >> 8) & 0xFF;
	pRawData[1] = (iAddr >> 0) & 0xFF;

	memcpy( pRawData + 2, (uint8_t*)pBuf, iRawSize - 2 );

	iRet = RawWrite( iSlave, pRawData, iRawSize );
	if( 0 > iRet )
	{
		printf("fail, [wr] i2c-%d, slave:0x%02x, addr:0x%02x, size:%d\n", m_iPort, iSlave, iAddr, iSize);
	}

	if( pRawData ) free( pRawData );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::Read( uint8_t iSlave, uint16_t iAddr )
{
	int32_t iRet = 0;
	uint8_t addr[2] = { 0x00, };
	uint8_t data[2] = { 0x00, };

	addr[0] = (iAddr >> 8) & 0xFF;
	addr[1] = (iAddr >> 0) & 0xFF;

	iRet = RawWrite( iSlave, addr, 2 );
	if( 0 > iRet )
	{
		printf("fail, [wr] i2c-%d, slave:0x%02x, addr:0x%02x, size:1\n", m_iPort, iSlave, iAddr );
		return iRet;
	}

	iRet = RawRead( iSlave, data, sizeof(data) );
	if( 0 > iRet ) 
	{
		printf("fail, [rd] i2c-%d, slave:0x%02x, addr:0x%02x, size:1\n", m_iPort, iSlave, iAddr );
		return iRet;
	}

	return (data[0] << 8 & 0xFF00) | (data[1] << 0 & 0x00FF);
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::Read( uint8_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize )
{
	int32_t iRet = 0;
	uint8_t addr[2] = { 0x00, };

	addr[0] = (iAddr >> 8) & 0xFF;
	addr[1] = (iAddr >> 0) & 0xFF;

	iRet = RawWrite( iSlave, addr, 2 );
	if( 0 > iRet )
	{
		printf("fail, [wr] i2c-%d, slave:0x%02x, addr:0x%02x, size:1\n", m_iPort, iSlave, iAddr );
		return iRet;
	}

	iRet = RawRead( iSlave, (uint8_t*)pBuf, iSize * 2 );
	if( 0 > iRet )
	{
		printf("fail, [rd] i2c-%d, slave:0x%02x, addr:0x%02x, size:%d\n", m_iPort, iSlave, iAddr, iSize );
		return iRet;
	}

	for( int32_t i = 0; i < iSize; i++ )
	{
		uint8_t iUpper = (pBuf[i] >> 0) & 0xFF;
		uint8_t iLower = (pBuf[i] >> 8) & 0xFF;

		pBuf[i] = (iUpper << 8 & 0xFF00) | (iLower << 0 & 0x00FF);
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_I2C::RawWrite( uint8_t iSlave, uint8_t *pBuf, int32_t iSize )
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
int32_t CNX_I2C::RawRead( uint8_t iSlave, uint8_t *pBuf, int32_t iSize )
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
int32_t CNX_I2C::IsValidFd( void )
{
	if( 0 == (fcntl(m_hDev, F_GETFL) != -1 || errno != EBADF) )
	{
		printf("fail, file descriptor. ( %d )\n", m_hDev);
		return false;
	}

	return true;
}
