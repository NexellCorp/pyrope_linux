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

#ifndef __CNX_TREG_H__
#define __CNX_TREG_H__

#include <stdint.h>

typedef struct NX_TREG_INFO {
	int32_t 	iGammaType[4];
	char*		pDesc;
	int32_t		iNumber;
	uint16_t*	pReg;
	uint16_t*	pData;
} NX_TREG_INFO;

class CNX_TReg
{
public:
	CNX_TReg();
	~CNX_TReg();

public:
	enum { MAX_INFO_NUM = 20, MAX_GAMMA_NUM = 4 };

	int32_t 	Parse( const char *pFile );
	int32_t 	Update( const char *pFile );
	int32_t 	Update( const char *pFile, int32_t iMode );
	int32_t 	Delete();
	int32_t 	Delete( int32_t iMdoe );
	int32_t 	Make( const char *pFile );

	int32_t		Dump();

	int32_t		IsValid( int32_t iMode );
	char*		GetDescriptor( int32_t iMode );
	int32_t*	GetGammaType( int32_t iMode );		// 0: disable, 1: enable
	int32_t		GetDataNum( int32_t iMode );
	uint16_t*	GetRegister( int32_t iMode );
	uint16_t*	GetData( int32_t iMode );

private:
	enum { NUM_INDEX = 1, NUM_GAMMA = 4, NUM_DESC = 1, NUM_NUMBER = 1 };
	NX_TREG_INFO*	m_pInfo[MAX_INFO_NUM];

private:
	CNX_TReg (const CNX_TReg &Ref);
	CNX_TReg &operator=(const CNX_TReg &Ref);
};

#endif	// __CNX_TREG_H__
