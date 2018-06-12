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
#include "CNX_Uniformity.h"

//------------------------------------------------------------------------------
CNX_Uniformity::CNX_Uniformity()
	: m_pData( NULL )
{
}

//------------------------------------------------------------------------------
CNX_Uniformity::~CNX_Uniformity()
{
	Deinit();
}

//------------------------------------------------------------------------------
int32_t CNX_Uniformity::Parse( const char *pFile )
{
	FILE *hFile = fopen( pFile, "rb" );
	if( NULL == hFile )
		return -1;

	Deinit();

	if( 0 > Init() )
	{
		return -1;
	}

	int32_t iDataCnt = 0;
	char szLine[1024], szToken[64];

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		if( iDataCnt >= MAX_DATA_NUM )
		{
			printf("Warn, Data Overflow.\n");
			break;
		}

		char *pLine = szLine;
		while( 0 < sscanf( pLine, "%s", szToken ) )
		{
			m_pData[iDataCnt] = atoi( szToken );
			pLine += strlen( szToken ) + 1;
			iDataCnt++;
		}
	}

	fclose(hFile);

#if 0
	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		printf("%d\n", m_pData[i]);
	}
#endif

	return 0;
}

//------------------------------------------------------------------------------
uint16_t* CNX_Uniformity::GetData()
{
	return m_pData;
}

//------------------------------------------------------------------------------
int32_t CNX_Uniformity::Init()
{
	m_pData = (uint16_t*)malloc( sizeof(uint16_t) * MAX_DATA_NUM );

	if( NULL == m_pData )
	{
		return -1;
	}

	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		m_pData[i] = 4096;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_Uniformity::Deinit()
{
	if( m_pData )
	{
		free( m_pData );
		m_pData = NULL;
	}
}
