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
#include <stdint.h>
#include <stdlib.h>

#include "CNX_File.h"
#include "CNX_Gamma.h"

//------------------------------------------------------------------------------
CNX_Gamma::CNX_Gamma()
	: m_iType( -1 )
	, m_iTable( -1 )
	, m_iChannel( -1 )
	, m_pData( NULL )
{
}

//------------------------------------------------------------------------------
CNX_Gamma::~CNX_Gamma()
{
	Deinit();
}

//------------------------------------------------------------------------------
int32_t CNX_Gamma::Parse( const char *pFile )
{
	CNX_File file( pFile );
	if( !file.IsExist() )
	{
		return -1;
	}

	char *pBasename = NULL;
	if( 0 > file.GetBasename( &pBasename ) )
	{
		return -1;
	}

	Deinit();

	char szType = 0, szTable = 0, szChannel = 0;
	sscanf( pBasename, "%cGAM%c_%c.txt", &szType, &szTable, &szChannel );
	if( szType == 0 || szTable == 0 || szChannel == 0 )
	{
		printf("Fail, Parse(). ( %s )\n", pFile);
		return -1;
	}

	if( szType == 'T' )		m_iType = TYPE_TARGET;
	if( szType == 'D' )		m_iType = TYPE_DEVICE;

	if( szTable == '0' )	m_iTable = TABLE_LUT0;
	if( szTable == '1' )	m_iTable = TABLE_LUT1;

	if( szChannel == 'R' )	m_iChannel = CHANNEL_RED;
	if( szChannel == 'G' )	m_iChannel = CHANNEL_GREEN;
	if( szChannel == 'B' )	m_iChannel = CHANNEL_BLUE;

	if( 0 > Init() )
	{
		return -1;
	}

	FILE *hFile = fopen( pFile, "rb" );
	if( NULL == hFile )
	{
		return -1;
	}

	char szLine[16];
	int32_t iLineCnt = 0;

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		if( iLineCnt >= MAX_DATA_NUM )
			break;

		m_pData[iLineCnt] = strtol( szLine, NULL, 10 );
		iLineCnt++;
	}

	fclose( hFile );

	// printf(">>> path: %s\n", pFile );
	// printf(">>> type: %c ( %d ), table: %c ( %d ), channel: %c ( %d )\n", szType, m_iType, szTable, m_iTable, szChannel, m_iChannel );
	// for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	// {
	// 	printf("%8d", m_pData[i]);
	// 	if( (i % 8) == 7 )	printf("\n");
	// 	else printf("\t");
	// }

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_Gamma::GetType()
{
	return m_iType;
}

//------------------------------------------------------------------------------
int32_t CNX_Gamma::GetTable()
{
	return m_iTable;
}

//------------------------------------------------------------------------------
int32_t CNX_Gamma::GetChannel()
{
	return m_iChannel;
}

//------------------------------------------------------------------------------
uint32_t* CNX_Gamma::GetData()
{
	return m_pData;
}

//------------------------------------------------------------------------------
int32_t CNX_Gamma::Init()
{
	m_pData = (uint32_t*)malloc( sizeof(uint32_t) * MAX_DATA_NUM );
	return m_pData ? 0 : -1;
}

//------------------------------------------------------------------------------
void CNX_Gamma::Deinit()
{
	if( m_pData )
	{
		free( m_pData );
		m_pData = NULL;
	}

	m_iType    = -1;
	m_iTable   = -1;
	m_iChannel = -1;
}
