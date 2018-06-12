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

#ifndef __CNX_PREG_H__
#define __CNX_PREG_H__

#include <stdint.h>

typedef struct NX_PREG_INFO {
	int32_t		iEnable;
	int32_t		iNumber;
	uint16_t*	pReg;
	uint16_t*	pData;
} NX_PREG_INFO;

class CNX_PReg
{
public:
	CNX_PReg();
	~CNX_PReg();

public:
	enum { MAX_INFO_NUM = 20 };

	int32_t		Parse( const char *pFile );
	int32_t		Update( const char *pFile );
	int32_t		Update( const char *pFile, int32_t iMode );
	int32_t		Delete();
	int32_t		Delete( int32_t iMdoe );
	int32_t		Make( const char *pFile );

	int32_t		Dump();

	int32_t		IsValid( int32_t iMode );
	int32_t		GetEnable( int32_t iMode );
	int32_t		GetDataNum( int32_t iMode );
	uint16_t*	GetRegister( int32_t iMode );
	uint16_t*	GetData( int32_t iMode );

private:
	enum { NUM_INDEX = 1, NUM_ENABLE = 1, NUM_NUMBER = 1 };
	NX_PREG_INFO*	m_pInfo[MAX_INFO_NUM];

private:
	CNX_PReg (const CNX_PReg &Ref);
	CNX_PReg &operator=(const CNX_PReg &Ref);
};

#endif	// __CNX_PREG_H__
