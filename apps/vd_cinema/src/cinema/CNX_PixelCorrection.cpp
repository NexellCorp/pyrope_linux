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

#include <NX_Utils.h>
#include "NX_Token.h"
#include "CNX_File.h"
#include "CNX_PixelCorrection.h"

#define NX_DISPLAY_DECIMAL		0

//------------------------------------------------------------------------------
CNX_PixelCorrection::CNX_PixelCorrection()
	: m_pData( NULL )
	, m_pReorder( NULL )
{

}

//------------------------------------------------------------------------------
CNX_PixelCorrection::~CNX_PixelCorrection()
{
	Deinit();
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::Parse( const char *pFile )
{
	CNX_File file(pFile);
	if( !file.IsExist() )
	{
		return -1;
	}

	char *pBasename = NULL;
	if( 0 > file.GetBasename( &pBasename) )
	{
		return -1;
	}

	Deinit();

	int32_t iIndex = -1, iSide = -1, iLine = -1, iType = -1;
	char szSide = 0, szType = 0;

	sscanf( pBasename, "RGB_P2_5_ID%03d_%c%d%c.txt", &iIndex, &szSide, &iLine, &szType );
	if( iIndex < 0 || szSide == 0 || iLine < 0 || szType == 0 )
	{
		printf("Fail, Parse(). ( %s )\n", pFile);
		return -1;
	}

	if( 8 > (iIndex % 16) ) m_iSlave = (iIndex       ) + 16;
	else					m_iSlave = (iIndex | 0x80) + 16;

	iSide = (szSide == 'L') ? 0 : 1;
	iType = (szType == 'A') ? 0 : 1;
	m_iModule = (4 * iLine) + (2 * iSide) + iType;

	if( 0 > Init() )
	{
		return -1;
	}

	FILE *hFile = fopen( pFile, "rb" );
	if( NULL == hFile )
	{
		return -1;
	}

	char szLine[1024];
	int32_t iData[MAX_REG_NUM];
	uint16_t *pData, *pReorder;

	pData = m_pData;

	while( fgets( szLine, sizeof(szLine)-1, hFile ) )
	{
		int32_t iReadSize = sscanf( szLine, "%d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			iData+0, iData+1, iData+2, iData+3, iData+4, iData+5, iData+6, iData+7, iData+8 );

		if( MAX_REG_NUM != iReadSize )
			break;

		for( int32_t i = 0; i < MAX_REG_NUM; i++ )
		{
			*(pData+i) = iData[i];
		}
		pData += MAX_REG_NUM;
	}

	fclose( hFile );

	pData    = m_pData;
	pReorder = m_pReorder;

	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		*(pReorder+0) = ((*(pData+7) & 0x0003) << 14) | ((*(pData+8) & 0x3FFF) >>  0);
		*(pReorder+1) = ((*(pData+6) & 0x000F) << 12) | ((*(pData+7) & 0x3FFC) >>  2);
		*(pReorder+2) = ((*(pData+5) & 0x003F) << 10) | ((*(pData+6) & 0x3FF0) >>  4);
		*(pReorder+3) = ((*(pData+4) & 0x00FF) <<  8) | ((*(pData+5) & 0x3FC0) >>  6);
		*(pReorder+4) = ((*(pData+3) & 0x03FF) <<  6) | ((*(pData+4) & 0x3F00) >>  8);
		*(pReorder+5) = ((*(pData+2) & 0x0FFF) <<  4) | ((*(pData+3) & 0x3C00) >> 10);
		*(pReorder+6) = ((*(pData+1) & 0x3FFF) <<  2) | ((*(pData+2) & 0x3000) >> 12);
		*(pReorder+7) =                                 ((*(pData+0) & 0x3FFF) >>  0);

		pData    += MAX_REG_NUM;
		pReorder += MAX_REG_NUM-1;
	}

#if 0
	pData    = m_pData;
	pReorder = m_pReorder;
	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		for( int32_t j = 0; j < MAX_REG_NUM; j++ )
		{
#if NX_DISPLAY_DECIMAL
			printf("%5d ", *(pData+j));
#else
			printf("0x%04X ", *(pData+j));
#endif
			pData++;
		}
		printf("\n");
	}

	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		for( int32_t j = 0; j < MAX_REG_NUM-1; j++ )
		{
#if NX_DISPLAY_DECIMAL
			printf("%5d ", *(pReorder+j));
#else
			printf("0x%04X ", *(pReorder+j));
#endif
			pReorder++;
		}
		printf("\n");
	}
