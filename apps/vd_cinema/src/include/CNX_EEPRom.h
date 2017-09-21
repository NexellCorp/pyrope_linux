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

#ifndef __CNX_EEPROM_H__
#define __CNX_EEPROM_H__

#include <stdint.h>

//
//	Address Range	: 0x00000 ~ 0x1FFFF ( 17bit )
//
class CNX_EEPRom
{
public:
	CNX_EEPRom();
	~CNX_EEPRom();

public:
	//
	//	Error is -1, Successful is 8bit data.
	//
	int32_t Read();													// Current Address Read
	int32_t Read( uint32_t iAddr );									// Random Address Read

	//878
	//	Error is -1, Successful is 0.
	//	pBuf is Sequential Read Data.
	//
	int32_t Read( uint8_t *pBuf, int32_t iSize );					// Sequential Current Address Read
	int32_t Read( uint32_t iAddr, uint8_t *pBuf, int32_t iSize );	// Sequential Random Address Read

	//
	//	Error is -1, Successful is 0.
	//
	int32_t Write( uint32_t iAddr, uint8_t iBuf );
	int32_t Write( uint32_t iAddr, uint8_t *pBuf, int32_t iSize );	//

	//
	//	\WC is 0 : data write enabled
	//	\WC is 1 : data write inhibited
	//
	int32_t WriteProtection( int32_t bEnable );

private:
	enum { MAX_PAGE_SIZE = 256 };

	int32_t RawWrite( uint8_t iSlave, uint8_t *pBuf, int32_t iSize );
	int32_t RawRead( uint8_t iSlave, uint8_t *pBuf, int32_t iSize );

	int32_t IsValidFd( void );
	int32_t IsValidAddress( uint32_t iAddr );

private:
	int32_t m_hDev;

private:
	CNX_EEPRom (const CNX_EEPRom &Ref);
	CNX_EEPRom &operator=(const CNX_EEPRom &Ref);
};

#endif	// __CNX_EEPROM_H__
