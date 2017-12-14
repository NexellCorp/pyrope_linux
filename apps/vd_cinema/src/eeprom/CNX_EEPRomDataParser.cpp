//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
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

#include "CNX_EEPRomDataParser.h"

#include <stdio.h>		//	FILE i/o
#include <stdlib.h>		//	malloc
#include <string.h>		//	memset

#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

CNX_EEPRomDataParser::CNX_EEPRomDataParser()
	: m_pInBuffer(NULL)
	, m_nInBufferSize(0)
	, m_pOutBuffer(NULL)
	, m_nOutBufSize(0)
	, m_pTconInfo(NULL)
	, m_InputMode(INPUT_MODE_NONE)
{
	m_pTconInfo = (TCON_EEPROM_INFO *)malloc(sizeof(TCON_EEPROM_INFO));
	memset( m_pTconInfo , 0, sizeof(TCON_EEPROM_INFO) );
}

CNX_EEPRomDataParser::~CNX_EEPRomDataParser()
{
	if( m_pTconInfo )
		free(m_pTconInfo );
	if( m_pInBuffer )
		free(m_pInBuffer);
}

int32_t CNX_EEPRomDataParser::Init( const char *inFile )
{
	if( NULL == inFile )
		return ERR_IN_FILE;
	if( m_pInBuffer )
		free(m_pInBuffer);

	FILE *pFd = fopen(inFile, "rb");
	if( NULL == pFd )
		return ERR_IN_FILE;

	//	File file size
	fseek( pFd, 0, SEEK_END );
	long fileSize = ftell( pFd );
	fseek( pFd, 0, SEEK_SET);

	//	Allocate Input Buffer
	m_pInBuffer = malloc( fileSize );
	m_nInBufferSize = fileSize;

	//	Read Buffer from file
	fileSize = fread( m_pInBuffer, 1, m_nInBufferSize, pFd );

	if( fileSize != m_nInBufferSize )
	{
		fclose( pFd );
		return ERR_IN_FILE;
	}

	fclose( pFd );
	m_InputMode = INPUT_MODE_FILE;
	return ERR_NONE;
}

int32_t CNX_EEPRomDataParser::Init( void *pBuf, int32_t bufSize )
{
	if( NULL == pBuf )
		return ERR_IN_BUF;

	if( m_pInBuffer )
		free(m_pInBuffer);
	m_pInBuffer = malloc( bufSize );
	m_nInBufferSize = bufSize;
	memcpy( m_pInBuffer, pBuf, bufSize );
	m_InputMode = INPUT_MODE_BUFFER;
	return ERR_NONE;
}