#endif

	printf("Parse Done. ( file: %s, slave: 0x%02X, module: %d )\n", pFile, m_iSlave, m_iModule);
	Make( "./", m_iSlave, m_iModule, m_pReorder, MAX_DATA_NUM * (MAX_REG_NUM-1) );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::Make( const char *pPath, uint8_t iSlave, int32_t iModule, uint16_t *pData, int32_t iDataSize )
{
	if( iDataSize != MAX_DATA_NUM * (MAX_REG_NUM-1) )
	{
		return -1;
	}

	//
	//	Convert Data
	//
	uint16_t result[MAX_DATA_NUM*MAX_REG_NUM];
	uint16_t *pSrc, *pDst;

	pSrc = pData;
	pDst = result;

	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		*(pDst+0) = ((*(pSrc+7)      ) & 0x3FFF);
		*(pDst+1) = ((*(pSrc+6) >>  2) & 0x3FFF);
		*(pDst+2) = ((*(pSrc+6) << 12) & 0x3000) | ((*(pSrc+5) >>  4) & 0x0FFF);
		*(pDst+3) = ((*(pSrc+5) << 10) & 0x3C00) | ((*(pSrc+4) >>  6) & 0x03FF);
		*(pDst+4) = ((*(pSrc+4) <<  8) & 0x3F00) | ((*(pSrc+3) >>  8) & 0x00FF);
		*(pDst+5) = ((*(pSrc+3) <<  6) & 0x3FC0) | ((*(pSrc+2) >> 10) & 0x003F);
		*(pDst+6) = ((*(pSrc+2) <<  4) & 0x3FF0) | ((*(pSrc+1) >> 12) & 0x000F);
		*(pDst+7) = ((*(pSrc+1) <<  2) & 0x3FFC) | ((*(pSrc  ) >> 14) & 0x0003);
		*(pDst+8) = ((*(pSrc  )      ) & 0x3FFF);

		// printf("%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d\r\n",
		// 	*(pSrc+0), *(pSrc+1), *(pSrc+2), *(pSrc+3),
		// 	*(pSrc+4), *(pSrc+5), *(pSrc+6), *(pSrc+7)
		// );

		// printf("%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d\r\n",
		// 	*(pDst+0), *(pDst+1), *(pDst+2), *(pDst+3), *(pDst+4),
		// 	*(pDst+5), *(pDst+6), *(pDst+7), *(pDst+8)
		// );

		pDst += MAX_REG_NUM;
		pSrc += MAX_REG_NUM-1;
	}

	char szPath[1024];
	sprintf(szPath, "%s/PIXEL_CORRECTION_ID%03d",
		pPath,
		(iSlave & 0x7F) - 0x10
	);

	NX_MakeDirectory( szPath );

	char szFile[1024];
	sprintf(szFile, "%s/RGB_P2_5_ID%03d_%s%d%s.txt",
		szPath,
		(iSlave & 0x7F) - 0x10,
		((iModule % 4) < 2) ? "L" : "R",
		(iModule / 4),
		((iModule % 4) % 2) ? "B" : "A"
	);

	FILE *hFile = fopen( szFile, "wb" );
	if( NULL == hFile )
	{
		return -1;
	}

	pDst = result;
	for( int32_t i = 0; i < MAX_DATA_NUM; i++ )
	{
		char szData[1024] = { 0x00, };
		sprintf(szData, "%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d\r\n",
			*(pDst+0), *(pDst+1), *(pDst+2), *(pDst+3), *(pDst+4),
			*(pDst+5), *(pDst+6), *(pDst+7), *(pDst+8)
		);

		fwrite(szData, 1, strlen(szData), hFile);
		pDst += MAX_REG_NUM;
	}
	fclose( hFile );

	printf("Make Done. ( file: %s, slave: 0x%02X, module: %d )\n", szFile, iSlave, iModule);
	return 0;
}

//------------------------------------------------------------------------------
uint8_t CNX_PixelCorrection::GetSlave()
{
	return m_iSlave;
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::GetModule()
{
	return m_iModule;
}

//------------------------------------------------------------------------------
uint16_t* CNX_PixelCorrection::GetData()
{
	return m_pData;
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::GetDataNum()
{
	return MAX_DATA_NUM * MAX_REG_NUM;
}

//------------------------------------------------------------------------------
uint16_t* CNX_PixelCorrection::GetDataReorder()
{
	return m_pReorder;
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::GetDataReorderNum()
{
	return MAX_DATA_NUM * (MAX_REG_NUM-1);
}

//------------------------------------------------------------------------------
int32_t CNX_PixelCorrection::Init()
{
	m_pData = (uint16_t*)malloc( sizeof(uint16_t) * MAX_DATA_NUM * MAX_REG_NUM );
	if( NULL == m_pData )
	{
		printf("Fail, malloc().\n");
		return -1;
	}

	m_pReorder = (uint16_t*)malloc( sizeof(uint16_t) * MAX_DATA_NUM * (MAX_REG_NUM-1) );
	if( NULL == m_pReorder )
	{
		printf("Fail, malloc().\n");
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_PixelCorrection::Deinit()
{
	if( m_pData )
	{
		free( m_pData );
	}

	if( m_pReorder )
	{
		free( m_pReorder );
	}
}
