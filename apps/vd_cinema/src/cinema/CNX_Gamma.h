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

#ifndef __CNX_GAMMA_H__
#define __CNX_GAMMA_H__

#include <stdint.h>

class CNX_Gamma
{
public:
	CNX_Gamma();
	~CNX_Gamma();

public:
	enum { MAX_DATA_NUM = 4096 };
	enum { TYPE_TARGET = 0, TYPE_DEVICE = 1 };
	enum { TABLE_LUT0 = 0, TABLE_LUT1 = 1 };
	enum { CHANNEL_RED = 0, CHANNEL_GREEN = 1, CHANNEL_BLUE = 2 };

	int32_t		Parse( const char *pFile );

	int32_t		GetType();
	int32_t		GetTable();
	int32_t		GetChannel();
	uint32_t*	GetData();

private:
	int32_t		Init();
	void		Deinit();

private:
	int32_t		m_iType;
	int32_t		m_iTable;
	int32_t		m_iChannel;
	uint32_t*	m_pData;

private:
	CNX_Gamma (const CNX_Gamma &Ref);
	CNX_Gamma &operator=(const CNX_Gamma &Ref);
};

#endif	// __CNX_GAMMA_H__
