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

#ifndef __CNX_PIXELCORRECTION_H__
#define __CNX_PIXELCORRECTION_H__

#include <stdint.h>

class CNX_PixelCorrection
{
public:
	CNX_PixelCorrection();
	~CNX_PixelCorrection();

public:
	enum { MAX_DATA_NUM = 64 * 60, MAX_REG_NUM = 9 };

	int32_t		Parse( const char *pFile );
	int32_t		Make( const char *pPath, uint8_t iSlave, int32_t iModule, uint16_t *pData, int32_t iDataSize );

	uint8_t		GetSlave();
	int32_t		GetModule();

	int32_t		GetDataNum();
	uint16_t*	GetData();

	int32_t		GetDataReorderNum();
	uint16_t*	GetDataReorder();

private:
	int32_t		Init();
	void		Deinit();

private:
	uint8_t		m_iSlave;
	int32_t		m_iModule;

	uint16_t*	m_pData;
	uint16_t*	m_pReorder;

private:
	CNX_PixelCorrection (const CNX_PixelCorrection &Ref);
	CNX_PixelCorrection &operator=(const CNX_PixelCorrection &Ref);
};

#endif	// __CNX_PIXELCORRECTION_H__
