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

#ifndef __CNX_CINEMABEHAVIOR_H__
#define __CNX_CINEMABEHAVIOR_H__

#include <stdint.h>

typedef struct NX_BEHAVIOR_INFO {
	int32_t 	iMode;					// Mode Number
	float		fVersion;				// Verison Information
	char*		pDesc;					// Description
	char*		pInput;					// XYZ, RGB
	int32_t		iLuminance;				// 48 - 500, 1000
	char*		pGamut;					// SRGB, SRGB_VIVID, P3, P3_VIVID, BYPASS, MANUAL
	uint16_t*	pGamutData;				// 16bit data ( 9EA )
	int32_t		iCopyMode;				// 0 - 9
	int32_t		iRefMode;				// 0 - 9
	int32_t		iCurMode;				// 0 - 9
	int32_t		iRefLuminance;			// 48 - 500

	char*		pTrans;					// Gxxx, PQxxx, Uxxx
	char*		pTransLoad;				// TCON_USB, TCON_DCI, USB
	char*		pTransColor;			// R, G, B
	int32_t		iTransNum;				// 4096
	uint16_t*	pTransTable;			// 16bit Data
} NX_BEHAVIOR_INFO;

typedef struct NX_TREG_INFO {
	int32_t		iMode;
	int32_t 	iGammaType[4];
	char*		pDesc;
	int32_t		iNumber;
	uint16_t*	pReg;
	uint16_t*	pData;
} NX_TREG_INFO;

class CNX_CinemaBehavior
{
public:
	CNX_CinemaBehavior();
	~CNX_CinemaBehavior();

public:
	int32_t	Parse( uint8_t *pInBuf, int32_t iInSize );
	int32_t	Parse( const char *pFormat, ... );
	int32_t ParseTreg( const char *pInFile, int32_t iMode, NX_TREG_INFO *pInfo );

	int32_t	MakeBehavior( const char *pFormat, ... );
	int32_t MakeTreg( const char *pFormat, ... );
	int32_t MakeGamma( const char *pFormat, ... );

	int32_t GetBehaviorInfo( NX_BEHAVIOR_INFO **ppInfo );

private:
	int32_t	ParseBehavior( uint8_t *pInBuf, int32_t iInSize );
	void	FreeBehavior( void );

	int32_t SetBehaviorInfo( FILE *hFile, char *pIndex, char *pData );
	void	FreeBehaviorInfo();
	void	DumpBehaviorInfo();

	int32_t SetTregInfo();
	void	FreeTregInfo();
	void	DumpTregInfo();

	int32_t CalulateMatrix( uint16_t iSrcData[3][3], uint16_t iDstData[3][3] );

	int32_t	IsRemark( char *pString );
	int32_t IsVivid( void );
	int32_t IsBypass( void );
	int32_t IsManual( void );

	void	DumpArray( uint16_t iArray[3][3], const char *pFormat, ... );
	void	DumpArray( uint32_t iArray[3][3], const char *pFormat, ... );
	void	DumpArray( double	fArray[3][3], const char *pFormat, ... );
	void	DumpArray( uint32_t iArray[3], const char *pFormat, ... );

private:
	enum { MAX_REF_NUM = 10 };
	enum { MAX_TREG_NUM = 30, NUM_TREG_INDEX = 1, NUM_TREG_GAMMA = 4, NUM_TREG_DESC = 1, NUM_TREG_NUMBER = 1 };

	NX_BEHAVIOR_INFO	m_BehaviorInfo;
	NX_TREG_INFO		m_TregInfo;

	uint8_t*	m_pBuf;
	int32_t		m_iBufSize;

private:
	CNX_CinemaBehavior (const CNX_CinemaBehavior &Ref);
	CNX_CinemaBehavior &operator=(const CNX_CinemaBehavior &Ref);
};

#endif	// __CNX_CINEMABEHAVIOR_H__