int32_t CNX_EEPRomDataParser::Parse( TCON_EEPROM_INFO **ppOutInfo )
{
	if( m_InputMode == INPUT_MODE_NONE || m_nInBufferSize<=0 )
		return ERR_NOT_INIT;

	int32_t pos = 0;
	int32_t end = m_nInBufferSize;
	uint8_t *pStart = (uint8_t*)m_pInBuffer;
	uint8_t *pBuf;		//	temporal data pointer

	memset( m_pTconInfo , 0, sizeof(TCON_EEPROM_INFO) );

	//
	//	Pre-parsing : parsing blocks from main input data block.
	//

	//	Header Block
	pBuf = pStart;
	if( end-pos < SIZE_HEADER )
	{
		return ERR_PARSING;
	}
	memcpy( m_pTconInfo->header, pBuf, SIZE_HEADER );

	//	Version
	pBuf = pStart + OFFSET_VERSION;
	pos = OFFSET_VERSION;
	if( end-pos < SIZE_VERSION )
	{
		return ERR_PARSING;
	}
	memcpy( m_pTconInfo->version, pBuf, SIZE_VERSION );

	//	DGAM0
	pBuf = pStart + OFFSET_DGAM0;
	pos = OFFSET_DGAM0;
	if( end-pos < SIZE_DGAM )
	{
		return ERR_PARSING;
	}
	ParseGammaData( pBuf, &m_pTconInfo->DGAM0 );

	//	DGAM1
	pBuf = pStart + OFFSET_DGAM1;
	pos = OFFSET_DGAM1;
	if( end-pos < SIZE_DGAM )
	{
		return ERR_PARSING;
	}
	ParseGammaData( pBuf, &m_pTconInfo->DGAM1 );

	//	TGAM1
	pBuf = pStart + OFFSET_TGAM1;
	pos = OFFSET_TGAM1;
	if( end-pos < SIZE_TGAM )
	{
		return ERR_PARSING;
	}
	ParseGammaData( pBuf, &m_pTconInfo->TGAM1 );

	//
	//	Parsing Mode Header & Mode Data
	//
	{
		uint8_t modeHeader[SIZE_MODE_HEADER];
		MODE_HEADER_INFO *pHeaderInfo = &m_pTconInfo->modeHeader;

		//	Read Mode Header Data
		pBuf = pStart + OFFSET_MODE_HEADER;
		pos = OFFSET_MODE_HEADER;
		if( end-pos < SIZE_MODE_HEADER )
		{
			return ERR_PARSING;
		}
		memcpy( modeHeader, pBuf, SIZE_MODE_HEADER );

		//
		//	Parsing MODE Header
		//
		pBuf = modeHeader;

		//	Number of Modes
		pHeaderInfo->numModes = pBuf[0];
		pBuf ++;
		for( int i=0 ; i<pHeaderInfo->numModes ; i++ )
		{
			//	Parsing eatch mode's start address
			pHeaderInfo->modeAddr[i] = (pBuf[2] << 16) |  (pBuf[1] << 8) | pBuf[0];
			pBuf += 3;
		}
		//	XOR_CODE3
		pHeaderInfo->modeXor = (pBuf[2] << 16) |  (pBuf[1] << 8) | pBuf[0];

		//	Parsing each MODE information
		for( int i=0 ; i<pHeaderInfo->numModes ; i++ )
		{
			ParseModeData( pStart + pHeaderInfo->modeAddr[i], &m_pTconInfo->modeData[i] );
		}
	}

	*ppOutInfo = m_pTconInfo;

	return ERR_NONE;
}

int32_t CNX_EEPRomDataParser::ParseVersion( TCON_EEPROM_INFO **ppOutInfo )
{
	if( m_InputMode == INPUT_MODE_NONE || m_nInBufferSize<=0 )
		return ERR_NOT_INIT;

	int32_t pos = 0;
	int32_t end = m_nInBufferSize;
	uint8_t *pStart = (uint8_t*)m_pInBuffer;
	uint8_t *pBuf;		//	temporal data pointer

	memset( m_pTconInfo , 0, sizeof(TCON_EEPROM_INFO) );

	//
	//	Pre-parsing : parsing blocks from main input data block.
	//

	//	Header Block
	pBuf = pStart;
	if( end-pos < SIZE_HEADER )
	{
		return ERR_PARSING;
	}
	memcpy( m_pTconInfo->header, pBuf, SIZE_HEADER );

	//	Version
	pBuf = pStart + OFFSET_VERSION;
	pos = OFFSET_VERSION;
	if( end-pos < SIZE_VERSION )
	{
		return ERR_PARSING;
	}
	memcpy( m_pTconInfo->version, pBuf, SIZE_VERSION );

	*ppOutInfo = m_pTconInfo;

	return ERR_NONE;
}

void CNX_EEPRomDataParser::Deinit()
{
	if( m_pTconInfo )
	{
		memset( m_pTconInfo , 0, sizeof(TCON_EEPROM_INFO) );
	}

	if( m_pInBuffer )
	{
		free(m_pInBuffer);
		m_pInBuffer = NULL;
	}
}


//
//	Private Methods
//
void CNX_EEPRomDataParser::ParseModeData( uint8_t *pInBuf, MODE_DATA_INFO *pOutInfo )
{
	//	MODE0 signiture ACCII : 'M','O','D','E','0' ~ 'M','O','D','E','5'
	memcpy( pOutInfo->signiture, pInBuf, 5 );
	pInBuf += 5;

	//	TGAM/DGAM USE
	pOutInfo->use = *pInBuf++;

	//	DESCRIPTION
	memcpy( pOutInfo->description, pInBuf, 32 );
	pInBuf += 32;

	//	REG_NUM
	pOutInfo->numReg = *pInBuf++;

	//	skip XOR_CODE1
	pInBuf++;

	//	Parsing : REG_ADDR & REG_DATA
	for( int i=0 ; i< pOutInfo->numReg ; i++ )
	{
		pOutInfo->regAddr[i] = pInBuf[1]<<8 | pInBuf[0];
		pOutInfo->regData[i] = pInBuf[3]<<8 | pInBuf[2];
		pInBuf += 4;
	}

	//	skip XOR_CODE2
	//pInBuf += 2;
}


