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

#ifndef __CNX_I2C_H__
#define __CNX_I2C_H__

#include <stdint.h>

class CNX_I2C 
{
public:
	CNX_I2C( int32_t iPort );
	~CNX_I2C();

	int32_t Open();
	void Close();

	int32_t Write( uint8_t iSlave );
	int32_t Write( uint8_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize );

	int32_t Read( uint8_t iSlave, uint16_t iAddr );
	int32_t Read( uint8_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize );

private:
	int32_t RawWrite( uint8_t iSlave, uint8_t *pBuf, int32_t iSize );
	int32_t RawRead( uint8_t iSlave, uint8_t *pBuf, int32_t iSize );

	int32_t IsValidFd( void );

private:
	int32_t		m_hDev;
	int32_t		m_iPort;
};

#endif	// __CNX_I2C_H__
