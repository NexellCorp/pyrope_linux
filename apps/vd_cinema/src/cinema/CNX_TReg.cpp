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

#include "NX_Token.h"
#include "CNX_TReg.h"

//------------------------------------------------------------------------------
CNX_TReg::CNX_TReg()
{
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		m_pInfo[i] = NULL;
	}
}

//------------------------------------------------------------------------------
CNX_TReg::~CNX_TReg()
{
	Delete();
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Parse( const char *pFile )
{
	FILE *hFile = fopen( pFile, "rb" );
	if( NULL == hFile )
		return -1;

	Delete();

	char szLine[1024], szMode[256];
	int32_t iModeNum = 0;
	int32_t iLineCnt = 0;
	int32_t iDataCnt = 0;

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		char szTokenMode[64] = { 0x00, };
		char szTokenData[64] = { 0x00, };
		int32_t iMode;

		if( iLineCnt < NUM_INDEX )
		{
			sprintf( szMode, "%s", szLine );
			iModeNum = NX_GetTokenNum( szMode );
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iMode );
				m_pInfo[iMode] = (NX_TREG_INFO*)malloc( sizeof(NX_TREG_INFO) );
			}
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA )
		{
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iMode );

				NX_GetToken( szLine, i, szTokenData );
				m_pInfo[iMode]->iGammaType[iLineCnt-NUM_INDEX] = strtol(szTokenData, NULL, 10);
			}
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC )
		{
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iMode );

				NX_GetToken( szLine, i, szTokenData );
				m_pInfo[iMode]->pDesc = strdup(szTokenData);
			}
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC + NUM_NUMBER )
		{
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iMode );

				NX_GetToken( szLine, i, szTokenData );
				m_pInfo[iMode]->iNumber = strtol(szTokenData, NULL, 10);
				m_pInfo[iMode]->pReg    = (uint16_t*)malloc( sizeof(uint16_t)*m_pInfo[iMode]->iNumber );
				m_pInfo[iMode]->pData   = (uint16_t*)malloc( sizeof(uint16_t)*m_pInfo[iMode]->iNumber );
			}
		}
		else
		{
			int32_t iSkipCnt = 0;
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iMode );

				if( iDataCnt/2 > m_pInfo[iMode]->iNumber-1 )
				{
					iSkipCnt++;
					continue;
				}

				NX_GetToken( szLine, i-iSkipCnt, szTokenData );
				if( !(iDataCnt % 2) )	m_pInfo[iMode]->pReg[iDataCnt/2]  = strtol(szTokenData, NULL, 16);
				else					m_pInfo[iMode]->pData[iDataCnt/2] = strtol(szTokenData, NULL, 10);
			}

			iDataCnt++;
		}

		iLineCnt++;
	}

	fclose(hFile);

#if 0
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		if( NULL == m_pInfo[i] )
			continue;

		printf("* mode %d : %s\n", i, m_pInfo[i]->pDesc );
		printf("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )\n",
			m_pInfo[i]->iGammaType[0], m_pInfo[i]->iGammaType[1], m_pInfo[i]->iGammaType[2], m_pInfo[i]->iGammaType[3]);
		printf("-. number of data : %d\n", m_pInfo[i]->iNumber );
		for( int j = 0; j < m_pInfo[i]->iNumber; j++ )
			printf("[%02d] reg( %5d, 0x%04x ), data( %5d, 0x%04x )\n",
				j, m_pInfo[i]->pReg[j], m_pInfo[i]->pReg[j], m_pInfo[i]->pData[j], m_pInfo[i]->pData[j] );
	}
