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

#ifndef __CNX_UNIFORMITY_H__
#define __CNX_UNIFORMITY_H__

#include <stdint.h>

class CNX_Uniformity
{
public:
	CNX_Uniformity();
	~CNX_Uniformity();

public:
	enum { MAX_DATA_NUM = 4096 };

	int32_t		Parse( const char *pFile );
	uint16_t*	GetData();

private:
	int32_t		Init();
	void		Deinit();

private:
	uint16_t*	m_pData;

private:
	CNX_Uniformity (const CNX_Uniformity &Ref);
	CNX_Uniformity &operator=(const CNX_Uniformity &Ref);
};

#endif	// __CNX_UNIFORMITY_H__