void CNX_EEPRomDataParser::ParseGammaData( uint8_t *pInBuf, GAMMA_INFO *pOutGamma )
{
	uint32_t *gamma;
	uint8_t *pBuf = pInBuf;
	for( int j=0 ; j<3 ; j++ )
	{
		if( 0 == j )		gamma = pOutGamma->GAMMA_R;
		else if( 1 == j )	gamma = pOutGamma->GAMMA_G;
		else if( 2 == j )	gamma = pOutGamma->GAMMA_B;

		//	GAMMA Upper[17:2]
		for(int i=0 ; i<4096 ; i++)
		{
			gamma[i] = ((pBuf[1] << 8) | pBuf[0]) << 2;
			pBuf += 2;
		}
		//	Skip xorCode2 2 bytes for upper
		pBuf += 2;
		//	GAMMA_B Lower[17:2]
		for(int i=0 ; i<(4096)/4 ; i++)
		{
			gamma[i*4+0] |= ((pBuf[0] >> 0)& 0x3);
			gamma[i*4+1] |= ((pBuf[0] >> 2)& 0x3);
			gamma[i*4+2] |= ((pBuf[0] >> 4)& 0x3);
			gamma[i*4+3] |= ((pBuf[0] >> 6)& 0x3);
			pBuf++;
		}
		//	Skip xorCode2 2 bytes for lower
		pBuf += 2;
	}
}




//
//	Write to File
//
#ifndef MAX_PATH
#define MAX_PATH	1024
#endif

/*
	Output Derectory Hierachy
	TOP(Output File Path)
	 |
	 +-- T_REG.txt
	 |
	 +-- LUT
	      |
		  +-- DGAM0_R.txt
		  +-- DGAM0_G.txt
		  +-- DGAM0_B.txt
		  +-- DGAM1_R.txt
		  +-- DGAM1_G.txt
		  +-- DGAM1_B.txt
		  +-- TGAM1_R.txt
		  +-- TGAM1_G.txt
		  +-- TGAM1_B.txt
*/
void CNX_EEPRomDataParser::WriteGamaInfo(GAMMA_INFO *pGamaInfo, const char *outFilePreFix)
{
	FILE *pFd;
	char filename[MAX_PATH];
	sprintf( filename, "%s_R.txt", outFilePreFix );
	pFd = fopen( filename, "w" );
	if( pFd )
	{
		for( int i=0 ; i<4096 ; i++ )
		{
			fprintf( pFd, "%d\n", pGamaInfo->GAMMA_R[i] );
		}
		fclose( pFd );
	}

	sprintf( filename, "%s_G.txt", outFilePreFix );
	pFd = fopen( filename, "w" );
	if( pFd )
	{
		for( int i=0 ; i<4096 ; i++ )
		{
			fprintf( pFd, "%d\n", pGamaInfo->GAMMA_G[i] );
		}
		fclose( pFd );
	}

	sprintf( filename, "%s_B.txt", outFilePreFix );
	pFd = fopen( filename, "w" );
	if( pFd )
	{
		for( int i=0 ; i<4096 ; i++ )
		{
			fprintf( pFd, "%d\n", pGamaInfo->GAMMA_B[i] );
		}
		fclose( pFd );
	}
}