#endif

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Update( const char *pFile )
{
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		Update( pFile, i );
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Update( const char *pFile, int32_t iMode )
{
	FILE *hFile = fopen( pFile, "rb" );
	if( NULL == hFile )
		return -1;

	char szLine[1024], szMode[256];
	int32_t iModeNum = 0;

	int32_t iLineCnt = 0;
	int32_t iDataCnt = 0;
	int32_t iSkipCnt = 0;

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		char szTokenMode[64] = { 0x00, };
		char szTokenData[64] = { 0x00, };
		int32_t iTemp;

		if( iLineCnt < NUM_INDEX )
		{
			sprintf( szMode, "%s", szLine );
			iModeNum = NX_GetTokenNum( szMode );
			for( int32_t i = 0; i < iModeNum; i++ )
			{
				NX_GetToken( szMode, i, szTokenMode );
				NX_GetTokenParse( szTokenMode, "MODE%d", &iTemp );

				if( iTemp == iMode )
					break;

				iSkipCnt++;
			}

			if( iTemp == iMode )
			{
				if( m_pInfo[iMode] )
				{
					if( m_pInfo[iMode]->pDesc )	free( m_pInfo[iMode]->pDesc );
					if( m_pInfo[iMode]->pReg )	free( m_pInfo[iMode]->pReg );
					if( m_pInfo[iMode]->pData )	free( m_pInfo[iMode]->pData );
				}
				m_pInfo[iTemp] = (NX_TREG_INFO*)malloc( sizeof(NX_TREG_INFO) );
			}
			else
			{
				// printf( "Fail, Not match mode. ( %d )\n", iMode );
				fclose( hFile );
				return -1;
			}
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA )
		{
			NX_GetToken( szLine, iSkipCnt, szTokenData );
			m_pInfo[iMode]->iGammaType[iLineCnt-NUM_INDEX] = strtol(szTokenData, NULL, 10);
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC )
		{
			NX_GetToken( szLine, iSkipCnt, szTokenData );
			m_pInfo[iMode]->pDesc = strdup(szTokenData);
		}
		else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC + NUM_NUMBER )
		{
			NX_GetToken( szLine, iSkipCnt, szTokenData );
			m_pInfo[iMode]->iNumber = strtol(szTokenData, NULL, 10);
			m_pInfo[iMode]->pReg    = (uint16_t*)malloc( sizeof(uint16_t)*m_pInfo[iMode]->iNumber );
			m_pInfo[iMode]->pData   = (uint16_t*)malloc( sizeof(uint16_t)*m_pInfo[iMode]->iNumber );
		}
		else
		{
			NX_GetToken( szLine, iSkipCnt, szTokenData );
			if( !(iDataCnt % 2) )	m_pInfo[iMode]->pReg[iDataCnt/2]  = strtol(szTokenData, NULL, 16);
			else					m_pInfo[iMode]->pData[iDataCnt/2] = strtol(szTokenData, NULL, 10);

			if( (++iDataCnt) / 2 == m_pInfo[iMode]->iNumber )
				break;
		}

		iLineCnt++;
	}

	fclose( hFile );

#if 0
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		if( NULL == m_pInfo[i] )
			continue;

		printf("* mode %d : %s\n", i, m_pInfo[i]->pDesc );
		printf("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )\n",
			m_pInfo[i]->iGammaType[0], m_pInfo[i]->iGammaType[1], m_pInfo[i]->iGammaType[2], m_pInfo[i]->iGammaType[3]);
		printf("-. number of data : %d\n", m_pInfo[i]->iNumber );
		for( int j = 0; j < m_pInfo[i]->iNumber; j++ )
			printf("-. reg( %5d, 0x%04x ), data( %5d, 0x%04x )\n",
				m_pInfo[i]->pReg[j], m_pInfo[i]->pReg[j], m_pInfo[i]->pData[j], m_pInfo[i]->pData[j] );
	}
