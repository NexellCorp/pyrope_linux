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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "CNX_CinemaBehavior.h"
#include "NX_String.h"

#define NX_DTAG	"[CNX_CinemaBehavior]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
#define NX_ENABLE_INFO			false
#define NX_ENABLE_DUMP			false

#ifdef ANDROID
#include "fmemopen.h"
#define NX_TREG_EEPROM_FILE		"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.txt"
#else
#define NX_TREG_EEPROM_FILE		"./T_REG.txt"
#endif

//------------------------------------------------------------------------------
CNX_CinemaBehavior::CNX_CinemaBehavior()
	: m_pBuf( NULL )
	, m_iBufSize( 0 )
{
	memset( &m_BehaviorInfo, 0x00, sizeof(m_BehaviorInfo) );
	memset( &m_TregInfo, 0x00, sizeof(m_TregInfo) );

	FreeBehaviorInfo();
}

//------------------------------------------------------------------------------
CNX_CinemaBehavior::~CNX_CinemaBehavior()
{
	FreeTregInfo();
	FreeBehaviorInfo();
	FreeBehavior();
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::Parse( uint8_t *pInBuf, int32_t iInSize )
{
	FreeBehavior();

	m_pBuf = (uint8_t*)malloc( iInSize );
	if( NULL == m_pBuf )
	{
		printf("Fail, malloc(). ( size: %d )\n", iInSize );
		return -1;
	}

	memcpy( m_pBuf, pInBuf, iInSize );
	m_iBufSize = iInSize;

	if( 0 > ParseBehavior( m_pBuf, m_iBufSize ) )
	{
		FreeBehavior();
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::Parse( const char *pFormat, ... )
{
	FreeBehavior();

	va_list args;
	char szName[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szName, sizeof(szName), pFormat, args);
	va_end(args);

	FILE *hFile = fopen( szName, "rb" );
	if( NULL == hFile )
	{
		printf("Fail, fopen(). ( %s )\n", szName);
		return -1;
	}

	fseek( hFile, 0, SEEK_END );
	m_iBufSize = ftell( hFile );
	m_pBuf     = (uint8_t*)malloc( m_iBufSize );
	if( NULL == m_pBuf )
	{
		printf("Fail, malloc(). ( size: %d )\n", m_iBufSize );
		return -1;
	}

	fseek( hFile, 0, SEEK_SET );

	int32_t iReadSize = fread( m_pBuf, 1, m_iBufSize, hFile );
	if( iReadSize != m_iBufSize )
	{
		printf("Fail, fread(). ( expected: %d, read: %d )\n", m_iBufSize, iReadSize );
		fclose( hFile );
		return -1;
	}

	fclose( hFile );

	if( 0 > ParseBehavior( m_pBuf, m_iBufSize ) )
	{
		FreeBehavior();
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::MakeBehavior( const char *pFormat, ... )
{
	va_list args;
	char szName[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szName, sizeof(szName), pFormat, args);
	va_end(args);

	if( NULL == m_pBuf )
	{
		printf("Fail, Parse().\n");
		return -1;
	}

	FILE* hFile = fopen( szName, "wb" );
	if( NULL == hFile )
	{
		printf("Fail, fopen(). ( %s )\n", szName );
		return -1;
	}

	int32_t iWriteSize = fwrite( m_pBuf, 1, m_iBufSize, hFile );
	if( iWriteSize != m_iBufSize )
	{
		printf("Fail, fwrite(). ( expected: %d, write: %d )\n", m_iBufSize, iWriteSize );
		fclose( hFile );
		return -1;
	}
	fclose( hFile );

	NxDbgMsg( NX_DBG_INFO, "Make Behavior. ( %s )\n", szName );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::MakeTreg( const char *pFormat, ... )
{
	va_list args;
	char szName[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szName, sizeof(szName), pFormat, args);
	va_end(args);

	FILE *hFile = fopen( szName, "wb" );
	if( NULL == hFile )
	{
		printf("Fail, fopen(). ( %s )\n", szName );
		return -1;
	}

	int32_t iLineCnt = 0;
	int32_t iDataCnt = 0;
	int32_t bRun = true;

	while( bRun )
	{
		char szData[128];

		if( iLineCnt < NUM_TREG_INDEX )
		{
			snprintf( szData, sizeof(szData), "Mode%d\r\n", m_TregInfo.iMode );
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA )
		{
			snprintf( szData, sizeof(szData), "%d\r\n", m_TregInfo.iGammaType[iLineCnt-NUM_TREG_INDEX] );
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA + NUM_TREG_DESC )
		{
			snprintf( szData, sizeof(szData), "%s\r\n", m_TregInfo.pDesc );
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA + NUM_TREG_DESC + NUM_TREG_NUMBER )
		{
			snprintf( szData, sizeof(szData), "%d\r\n", m_TregInfo.iNumber );
		}
		else
		{
			iDataCnt = (iLineCnt - NUM_TREG_INDEX - NUM_TREG_GAMMA - NUM_TREG_DESC - NUM_TREG_NUMBER);

			if( (iDataCnt/2) == m_TregInfo.iNumber )
			{
				bRun = false;
				break;
			}

			if( !(iDataCnt % 2) )
			{
				snprintf( szData, sizeof(szData), "0x%X\r\n", m_TregInfo.pReg[iDataCnt/2] );
			}
			else
			{
				snprintf( szData, sizeof(szData), "%d\r\n", m_TregInfo.pData[iDataCnt/2] );
			}
		}

		fwrite( szData, 1, strlen(szData), hFile );
		iLineCnt++;
	}
	fclose( hFile );

	NxDbgMsg( NX_DBG_INFO, "Make TReg. ( %s )\n", szName );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::MakeGamma( const char *pFormat, ... )
{
	va_list args;
	char szName[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szName, sizeof(szName), pFormat, args);
	va_end(args);

	FILE *hFile = fopen( szName, "wb" );
	if( NULL == hFile )
	{
		printf("Fail, fopen(). ( %s )\n", szName );
		return -1;
	}

	for( int32_t i = 0; i < m_BehaviorInfo.iTransNum; i++ )
	{
		char szData[1024];
		snprintf( szData, sizeof(szData), "%d\r\n", m_BehaviorInfo.pTransTable[i] );

		int32_t iWriteSize = fwrite( szData, 1, strlen(szData), hFile );
		if( iWriteSize != (int32_t)strlen(szData) )
		{
			printf("Fail, fwrite(). ( expected: %d, write: %d )\n", (int32_t)strlen(szData), iWriteSize );
			fclose( hFile );
			return -1;
		}
	}
	fclose( hFile );

	NxDbgMsg( NX_DBG_INFO, "Make Gamma. ( %s )\n", szName );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::GetBehaviorInfo( NX_BEHAVIOR_INFO **ppInfo )
{
	*ppInfo = &m_BehaviorInfo;
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::ParseBehavior( uint8_t *pInBuf, int32_t iInSize )
{
	FILE *hFile = fmemopen( pInBuf, iInSize, "rb" );
	if( NULL == hFile )
	{
		printf("Fail, fmemopen(). ( buf: %p, size: %d )\n", pInBuf, iInSize );
		return -1;
	}

	FreeBehaviorInfo();
	FreeTregInfo();

	char szLine[1024];
	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		NX_StringTrim( szLine );

		if( IsRemark(szLine) )
		{
			// printf("Skip. Remark.( %s )\n", szLine );
			continue;
		}

		char szIndex[128] = { 0x00, };
		char szData[128] = { 0x00, };

		NX_GetStringParse( szLine, "%s = %s", szIndex, szData );
		NX_StringUpper( szIndex );
		NX_StringUpper( szData );

		SetBehaviorInfo( hFile, szIndex, szData );
	}

	fclose( hFile );
	SetTregInfo();

	DumpBehaviorInfo();
	DumpTregInfo();

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::ParseTreg( const char *pInFile, int32_t iMode, NX_TREG_INFO *pInfo )
{
	FILE *hFile = fopen( pInFile, "rb" );
	if( NULL == hFile )
	{
		printf("Fail, fopen(). ( %s )\n", pInFile );
		return -1;
	}

	char szLine[1024], szMode[256];
	int32_t iModeNum = 0;

	int32_t iLineCnt = 0;
	int32_t iDataCnt = 0;
	int32_t iPosCnt = 0;

	int32_t iDataNum[MAX_TREG_NUM] = { 0, };

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		NX_StringTrim( szLine );

		char szTokenMode[64] = { 0x00, };
		char szTokenData[64] = { 0x00, };
		int32_t iTemp;

		if( iLineCnt < NUM_TREG_INDEX )
		{
			sprintf( szMode, "%s", szLine );
			iModeNum = NX_GetStringNum( szMode );
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetString( szMode, i, szTokenMode );
				NX_StringUpper( szTokenMode );
				NX_GetStringParse( szTokenMode, "MODE%d", &iTemp );

				if( iTemp == iMode )
					break;

				iPosCnt++;
			}

			if( iTemp == iMode )
			{
				pInfo->iMode = iMode;
			}
			else
			{
				// printf( "Fail, Not match mode. ( %d )\n", iMode );
				fclose( hFile );
				return -1;
			}
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA )
		{
			NX_GetString( szLine, iPosCnt, szTokenData );
			pInfo->iGammaType[iLineCnt-NUM_TREG_INDEX] = strtol(szTokenData, NULL, 10);
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA + NUM_TREG_DESC )
		{
			NX_GetString( szLine, iPosCnt, szTokenData );
			pInfo->pDesc = strdup(szTokenData);
		}
		else if( iLineCnt < NUM_TREG_INDEX + NUM_TREG_GAMMA + NUM_TREG_DESC + NUM_TREG_NUMBER )
		{
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetString( szMode, i, szTokenMode );
				NX_StringUpper( szTokenMode );
				NX_GetStringParse( szTokenMode, "MODE%d", &iTemp );

				NX_GetString( szLine, i, szTokenData );
				iDataNum[iTemp] = strtol(szTokenData, NULL, 10);
			}

			NX_GetString( szLine, iPosCnt, szTokenData );
			pInfo->iNumber = strtol(szTokenData, NULL, 10);
			pInfo->pReg    = (uint16_t*)malloc( sizeof(uint16_t)*pInfo->iNumber );
			pInfo->pData   = (uint16_t*)malloc( sizeof(uint16_t)*pInfo->iNumber );
		}
		else
		{
			int32_t iSkipCnt = 0;
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetString( szMode, i, szTokenMode );
				NX_StringUpper( szTokenMode );
				NX_GetStringParse( szTokenMode, "MODE%d", &iTemp );

				if( iDataCnt/2 > iDataNum[iTemp]-1 )
				{
					// printf("iDataCnt = %d, iDataNum[%d] = %d\n", iDataCnt, iTemp, iDataNum[iTemp]);
					iSkipCnt++;
					continue;
				}

				if( iTemp == iMode )
				{
					NX_GetString( szLine, iPosCnt-iSkipCnt, szTokenData );
					if( !(iDataCnt % 2) )	pInfo->pReg[iDataCnt/2]  = strtol(szTokenData, NULL, 16);
					else					pInfo->pData[iDataCnt/2] = strtol(szTokenData, NULL, 10);

					break;
				}
			}

			if( (++iDataCnt) / 2 == pInfo->iNumber )
				break;
		}

		iLineCnt++;
	}

	fclose( hFile );

#if 0
	{
		printf("* mode %d : %s\n", pInfo->iMode, pInfo->pDesc );
		printf("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )\n",
			pInfo->iGammaType[0], pInfo->iGammaType[1], pInfo->iGammaType[2], pInfo->iGammaType[3]);
		printf("-. number of data : %d\n", pInfo->iNumber );
		for( int j = 0; j < pInfo->iNumber; j++ )
			printf("-. data #%3d : reg( %5d, 0x%04x ), data( %5d, 0x%04x )\n",
				j, pInfo->pReg[j], pInfo->pReg[j], pInfo->pData[j], pInfo->pData[j] );
	}
#endif

	return 0;
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::FreeBehavior( void )
{
	if( NULL != m_pBuf )
	{
		free( m_pBuf );
		m_pBuf = NULL;
	}

	m_iBufSize = 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::SetBehaviorInfo( FILE *hFile, char *pIndex, char *pData )
{
	if( !strcasecmp( pIndex,		"MODE" ) )
	{
		m_BehaviorInfo.iMode		= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"VERSION" ) )
	{
		m_BehaviorInfo.fVersion		= strtof( pData, NULL );
	}
	else if( !strcasecmp( pIndex,	"NAME" ) )
	{
		m_BehaviorInfo.pDesc		= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"INPUT" ) )
	{
		m_BehaviorInfo.pInput		= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"LUMINANCE" ) )
	{
		m_BehaviorInfo.iLuminance	= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"GAMUT" ) )
	{
		m_BehaviorInfo.pGamut		= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"COPY_MODE" ) )
	{
		m_BehaviorInfo.iCopyMode	= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"REF_MODE" ) )
	{
		m_BehaviorInfo.iRefMode		= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"CUR_MODE" ) )
	{
		m_BehaviorInfo.iCurMode		= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"REF_LUMINANCE" ) )
	{
		m_BehaviorInfo.iRefLuminance= strtol( pData, NULL, 10 );
	}
	else if( !strcasecmp( pIndex,	"TRANSFER" ) )
	{
		m_BehaviorInfo.pTrans		= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"TRANSFER_LOAD" ) )
	{
		m_BehaviorInfo.pTransLoad	= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"TRANSFER_COLOR" ) )
	{
		m_BehaviorInfo.pTransColor	= strdup( pData );
	}
	else if( !strcasecmp( pIndex,	"TRANSFER_NUM" ) )
	{
		m_BehaviorInfo.iTransNum	= strtol( pData, NULL, 10 );
		m_BehaviorInfo.pTransTable	= (uint16_t*)malloc( sizeof(uint16_t)*m_BehaviorInfo.iTransNum );
	}
	else if( !strcasecmp( pIndex,	"GAMUT_DATA" ) )
	{
		int32_t iDataCnt = 0;
		char szData[32] = { 0x00, };

		while( fgets( szData, sizeof(szData)-1, hFile ) || iDataCnt < 9 )
		{
			m_BehaviorInfo.pGamutData[iDataCnt++] = strtol( szData, NULL, 10 );
		}
	}
	else if( !strcasecmp( pIndex,	"TRANSFER_TABLE" ) )
	{
		int32_t iDataCnt = 0;
		char szData[32] = { 0x00, };

		while( fgets( szData, sizeof(szData)-1, hFile ) || iDataCnt < m_BehaviorInfo.iTransNum )
		{
			m_BehaviorInfo.pTransTable[iDataCnt++] = strtol( szData, NULL, 10 );
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::FreeBehaviorInfo()
{
	if( m_BehaviorInfo.pDesc )			free( m_BehaviorInfo.pDesc );
	if( m_BehaviorInfo.pInput )			free( m_BehaviorInfo.pInput );
	if( m_BehaviorInfo.pGamut )			free( m_BehaviorInfo.pGamut );
	if( m_BehaviorInfo.pTrans )			free( m_BehaviorInfo.pTrans );
	if( m_BehaviorInfo.pTransLoad )		free( m_BehaviorInfo.pTransLoad );
	if( m_BehaviorInfo.pTransColor )	free( m_BehaviorInfo.pTransColor );
	if( m_BehaviorInfo.pTransTable )	free( m_BehaviorInfo.pTransTable );

	memset( &m_BehaviorInfo, 0x00, sizeof(m_BehaviorInfo) );
	m_BehaviorInfo.iMode         = -1;
	m_BehaviorInfo.fVersion      = -1;
	m_BehaviorInfo.iLuminance    = -1;
	m_BehaviorInfo.iCopyMode     = -1;
	m_BehaviorInfo.iRefMode      = -1;
	m_BehaviorInfo.iCurMode      = -1;
	m_BehaviorInfo.iRefLuminance = -1;
	m_BehaviorInfo.iTransNum     = -1;
	m_BehaviorInfo.pGamutData    = (uint16_t*)malloc( sizeof(uint16_t)*9 );
}

//------------------------------------------------------------------------------
// #define NX_DATA_PER_LINE		16

void CNX_CinemaBehavior::DumpBehaviorInfo()
{
#if NX_ENABLE_INFO
	printf("--------------------------------------------------------------------------------\n");
	printf("Behavior Information                                                            \n");
	printf("--------------------------------------------------------------------------------\n");
	printf("iMode         : %d\n", m_BehaviorInfo.iMode );
	printf("fVersion      : %.1f\n", m_BehaviorInfo.fVersion);
	printf("pDesc         : %s\n", m_BehaviorInfo.pDesc );
	printf("pInput        : %s\n", m_BehaviorInfo.pInput );
	printf("iLuminance    : %d\n", m_BehaviorInfo.iLuminance );
	printf("pGamut        : %s\n", m_BehaviorInfo.pGamut );
	printf("iCopyMode     : %d\n", m_BehaviorInfo.iCopyMode );
	printf("iRefMode      : %d\n", m_BehaviorInfo.iRefMode );
	printf("iCurMode      : %d\n", m_BehaviorInfo.iCurMode );
	printf("iRefLuminance : %d\n", m_BehaviorInfo.iRefLuminance );
	printf("pTrans        : %s\n", m_BehaviorInfo.pTrans );
	printf("pTransLoad    : %s\n", m_BehaviorInfo.pTransLoad );
	printf("pTransColor   : %s\n", m_BehaviorInfo.pTransColor );
	printf("iTransNum     : %d\n", m_BehaviorInfo.iTransNum );

#ifdef NX_DATA_PER_LINE
	printf("pGamutData    : \n");
	for( int32_t i = 0; i < 9; i++ )
	{
		printf("%5d\t", m_BehaviorInfo.pGamutData[i]);
		if( (i % NX_DATA_PER_LINE) == NX_DATA_PER_LINE-1 ) printf("\n");
	}
	printf("\n");

	printf("pTransTable    : \n");
	for( int32_t i = 0; i < m_BehaviorInfo.iTransNum; i++ )
	{
		printf("%5d\t", m_BehaviorInfo.pTransTable[i]);
		if( (i % NX_DATA_PER_LINE) == NX_DATA_PER_LINE-1 ) printf("\n");
	}
	printf("\n");
#endif
#endif
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::SetTregInfo()
{
	int32_t iCopyMode = m_BehaviorInfo.iCopyMode;
	int32_t iRefMode  = m_BehaviorInfo.iRefMode;
	int32_t iCurMode  =
		(0 > m_BehaviorInfo.iCurMode) ? m_BehaviorInfo.iRefMode : m_BehaviorInfo.iCurMode;

	int32_t iTransfer = 0;
	if( NULL != m_BehaviorInfo.pTrans )
	{
		if( !strncasecmp( m_BehaviorInfo.pTrans, "G", 1 ) ) iTransfer = 1000 + strtol(m_BehaviorInfo.pTrans+1, NULL, 10);
		if( !strncasecmp( m_BehaviorInfo.pTrans, "P", 1 ) ) iTransfer = 2000 + strtol(m_BehaviorInfo.pTrans+1, NULL, 10);
		if( !strncasecmp( m_BehaviorInfo.pTrans, "U", 1 ) ) iTransfer = 3000 + strtol(m_BehaviorInfo.pTrans+1, NULL, 10);
	}

	{
		NX_TREG_INFO info;

		if( 0 > ParseTreg( NX_TREG_EEPROM_FILE, iCopyMode, &info ) )
		{
			return -1;
		}

		m_TregInfo.iMode         = m_BehaviorInfo.iMode;
		m_TregInfo.iGammaType[0] = 0;
		m_TregInfo.iGammaType[1] = iTransfer;
		m_TregInfo.iGammaType[2] = 0;
		m_TregInfo.iGammaType[3] = 0;
		m_TregInfo.pDesc         = strdup( m_BehaviorInfo.pDesc );
		m_TregInfo.iNumber       = info.iNumber;
		m_TregInfo.pReg          = (uint16_t*)malloc( sizeof(uint16_t)*info.iNumber );
		m_TregInfo.pData         = (uint16_t*)malloc( sizeof(uint16_t)*info.iNumber );

		for( int32_t i = 0; i < m_TregInfo.iNumber; i++ )
		{
			m_TregInfo.pReg[i]  = info.pReg[i];

			switch( m_TregInfo.pReg[i] )
			{
			case 0x0170:
				m_TregInfo.pData[i] = m_BehaviorInfo.iMode;
				break;
			case 0x0004:	// REG_XYZ_TO_RGB
				if( m_BehaviorInfo.pInput && !strcasecmp(m_BehaviorInfo.pInput, "RGB" ) )	m_TregInfo.pData[i] = 0;
				else																		m_TregInfo.pData[i] = 1;
				break;
			case 0x0089:	// REG_ROM_SEL
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_DCI" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM1" ) )	m_TregInfo.pData[i] = 1;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM2" ) )	m_TregInfo.pData[i] = 2;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM3" ) )	m_TregInfo.pData[i] = 3;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM4" ) )	m_TregInfo.pData[i] = 4;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM5" ) )	m_TregInfo.pData[i] = 5;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "USB" ) )		m_TregInfo.pData[i] = 0;
				break;
			case 0x0082:	// REG_TGAM_RD_SEL
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_DCI" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM1" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM2" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM3" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM4" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "TCON_ROM5" ) )	m_TregInfo.pData[i] = 0;
				if( m_BehaviorInfo.pTransLoad && !strcasecmp(m_BehaviorInfo.pTransLoad, "USB" ) )		m_TregInfo.pData[i] = 1;
				break;
			default:
				m_TregInfo.pData[i] = info.pData[i];
			}
		}

		if( info.pDesc ) 	free( info.pDesc );
		if( info.pReg ) 	free( info.pReg );
		if( info.pData ) 	free( info.pData );
	}

	{
		uint16_t iSrcData[3][3] = { 0x0000, };
		uint16_t iDstData[3][3] = { 0x0000, };

		NX_TREG_INFO info;

		if( 0 > ParseTreg( NX_TREG_EEPROM_FILE, iRefMode, &info ) )
		{
			return -1;
		}

		for( int32_t i = 0; i < info.iNumber; i++ )
		{
			switch( info.pReg[i] )
			{
			case 0x0056: iSrcData[0][0] = info.pData[i]; break;
			case 0x0057: iSrcData[0][1] = info.pData[i]; break;
			case 0x0058: iSrcData[0][2] = info.pData[i]; break;
			case 0x0059: iSrcData[1][0] = info.pData[i]; break;
			case 0x005A: iSrcData[1][1] = info.pData[i]; break;
			case 0x005B: iSrcData[1][2] = info.pData[i]; break;
			case 0x005C: iSrcData[2][0] = info.pData[i]; break;
			case 0x005D: iSrcData[2][1] = info.pData[i]; break;
			case 0x005E: iSrcData[2][2] = info.pData[i]; break;
			default: break;
			}
		}

		CalulateMatrix( iSrcData, iDstData );

		for( int32_t i = 0; i < m_TregInfo.iNumber; i++ )
		{
			switch( m_TregInfo.pReg[i] )
			{
			case 0x0056: m_TregInfo.pData[i] = iDstData[0][0]; break;
			case 0x0057: m_TregInfo.pData[i] = iDstData[0][1]; break;
			case 0x0058: m_TregInfo.pData[i] = iDstData[0][2]; break;
			case 0x0059: m_TregInfo.pData[i] = iDstData[1][0]; break;
			case 0x005A: m_TregInfo.pData[i] = iDstData[1][1]; break;
			case 0x005B: m_TregInfo.pData[i] = iDstData[1][2]; break;
			case 0x005C: m_TregInfo.pData[i] = iDstData[2][0]; break;
			case 0x005D: m_TregInfo.pData[i] = iDstData[2][1]; break;
			case 0x005E: m_TregInfo.pData[i] = iDstData[2][2]; break;
			default: break;
			}
		}

		if( info.pDesc ) 	free( info.pDesc );
		if( info.pReg ) 	free( info.pReg );
		if( info.pData ) 	free( info.pData );
	}

	{
		uint16_t iData[9] = { 0x0000, };

		NX_TREG_INFO info;

		if( 0 > ParseTreg( NX_TREG_EEPROM_FILE, iCurMode, &info ) )
		{
			return -1;
		}

		for( int32_t i = 0; i < info.iNumber; i++ )
		{
			switch( info.pReg[i] )
			{
			case 0x00DD: iData[0] = info.pData[i]; break;
			case 0x00B9: iData[1] = info.pData[i]; break;
			case 0x00BA: iData[2] = info.pData[i]; break;
			case 0x00BB: iData[3] = info.pData[i]; break;
			case 0x00B6: iData[4] = info.pData[i]; break;
			case 0x00B7: iData[5] = info.pData[i]; break;
			case 0x00B8: iData[6] = info.pData[i]; break;
			case 0x0097: iData[7] = info.pData[i]; break;
			case 0x0191: iData[8] = info.pData[i]; break;
			default: break;
			}
		}

		for( int32_t i = 0; i < m_TregInfo.iNumber; i++ )
		{
			switch( m_TregInfo.pReg[i] )
			{
			case 0x00DD: m_TregInfo.pData[i] = iData[0]; break;
			case 0x00B9: m_TregInfo.pData[i] = iData[1]; break;
			case 0x00BA: m_TregInfo.pData[i] = iData[2]; break;
			case 0x00BB: m_TregInfo.pData[i] = iData[3]; break;
			case 0x00B6: m_TregInfo.pData[i] = iData[4]; break;
			case 0x00B7: m_TregInfo.pData[i] = iData[5]; break;
			case 0x00B8: m_TregInfo.pData[i] = iData[6]; break;
			case 0x0097: m_TregInfo.pData[i] = iData[7]; break;
			case 0x0191: m_TregInfo.pData[i] = iData[8]; break;
			default: break;
			}
		}

		if( info.pDesc ) 	free( info.pDesc );
		if( info.pReg ) 	free( info.pReg );
		if( info.pData ) 	free( info.pData );
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::FreeTregInfo()
{
	if( m_TregInfo.pDesc )	free( m_TregInfo.pDesc );
	if( m_TregInfo.pReg	)	free( m_TregInfo.pReg );
	if( m_TregInfo.pData )	free( m_TregInfo.pData );

	memset( &m_TregInfo, 0x00, sizeof(m_TregInfo) );
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::DumpTregInfo()
{
#if NX_ENABLE_INFO
	printf("--------------------------------------------------------------------------------\n");
	printf("TREG Information                                                                \n");
	printf("--------------------------------------------------------------------------------\n");
	printf("iMode         : %d\n", m_TregInfo.iMode );
	printf("iGammaType[0] : %d\n", m_TregInfo.iGammaType[0] );
	printf("iGammaType[1] : %d\n", m_TregInfo.iGammaType[1] );
	printf("iGammaType[2] : %d\n", m_TregInfo.iGammaType[2] );
	printf("iGammaType[3] : %d\n", m_TregInfo.iGammaType[3] );
	printf("pDesc         : %s\n", m_TregInfo.pDesc );
	printf("iNumber       : %d\n", m_TregInfo.iNumber );

#if 0
	printf("pData         : \n");
	for( int32_t i = 0; i < m_TregInfo.iNumber; i++ )
	{
		printf("reg( 0x%04x, %5d ), data( 0x%04x, %5d )\n",
			m_TregInfo.pReg[i], m_TregInfo.pReg[i],
			m_TregInfo.pData[i], m_TregInfo.pData[i]
		);
	}
#endif
#endif
}

//------------------------------------------------------------------------------
#include <math.h>
#define NX_MAX( X, Y )		(( X < Y ) ? Y : X)

int32_t CNX_CinemaBehavior::CalulateMatrix( uint16_t iSrcData[3][3], uint16_t iDstData[3][3] )
{
	int32_t iLuminance    = m_BehaviorInfo.iLuminance;
	int32_t iRefLuminance = m_BehaviorInfo.iRefLuminance;

	uint32_t iTempData[3][3];
	double fTempData[3][3];

	if( IsManual() )		// MANUAL
	{
		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				iDstData[i][j] = m_BehaviorInfo.pGamutData[3 * i + j];
			}
		}
	}
	else if( IsBypass() )	// BYPASS
	{
		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				iDstData[i][j] = (i == j) ? (16383) : (0);
			}
		}
	}
	else					// SRGB, P3, SRGB_VIVID, P3_VIVID
	{
		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				fTempData[i][j] = (double)iSrcData[i][j] * (double)iLuminance / (double)iRefLuminance;
			}
		}

		DumpArray( fTempData,
			"\n>> fTempData[][] = iSrcData[][] x iLuminance( %d ) / iRefLuminance( %d )\n", iLuminance, iRefLuminance );

		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				fTempData[i][j] = floor(fTempData[i][j] * 10) / 10;
			}
		}

		DumpArray( fTempData,
			"\n>> Floor of fTempData[][] by one decimal point\n" );

		for( int32_t i = 0; i < 3; i ++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				iTempData[i][j] = floor( fTempData[i][j] + 0.5 );
			}
		}

		DumpArray( iTempData,
			"\n>> iTempData[][] = Rounds of fTempData[][]\n" );

		uint32_t iSum[3] = { 0x0000, 0x000, 0x0000 };
		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				iSum[i] += iTempData[i][j];
			}
		}

		int32_t bNormalize = false;
		if( iSum[0] > 16383 || iSum[1] > 16383 || iSum[2] > 16383 )
		{
			bNormalize = true;
			DumpArray( iSum,
				"\n>>iSum[] = cc[][0] + cc[][1] + cc[][2]\n" );
		}

		if( bNormalize )
		{
			uint32_t iNormalize = NX_MAX( NX_MAX( iSum[0], iSum[1] ), iSum[2] );
			for( int32_t i = 0; i < 3; i++ )
			{
				for( int32_t j = 0; j < 3; j++ )
				{
					fTempData[i][j] = (double)iTempData[i][j] * (double)16383. / (double)iNormalize;
				}
			}
			DumpArray( fTempData,
				"\n>> fTempData[][] of Normalize = iTempData[][] x 16383 / MAX of iSum\n" );

			for( int32_t i = 0; i < 3; i++ )
			{
				for( int32_t j = 0; j < 3; j++ )
				{
					fTempData[i][j] = floor(fTempData[i][j] * 10) / 10;
				}
			}
			DumpArray( fTempData,
				"\n>> Floor of fTempData[][] by one decimal point\n" );

			for( int32_t i = 0; i < 3; i ++ )
			{
				for( int32_t j = 0; j < 3; j++ )
				{
					iTempData[i][j] = floor( fTempData[i][j] + 0.5 );
				}
			}
			DumpArray( iTempData,
				"\n>> iTempData[][] = Rounds of fTempData[][]\n" );
		}

		for( int32_t i = 0; i < 3; i++ )
		{
			for( int32_t j = 0; j < 3; j++ )
			{
				if( IsVivid() )	// SRGB_VIVID, P3_VIVID
				{
					iDstData[i][j] = (i == j ) ? (iTempData[i][0] +  iTempData[i][1] + iTempData[i][2]) : (0);
				}
				else			// SRGB, P3
				{
					iDstData[i][j] = (uint16_t)iTempData[i][j];
				}
			}
		}
	}

	DumpArray( iDstData, "\n>> Result ( %s ) :\n", m_BehaviorInfo.pGamut );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::IsRemark( char *pString )
{
	return (*pString == '#');
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::IsVivid( void )
{
	return	!strcasecmp( m_BehaviorInfo.pGamut, "SRGB_VIVID") ||
			!strcasecmp( m_BehaviorInfo.pGamut, "P3_VIVID");
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::IsBypass( void )
{
	return	!strcasecmp( m_BehaviorInfo.pGamut, "BYPASS" );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaBehavior::IsManual( void )
{
	return	!strcasecmp( m_BehaviorInfo.pGamut, "MANUAL" );
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::DumpArray( uint16_t iArray[3][3], const char *pFormat, ... )
{
#if NX_ENABLE_DUMP
	va_list args;
	char szMsg[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szMsg, sizeof(szMsg), pFormat, args);
	va_end(args);

	if( 0 < strlen( szMsg ) )
	{
		printf( "%s", szMsg );
	}

	{
		printf("+-----+-------------+-------------+-------------+\n");
		printf("|     |       0     |       1     |       2     |\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}

	for( int32_t i = 0; i < 3; i++ )
	{
		printf("|  %d  |", i);
		for( int32_t j = 0; j < 3; j++ )
		{
			printf("%11d  |", iArray[i][j]);
		}

		printf("\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}
#else
	(void)(iArray);
	(void)(pFormat);
#endif
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::DumpArray( uint32_t iArray[3][3], const char *pFormat, ... )
{
#if NX_ENABLE_DUMP
	va_list args;
	char szMsg[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szMsg, sizeof(szMsg), pFormat, args);
	va_end(args);

	if( 0 < strlen( szMsg ) )
	{
		printf( "%s", szMsg );
	}

	{
		printf("+-----+-------------+-------------+-------------+\n");
		printf("|     |       0     |       1     |       2     |\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}

	for( int32_t i = 0; i < 3; i++ )
	{
		printf("|  %d  |", i);
		for( int32_t j = 0; j < 3; j++ )
		{
			printf("%11d  |", iArray[i][j]);
		}

		printf("\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}
#else
	(void)(iArray);
	(void)(pFormat);
#endif
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::DumpArray( double fArray[3][3], const char *pFormat, ... )
{
#if NX_ENABLE_DUMP
	va_list args;
	char szMsg[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szMsg, sizeof(szMsg), pFormat, args);
	va_end(args);

	if( 0 < strlen( szMsg ) )
	{
		printf( "%s", szMsg );
	}

	{
		printf("+-----+-------------+-------------+-------------+\n");
		printf("|     |       0     |       1     |       2     |\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}

	for( int32_t i = 0; i < 3; i++ )
	{
		printf("|  %d  |", i);
		for( int32_t j = 0; j < 3; j++ )
		{
			printf("%11.3f  |", fArray[i][j]);
		}

		printf("\n");
		printf("+-----+-------------+-------------+-------------+\n");
	}
#else
	(void)(fArray);
	(void)(pFormat);
#endif
}

//------------------------------------------------------------------------------
void CNX_CinemaBehavior::DumpArray( uint32_t iArray[3], const char *pFormat, ... )
{
#if NX_ENABLE_DUMP
	va_list args;
	char szMsg[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szMsg, sizeof(szMsg), pFormat, args);
	va_end(args);

	if( 0 < strlen( szMsg ) )
	{
		printf( "%s", szMsg );
	}

	{
		printf("+-----+-------------+\n");
		printf("|     |       0     |\n");
		printf("+-----+-------------+\n");
	}

	for( int32_t i = 0; i < 3; i++ )
	{
		printf("|  %d  |", i);
		for( int32_t j = 0; j < 1; j++ )
		{
			printf("%11d  |", iArray[i]);
		}

		printf("\n");
		printf("+-----+-------------+\n");
	}
#else
	(void)(iArray);
	(void)(pFormat);
#endif
}