void CNX_EEPRomDataParser::WriteModeInfo(TCON_EEPROM_INFO *pTconInfo, const char *outFileName)
{
	FILE *pFd;
	int32_t numModes;
	pFd = fopen( outFileName, "w" );

	if( NULL == pFd )
		return;

	numModes = pTconInfo->modeHeader.numModes;

	//	Mode Name : MODE0 ~ MODE9
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%s\t", pTconInfo->modeData[i].signiture);
	}
	fprintf(pFd, "\n");

	//	USE : DGAM0
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%d\t", pTconInfo->modeData[i].use & 0x1);
	}
	fprintf(pFd, "\n");

	//	USE : DGAM1
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%d\t", (pTconInfo->modeData[i].use & 0x2)?1:0);
	}
	fprintf(pFd, "\n");

	//	USE : TGAM0
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%d\t", (pTconInfo->modeData[i].use & 0x4)?1:0);
	}
	fprintf(pFd, "\n");

	//	USE : TGAM1
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%d\t", (pTconInfo->modeData[i].use & 0x8)?1:0);
	}
	fprintf(pFd, "\n");


	//	Description
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%s\t", pTconInfo->modeData[i].description);
	}
	fprintf(pFd, "\n");

	//	Number of Registers
	for( int i=0 ; i<numModes ; i++ )
	{
		fprintf(pFd, "%d\t", pTconInfo->modeData[i].numReg);
	}
	fprintf(pFd, "\n");


	int32_t maxReg = 0;
	for( int i=0 ; i<numModes ; i++ )
	{
		if( pTconInfo->modeData[i].numReg > maxReg )
		{
			maxReg = pTconInfo->modeData[i].numReg;
		}
	}

	// Number Registers
	for( int j=0 ; j<maxReg ; j++ )
	{
		//	Address
		for( int i=0 ; i<numModes ; i++ )
		{
			if( j < pTconInfo->modeData[i].numReg )
			{
				fprintf(pFd, "0x%X\t", pTconInfo->modeData[i].regAddr[j]);
			}
			else
			{
				fprintf(pFd, "0\t");
			}
		}
		fprintf(pFd, "\n");
		//	Values
		for( int i=0 ; i<numModes ; i++ )
		{
			if( j < pTconInfo->modeData[i].numReg )
			{
				fprintf(pFd, "%d\t", pTconInfo->modeData[i].regData[j]);
			}
			else
			{
				fprintf(pFd, "0\t");
			}
		}
		fprintf(pFd, "\n");
	}
	fclose( pFd );
}

void CNX_EEPRomDataParser::WriteTconInfo( TCON_EEPROM_INFO *pTconInfo, const char *outputPath )
{
	char prefix[MAX_PATH];

#ifdef WIN32
	//	DGAM0
	sprintf( prefix, "%s\\LUT\\DGAM0", outputPath );
	WriteGamaInfo( &pTconInfo->DGAM0, prefix );
	//	DGAM1
	sprintf( prefix, "%s\\LUT\\DGAM1", outputPath );
	WriteGamaInfo( &pTconInfo->DGAM1, prefix );
	////	TGAM0
	//sprintf( prefix, "%s\\LUT\\TGAM0", outputPath );
	//WriteGamaInfo( &pTconInfo->TGAM0, prefix );
	//	TGAM1
	sprintf( prefix, "%s\\LUT\\TGAM1", outputPath );
	WriteGamaInfo( &pTconInfo->TGAM1, prefix );

	//	TREG
	sprintf( prefix, "%s\\T_REG.txt", outputPath );
	WriteModeInfo( pTconInfo, prefix );

#else
	sprintf( prefix, "%s/LUT", outputPath );
	mkdir(prefix, 0777);
	//	DGAM0
	sprintf( prefix, "%s/LUT/DGAM0", outputPath );
	WriteGamaInfo( &pTconInfo->DGAM0, prefix );
	//	DGAM1
	sprintf( prefix, "%s/LUT/DGAM1", outputPath );
	WriteGamaInfo( &pTconInfo->DGAM1, prefix );
	////	TGAM0
	//sprintf( prefix, "%s/LUT/TGAM0", outputPath );
	//WriteGamaInfo( &pTconInfo->TGAM0, prefix );
	//	TGAM1
	sprintf( prefix, "%s/LUT/TGAM1", outputPath );
	WriteGamaInfo( &pTconInfo->TGAM1, prefix );

	//	TREG
	sprintf( prefix, "%s/T_REG.txt", outputPath );
	WriteModeInfo( pTconInfo, prefix );
#endif
}