#endif

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Delete()
{
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		if( m_pInfo[i] )
		{
			if( m_pInfo[i]->pDesc )	free( m_pInfo[i]->pDesc );
			if( m_pInfo[i]->pReg )	free( m_pInfo[i]->pReg );
			if( m_pInfo[i]->pData )	free( m_pInfo[i]->pData );

			m_pInfo[i] = NULL;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Delete( int32_t iMode )
{
	if( NULL != m_pInfo[iMode] )
	{
		if( m_pInfo[iMode] )
		{
			if( m_pInfo[iMode]->pDesc )	free( m_pInfo[iMode]->pDesc );
			if( m_pInfo[iMode]->pReg )	free( m_pInfo[iMode]->pReg );
			if( m_pInfo[iMode]->pData )	free( m_pInfo[iMode]->pData );

			m_pInfo[iMode] = NULL;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Make( const char *pFile )
{
	FILE *hFile = fopen( pFile, "wb" );
	if( NULL == hFile )
		return -1;

	int32_t iLineCnt = 0;
	int32_t iDataCnt = 0;
	int32_t iMaxDataNum = 0;
	int32_t bRun = true;

	while( bRun )
	{
		int32_t iWritten = 0;
		char szData[128];

		for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
		{
			if( NULL == m_pInfo[i] )
				continue;

			if( iLineCnt < NUM_INDEX )
			{
				iWritten += sprintf( szData, "%sMODE%d",
					(iWritten > 0) ? "\t" : "",
					i
				);
				fwrite( szData, 1, strlen(szData), hFile );
			}
			else if( iLineCnt < NUM_INDEX + NUM_GAMMA )
			{
				iWritten += sprintf( szData, "%s%d",
					(iWritten > 0) ? "\t" : "",
					m_pInfo[i]->iGammaType[iLineCnt-NUM_INDEX]
				);
				fwrite( szData, 1, strlen(szData), hFile );
			}
			else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC )
			{
				iWritten += sprintf( szData, "%s%s",
					(iWritten > 0) ? "\t" : "",
					m_pInfo[i]->pDesc
				);
				fwrite( szData, 1, strlen(szData), hFile );
			}
			else if( iLineCnt < NUM_INDEX + NUM_GAMMA + NUM_DESC + NUM_NUMBER )
			{
				iWritten += sprintf( szData, "%s%d",
					(iWritten > 0) ? "\t" : "",
					m_pInfo[i]->iNumber
				);
				fwrite( szData, 1, strlen(szData), hFile );

				if( iMaxDataNum < m_pInfo[i]->iNumber )
					iMaxDataNum = m_pInfo[i]->iNumber;
			}
			else
			{
				iDataCnt = (iLineCnt - NUM_INDEX - NUM_GAMMA - NUM_DESC - NUM_NUMBER);

				if( (iDataCnt/2) == iMaxDataNum )
				{
					bRun = false;
					break;
				}

				if( (iDataCnt/2) < m_pInfo[i]->iNumber )
				{
					if( !(iDataCnt % 2) )
					{
						iWritten = sprintf( szData, "%s0x%X",
							(iWritten > 0) ? "\t" : "",
							m_pInfo[i]->pReg[iDataCnt/2]
						);
						fwrite( szData, 1, strlen(szData), hFile );
					}
					else
					{
						iWritten += sprintf( szData, "%s%d",
							(iWritten > 0) ? "\t" : "",
							m_pInfo[i]->pData[iDataCnt/2]
						);
						fwrite( szData, 1, strlen(szData), hFile );
					}
				}
				else
				{
					iWritten += sprintf( szData, "\t" );
					fwrite( szData, 1, strlen(szData), hFile );
				}
			}
		}

		sprintf( szData, "\r\n" );
		fwrite( szData, 1, strlen(szData), hFile );

		iLineCnt++;
	}

	fclose( hFile );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::Dump()
{
	for( int32_t i = 0; i < MAX_INFO_NUM; i++ )
	{
		if( NULL == m_pInfo[i] )
			continue;

		printf("* mode %d : %s\n", i, m_pInfo[i]->pDesc );
		printf("-. TGAM0 ( %d ), TGAM1 ( %d ), DGAM0 ( %d ), DGAM1 ( %d )\n",
			m_pInfo[i]->iGammaType[0], m_pInfo[i]->iGammaType[1], m_pInfo[i]->iGammaType[2], m_pInfo[i]->iGammaType[3]);
		printf("-. number of data : %d\n", m_pInfo[i]->iNumber );
		for( int j = 0; j < m_pInfo[i]->iNumber; j++ )
			printf("-. reg( %5d, 0x%04x ), data( %5d, 0x%04x )\n",
				m_pInfo[i]->pReg[j], m_pInfo[i]->pReg[j], m_pInfo[i]->pData[j], m_pInfo[i]->pData[j] );
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::IsValid( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return false;

	return m_pInfo[iMode] ? true : false;
}

//------------------------------------------------------------------------------
int32_t* CNX_TReg::GetGammaType( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return NULL;

	return m_pInfo[iMode] ? m_pInfo[iMode]->iGammaType : NULL;
}

//------------------------------------------------------------------------------
char* CNX_TReg::GetDescriptor( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return NULL;

	return m_pInfo[iMode] ? m_pInfo[iMode]->pDesc : NULL;
}

//------------------------------------------------------------------------------
int32_t CNX_TReg::GetDataNum( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return 0;

	return m_pInfo[iMode] ? m_pInfo[iMode]->iNumber : 0;
}

//------------------------------------------------------------------------------
uint16_t* CNX_TReg::GetRegister( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return NULL;

	return m_pInfo[iMode] ? m_pInfo[iMode]->pReg : NULL;
}

//------------------------------------------------------------------------------
uint16_t* CNX_TReg::GetData( int32_t iMode )
{
	if( 0 > iMode || iMode > MAX_INFO_NUM )
		return NULL;

	return m_pInfo[iMode] ? m_pInfo[iMode]->pData : NULL;
}

