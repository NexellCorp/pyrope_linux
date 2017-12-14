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

#ifndef __CNX_EEPROMDATAPARSER_H__
#define __CNX_EEPROMDATAPARSER_H__

#include <stdint.h>

typedef struct tagEEPROM_HEADER {
	uint8_t		update;
	uint32_t	verAddr;
	uint32_t	DGAM0Addr;
	uint32_t	DGAM1Addr;
	uint32_t	TGAM1Addr;
	uint32_t	modeAddr;
} EEPROM_HEADER;

typedef struct tagGAMMA_INFO {
	uint32_t	GAMMA_R[4096];
	uint32_t	GAMMA_G[4096];
	uint32_t	GAMMA_B[4096];
} GAMMA_INFO;

typedef struct tagMODE_DATA_INFO {
	char		signiture[6];
	uint8_t		use;
	char		description[32];
	uint8_t		numReg;
	uint16_t	regAddr[255];
	uint16_t	regData[255];
} MODE_DATA_INFO;

typedef struct tagMODE_HEADER_INFO {
	int32_t numModes;
	int32_t modeAddr[10];
	int32_t modeXor;
} MODE_HEADER_INFO;

typedef struct tagTCON_EEPROM_INFO{
	uint8_t				header[19];
	uint8_t				version[257];
	GAMMA_INFO			DGAM0;
	GAMMA_INFO			DGAM1;
	GAMMA_INFO			TGAM0;
	GAMMA_INFO			TGAM1;
	MODE_HEADER_INFO	modeHeader;			//	Parsed Mode Header Information
	MODE_DATA_INFO		modeData[10];
} TCON_EEPROM_INFO;


/*
	ITEM             Bytes			Base Address
	-----------------------------------------------------
	MODE9			max.1060
	...				...
	MODE1			max.1060
	MODE0			max.1060
	MODE_HEADER		34				0x18000
	TGAM1			27660			0x10000
	DGAM1			27660			0x9000
	DGAM0			27660			0x200
	VERSION			257				0x14
	HEADER			19				0x0
*/

class CNX_EEPRomDataParser {
public:
	CNX_EEPRomDataParser();
	~CNX_EEPRomDataParser();

	//	for file input mode
	int32_t Init( const char *inFile );
	//	for bffer input mode : bufSize shoud be 128KB
	int32_t Init( void *pBuf, int32_t bufSize );
	int32_t Parse( TCON_EEPROM_INFO **ppOutInfo );
	int32_t ParseVersion( TCON_EEPROM_INFO **ppOutInfo );
	void Deinit();
private:
	void ParseModeData( uint8_t *pInBuf, MODE_DATA_INFO *pOutInfo );
	void ParseGammaData( uint8_t *pInBuf, GAMMA_INFO *pOutGamma );


private:
	void	*m_pInBuffer;
	int32_t	m_nInBufferSize;
	void	*m_pOutBuffer;
	int32_t	m_nOutBufSize;
	TCON_EEPROM_INFO *m_pTconInfo;
	int32_t	m_InputMode;


	//////////////////////////////////////////////////////////////////////////
	//																		//
	//						Output Writing Routine							//
	//																		//
	//////////////////////////////////////////////////////////////////////////
public:
	void WriteTconInfo( TCON_EEPROM_INFO *pTconInfo, const char *outputPath );
private:
	void WriteModeInfo(TCON_EEPROM_INFO *pTconInfo, const char *outFileName);
	void WriteGamaInfo(GAMMA_INFO *pGamaInfo, const char *outFilePreFix);

	//////////////////////////////////////////////////////////////////////////
	//																		//
	//							Enumerator									//
	//																		//
	//////////////////////////////////////////////////////////////////////////
public:
	enum {
		ERR_NOT_INIT	= -7,
		ERR_MODE_HDR	= -6,
		ERR_HEADER		= -5,
		ERR_PARSING		= -4,
		ERR_IN_BUF		= -3,
		ERR_IN_FILE		= -2,
		ERR_ERROR		= -1,
		ERR_NONE		=  0,
	};

private:
	enum{
		INPUT_MODE_NONE,
		INPUT_MODE_FILE,
		INPUT_MODE_BUFFER,
	};

	enum {
		SIZE_HEADER			= 19,
		SIZE_VERSION		= 257,
		SIZE_DGAM			= 27660,
		SIZE_TGAM			= 27660,
		SIZE_MODE_HEADER	= 34,
		SIZE_MODE_MAX		= 1060,
	};
	//	Data Start Offset
	enum {
		OFFSET_HEADER		= 0x0,
		OFFSET_VERSION		= 0x14,
		OFFSET_DGAM0		= 0x200,
		OFFSET_DGAM1		= 0x9000,
		OFFSET_TGAM1		= 0x10000,
		OFFSET_MODE_HEADER	= 0x18000,
	};
};

#endif	//__CNX_EEPROMDATAPARSER_H__
