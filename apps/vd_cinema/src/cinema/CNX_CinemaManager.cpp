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
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <CNX_Base.h>
#include <CNX_I2C.h>
#include <CNX_EEPRom.h>
#include <CNX_EEPRomData.h>

#include <NX_Utils.h>
#include <NX_CinemaCommand.h>
#include <NX_Version.h>

#include "CNX_File.h"
#include "CNX_CinemaManager.h"
#include "NX_CinemaRegister.h"

#define NX_DTAG	"[NX_CinemaControl]"
#include <NX_DbgMsg.h>

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

//------------------------------------------------------------------------------
#define MAX_PAYLOAD_SIZE				65535

#define TCON_CABINET( A )				((A & 0x7F) - 16)
#define TCON_I2C_PORT( A )				((A & 0x80) >> 7)
#define TCON_I2C_SLAVE( A )				((A & 0x7F))

#define TCON_BURST_RETRY_COUNT			3
#define MAX_LOD_MODULE					12

#define TCON_USB_TREG_PATH				"/storage/sdcard0/SAMSUNG/TCON_USB"
#define TCON_USB_TREG_FILE				"/storage/sdcard0/SAMSUNG/TCON_USB/T_REG.txt"
#define TCON_USB_GAMMA_PATH				"/storage/sdcard0/SAMSUNG/TCON_USB/LUT"

#define TCON_EEPROM_TREG_PATH			"/storage/sdcard0/SAMSUNG/TCON_EEPROM"
#define TCON_EEPROM_TREG_FILE			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.txt"
#define TCON_EEPROM_TREG_BINARY			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.bin"
#define TCON_EEPROM_GAMMA_PATH			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/LUT"

#define TCON_GAMMA_PATTERN				"(T|D)GAM(0|1)_(R|G|B).txt"

#define PFPGA_USB_PREG_PATH				"/storage/sdcard0/SAMSUNG/PFPGA"
#define PFPGA_USB_PREG_FILE				"/storage/sdcard0/SAMSUNG/PFPGA/P_REG.txt"
#define PFPGA_USB_UNIFORMITY_FILE		"/storage/sdcard0/SAMSUNG/PFPGA/UC_COEF.txt"

#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

#define I2C_SEPARATE_BURST 				1

//------------------------------------------------------------------------------
static int32_t	TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t	TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t	TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t	TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t	TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t	TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );

static void		MakeDirectory( const char *pDir );
static int32_t	SendRemote( const char *pSock, const char *pMsg );

//------------------------------------------------------------------------------
CNX_CinemaManager::CNX_CinemaManager()
	: m_hThreadCommand( 0x00 )
	, m_bRun( false )
	, m_iCmd( 0 )
	, m_pPreference( NULL )
	, m_pTReg( NULL )
	, m_pPReg( NULL )
	, m_pGamma( NULL )
	, m_pUniformity( NULL )
{
	memset( m_NapVersion, 0x00, sizeof(m_NapVersion) );
	memset( m_SapVersion, 0x00, sizeof(m_SapVersion) );
	memset( m_IpcVersion, 0x00, sizeof(m_IpcVersion) );

	MakeDirectory( TCON_EEPROM_TREG_PATH );
	MakeDirectory( TCON_USB_TREG_PATH );
	MakeDirectory( PFPGA_USB_PREG_PATH );

	m_pPreference = new CNX_Preference();
	m_pTReg = new CNX_TReg();
	m_pPReg = new CNX_PReg();
	m_pGamma = new CNX_Gamma();
	m_pUniformity = new CNX_Uniformity();

	pthread_attr_t hAttr;
	pthread_attr_init( &hAttr );
	pthread_attr_setdetachstate( &hAttr, PTHREAD_CREATE_DETACHED );
	pthread_create( &m_hThreadCommand, &hAttr, this->ThreadInitialStub, this );
	pthread_attr_destroy( &hAttr );
}

//------------------------------------------------------------------------------
CNX_CinemaManager::~CNX_CinemaManager()
{
	if( m_hThreadCommand )
	{
		pthread_join( m_hThreadCommand, NULL );
		m_hThreadCommand = 0x00;
	}

	if( m_pPreference )	delete m_pPreference;
	if( m_pTReg )		delete m_pTReg;
	if( m_pPReg )		delete m_pPReg;
	if( m_pGamma )		delete m_pGamma;
	if( m_pUniformity )	delete m_pUniformity;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::Init()
{
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::Deinit()
{
	return 0;
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetNapVersion( const char *pVersion )
{
	memset( m_NapVersion, 0x00, sizeof(m_NapVersion) );
	snprintf( (char*)m_NapVersion, sizeof(m_NapVersion), "%s", pVersion );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetSapVersion( const char *pVersion )
{
	memset( m_SapVersion, 0x00, sizeof(m_SapVersion) );
	snprintf( (char*)m_SapVersion, sizeof(m_SapVersion), "%s", pVersion );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetIpcVersion( const char *pVersion )
{
	memset( m_IpcVersion, 0x00, sizeof(m_IpcVersion) );
	snprintf( (char*)m_IpcVersion, sizeof(m_IpcVersion), "%s", pVersion );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetTmsVersion( const char *pVersion )
{
	memset( m_TmsVersion, 0x00, sizeof(m_TmsVersion) );
	snprintf( (char*)m_TmsVersion, sizeof(m_TmsVersion), "%s", pVersion );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::GetSlave()
{
	memset( m_iSlave, 0x00, sizeof(m_iSlave) );
	m_iSlaveNum = 0;

	for( uint8_t i = 0x00; i < 0xFF; i++ )
	{
		if( (i & 0x7F) < 0x10 )
			continue;
#if 1
		uint8_t outBuf;
		int32_t iOutSize;
		ProcessCommand( TCON_CMD_STATUS, &i, 1, &outBuf, &iOutSize );
#else
		uint8_t outBuf;

		if( (i > 0x30) && (i < 0x35) ) outBuf = 0x00;
		else if( (i > 0xB4) && ( i < 0xBA) ) outBuf = 0x00;
		else outBuf = 0xFF;
#endif
		if( outBuf == 0xFF )
			continue;

		m_iSlave[m_iSlaveNum] = i;
		m_iSlaveNum++;
	}

	for( int32_t i = 0; i < m_iSlaveNum; i++ )
	{
		for( int32_t j = 0; j < m_iSlaveNum-1; j++ )
		{
			if( (m_iSlave[j] & 0x7F) > (m_iSlave[j+1] & 0x7F) )
			{
				uint8_t iTemp = m_iSlave[j];
				m_iSlave[j]   = m_iSlave[j+1];
				m_iSlave[j+1] = iTemp;
			}
		}
	}

	// for( int32_t i = 0; i < m_iSlaveNum; i++ )
	// {
	// 	printf("m_iSlave[ %d ] = %d ( 0x%02X )\n", i, m_iSlave[i], m_iSlave[i]);
	// }

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IsValidPort( int32_t iPort )
{
	for( int32_t i = 0; i < m_iSlaveNum; i++ )
	{
		if( iPort == ((m_iSlave[i] >> 7) & 0x01) )
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IsBusy()
{
	CNX_AutoLock lock( &m_hLock );
	return m_bRun;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::SendCommand( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	{
		CNX_AutoLock lock( &m_hLock );

		if( m_bRun == true )
		{
			printf("Fail, Resource busy.\n");

			*iOutSize  = 4;

			pOutBuf[0] = 0xFF;
			pOutBuf[1] = 0xFF;
			pOutBuf[2] = 0xFF;
			pOutBuf[3] = 0xFE;

			return NX_RET_RESOURCE_BUSY;
		}

		m_bRun = true;
	}

	m_iCmd     = iCmd;
	m_pInBuf   = pInBuf;
	m_iInSize  = iInSize;
	m_pOutBuf  = pOutBuf;
	m_iOutSize = 0;

	if( 0 != pthread_create( &m_hThreadCommand, NULL, this->ThreadCommandStub, this ) )
	{
		CNX_AutoLock lock( &m_hLock );
		printf( "Fail, pthread_create().\n" );

		*iOutSize  = 4;

		pOutBuf[0] = 0xFF;
		pOutBuf[1] = 0xFF;
		pOutBuf[2] = 0xFF;
		pOutBuf[3] = 0xFF;

		m_bRun = false;
		return NX_RET_ERROR;
	}

	if( m_hThreadCommand )
	{
		pthread_join( m_hThreadCommand, NULL );
		m_hThreadCommand = 0x00;
	}

	*iOutSize = m_iOutSize;

#if 0
	NX_HexDump( (void*)pInBuf, iInSize, "InBuf: " );
	NX_HexDump( (void*)pOutBuf, *iOutSize, "OutBuf: " );
#endif

	{
		CNX_AutoLock lock( &m_hLock );
		m_bRun = false;
	}

	return NX_RET_DONE;
}

//------------------------------------------------------------------------------
void *CNX_CinemaManager::ThreadInitialStub( void *pObj )
{
	if( NULL != pObj )
	{
		((CNX_CinemaManager*)pObj)->ThreadInitialProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
#define INT16( C0, C1 )						\
		( ((uint16_t)(uint8_t)(C1) <<  0 ) |\
		  ((uint16_t)(uint8_t)(C0) <<  8 ) )

#define INT32( C0, C1, C2, C3 )				\
		( ((uint16_t)(uint8_t)(C3) <<  0 ) |\
		  ((uint16_t)(uint8_t)(C2) <<  8 ) |\
		  ((uint16_t)(uint8_t)(C1) << 16 ) |\
		  ((uint16_t)(uint8_t)(C0) << 24 ) )

#define ARRAY_SIZE(A)	(int32_t)(sizeof(A) / sizeof(A[0]))

void CNX_CinemaManager::ThreadInitialProc()
{
	{
		CNX_AutoLock lock( &m_hLock );
		m_bRun = true;
	}

	int32_t iMode = 0;
	int32_t bBooting = true;

	GetSlave();

	//
	//	Initialize Sequence
	//
	uint8_t* pInBuf;
	uint8_t inBuf[MAX_PAYLOAD_SIZE];
	uint8_t outBuf[MAX_PAYLOAD_SIZE];
	int32_t iInSize  = 0;
	int32_t iOutSize = 0;
	char*	pValue = NULL;

	//	0. Get Mode
	m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
	m_pPreference->Read( CINEMA_KEY_INTIAL_MODE, &pValue );
	if( NULL != pValue ) iMode = atoi( pValue );
	m_pPreference->Close();

	//	1. Read EEPRom Data
	{
		ProcessCommand( TCON_CMD_EEPROM_READ, inBuf, iInSize, outBuf, &iOutSize );

		m_pTReg->Delete();
		m_pPReg->Delete();

		m_pTReg->Update( TCON_USB_TREG_FILE );
		m_pTReg->Update( TCON_EEPROM_TREG_FILE );
		m_pPReg->Update( PFPGA_USB_PREG_FILE );
	}

	//	2. Check Booting Status
	{

		for( int32_t i = 0; i < m_iSlaveNum; i++ )
		{
			inBuf[0] = m_iSlave[i];
			iInSize  = 1;

			ProcessCommand( TCON_CMD_BOOTING_STATUS, inBuf, iInSize, outBuf, &iOutSize );

			if( 0x01 != outBuf[0] )
			{
				printf("Fail, ProcessCommand(). ( cabinet: %d, port: %d, slave: %d )",
					TCON_CABINET(m_iSlave[i]), TCON_I2C_PORT(m_iSlave[i]), TCON_I2C_SLAVE(m_iSlave[i]) );

				bBooting = false;
			}
		}

		if( false == bBooting )
		{
			printf("Fail, TCON Booting.\n" );
			goto ERROR_INTIALIZE;
		}
	}

	//	3. PFPGA Mute On
	{
		inBuf[0] = 0x01;
		iInSize  = 1;

		ProcessCommand( PFPGA_CMD_MUTE, inBuf, iInSize, outBuf, &iOutSize );
	}

	//	4. Write P_REG.txt
	{
		uint16_t *pReg = m_pPReg->GetRegister(iMode);
		uint16_t *pDat = m_pPReg->GetData(iMode);

		for( int32_t i = 0; i < m_pPReg->GetDataNum(iMode); i++ )
		{
			pInBuf  = inBuf;
			iInSize = 0;

			*pInBuf++ = ((pReg[i] >> 8) & 0xFF);	iInSize++;
			*pInBuf++ = ((pReg[i] >> 0) & 0xFF);	iInSize++;
			*pInBuf++ = ((pDat[i] >> 8) & 0xFF);	iInSize++;
			*pInBuf++ = ((pDat[i] >> 0) & 0xFF);	iInSize++;

			ProcessCommand( PFPGA_CMD_REG_WRITE, inBuf, iInSize, outBuf, &iOutSize );
		}
	}

	//	5. Write Uniformity
	{
		if( m_pPReg->GetEnable(iMode) )
		{
			if( !m_pUniformity->Parse( PFPGA_USB_UNIFORMITY_FILE ) )
			{
				pInBuf = inBuf;
				iInSize = 0;

				uint16_t *pDat = m_pUniformity->GetData();
				for( int32_t i = 0; i < CNX_Uniformity::MAX_DATA_NUM; i++ )
				{
					*pInBuf++ = ((pDat[i] >> 8) && 0xFF); iInSize++;
					*pInBuf++ = ((pDat[i] >> 0) && 0xFF); iInSize++;
				}

				ProcessCommand( PFPGA_CMD_UNIFORMITY_DATA, inBuf, iInSize, outBuf, &iOutSize );
			}

		}
	}

	//	6. Write T_REG.txt
	{
		uint16_t *pReg = m_pTReg->GetRegister(iMode);
		uint16_t *pDat = m_pTReg->GetData(iMode);

		for( int32_t i = 0; i < m_pTReg->GetDataNum(iMode); i++ )
		{
			pInBuf  = inBuf;
			iInSize = 0;

			*pInBuf++ = 0x09;						iInSize++;
			*pInBuf++ = ((pReg[i] >> 8) & 0xFF);	iInSize++;
			*pInBuf++ = ((pReg[i] >> 0) & 0xFF);	iInSize++;
			*pInBuf++ = ((pDat[i] >> 8) & 0xFF);	iInSize++;
			*pInBuf++ = ((pDat[i] >> 0) & 0xFF);	iInSize++;

			for( int port = 0; port < 2; port++ )
			{
				if( IsValidPort(port) )
				{
					inBuf[0] = 0x09 | (port << 7);
					ProcessCommand( TCON_CMD_REG_WRITE, inBuf, iInSize, outBuf, &iOutSize );
				}
			}
		}
	}

	//	7. Write Gamma
	{
		char *pList[32];
		int32_t iListNum = 0;

		if( 10 > iMode )
		{
			CNX_File::GetList( TCON_EEPROM_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 1)) )
					{
						printf("Skip. Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pInBuf  = inBuf;
					iInSize = 0;

					*pInBuf++ = 0x09;					iInSize++;
					*pInBuf++ = m_pGamma->GetTable();	iInSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pInBuf++ = ((pDat[i] >> 16) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  8) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  0) & 0xFF);	iInSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInSize, outBuf, &iOutSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );
		}
		else
		{
			CNX_File::GetList( TCON_EEPROM_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 1)) )
					{
						printf("Skip. Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pInBuf  = inBuf;
					iInSize = 0;

					*pInBuf++ = 0x09;					iInSize++;
					*pInBuf++ = m_pGamma->GetTable();	iInSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pInBuf++ = ((pDat[i] >> 16) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  8) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  0) & 0xFF);	iInSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInSize, outBuf, &iOutSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );

			CNX_File::GetList( TCON_USB_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 2)) )
					{
						printf("Skip. Update USB Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pInBuf  = inBuf;
					iInSize = 0;

					*pInBuf++ = 0x09;					iInSize++;
					*pInBuf++ = m_pGamma->GetTable();	iInSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pInBuf++ = ((pDat[i] >> 16) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  8) & 0xFF);	iInSize++;
						*pInBuf++ = ((pDat[i] >>  0) & 0xFF);	iInSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInSize, outBuf, &iOutSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );
		}
	}

	//	8. Update Gamma Status For Skip
	{
		char szGamma[4][2];
		snprintf( szGamma[0], sizeof(szGamma[0]), "%d", m_pTReg->GetGammaType(iMode)[0] );
		snprintf( szGamma[1], sizeof(szGamma[1]), "%d", m_pTReg->GetGammaType(iMode)[1] );
		snprintf( szGamma[2], sizeof(szGamma[2]), "%d", m_pTReg->GetGammaType(iMode)[2] );
		snprintf( szGamma[3], sizeof(szGamma[3]), "%d", m_pTReg->GetGammaType(iMode)[3] );

		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
		m_pPreference->Write( CINEMA_KEY_UPDATE_TGAM0, szGamma[0]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_TGAM1, szGamma[1]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_DGAM0, szGamma[2]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_DGAM1, szGamma[3]);
		m_pPreference->Close();
	}

	//	9. SW Reset
	{
		for( int32_t port = 0; port < 2; port++ )
		{
			if( IsValidPort(port) )
			{
				inBuf[0] = 0x09 | (port << 7);
				iInSize  = 1;

				ProcessCommand( TCON_CMD_SW_RESET, inBuf, iInSize, outBuf, &iOutSize );
			}
		}
	}

	//	10. Write Global Register
	{
		int32_t treg[] = {
			0x018B, 0x018C, 0x018A, 0x018D, 0x018E,
			0x0192, 0x0055, 0x0004, 0x0100,	0x011E,
		};

		int32_t preg[] = {
			0x0199,
		};

		char szKey[256];
		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );

		for( int32_t i = 0; i < ARRAY_SIZE(treg); i++ )
		{
			char *pValue = NULL;
			snprintf(szKey, sizeof(szKey), "treg.0x%04x", treg[i]);
			m_pPreference->Read( szKey, &pValue );
			if( NULL != pValue )
			{
				int32_t iValue = atoi( pValue );

				pInBuf  = inBuf;
				iInSize = 0;

				*pInBuf++ = 0x09;						iInSize++;
				*pInBuf++ = ((treg[i] >> 8) & 0xFF);	iInSize++;
				*pInBuf++ = ((treg[i] >> 0) & 0xFF);	iInSize++;
				*pInBuf++ = ((iValue  >> 8) & 0xFF);	iInSize++;
				*pInBuf++ = ((iValue  >> 0) & 0xFF);	iInSize++;

				for( int port = 0; port < 2; port++ )
				{
					if( IsValidPort(port) )
					{
						inBuf[0] = 0x09 | (port << 7);
						ProcessCommand( TCON_CMD_REG_WRITE, inBuf, iInSize, outBuf, &iOutSize );
					}
				}
			}
		}

		for( int32_t i = 0 ; i < ARRAY_SIZE(preg); i++ )
		{
			char *pValue = NULL;
			snprintf(szKey, sizeof(szKey), "preg.0x%04x", preg[i]);
			m_pPreference->Read( szKey, &pValue );
			if( NULL != pValue )
			{
				int32_t iValue = atoi( pValue );

				pInBuf  = inBuf;
				iInSize = 0;

				*pInBuf++ = ((preg[i] >> 8) & 0xFF);	iInSize++;
				*pInBuf++ = ((preg[i] >> 0) & 0xFF);	iInSize++;
				*pInBuf++ = ((iValue  >> 8) & 0xFF);	iInSize++;
				*pInBuf++ = ((iValue  >> 0) & 0xFF);	iInSize++;

				ProcessCommand( PFPGA_CMD_REG_WRITE, inBuf, iInSize, outBuf, &iOutSize );
			}
		}

		m_pPreference->Close();
	}

	//	11. PFPGA Mute Off
	{
		inBuf[0] = 0x00;
		iInSize  = 1;

		ProcessCommand( PFPGA_CMD_MUTE, inBuf, iInSize, outBuf, &iOutSize );
	}

	//	12. TCON Initialize
	{
		for( int port = 0; port < 2; port++ )
		{
			if( IsValidPort(port) )
			{
				inBuf[0] = 0x09 | (port << 7);
				ProcessCommand( TCON_CMD_INIT, inBuf, iInSize, outBuf, &iOutSize );
			}
		}
	}

ERROR_INTIALIZE:
	{
		CNX_AutoLock lock( &m_hLock );
		m_bRun = false;
	}
}

//------------------------------------------------------------------------------
void *CNX_CinemaManager::ThreadCommandStub( void *pObj )
{
	if( NULL != pObj )
	{
		((CNX_CinemaManager*)pObj)->ThreadCommandProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::ThreadCommandProc()
{
	ProcessCommand( m_iCmd, m_pInBuf, m_iInSize, m_pOutBuf, &m_iOutSize );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::ProcessCommand( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	switch( iCmd )
	{
	case TCON_CMD_REG_WRITE:								return TCON_RegWrite( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_REG_READ:									return TCON_RegRead( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_REG_BURST_WRITE:							return TCON_RegBurstWrite( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_INIT:										return TCON_Init( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_STATUS:									return TCON_Status( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_DOOR_STATUS:								return TCON_DoorStatus( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_LVDS_STATUS:								return TCON_LvdsStatus( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_BOOTING_STATUS:							return TCON_BootingStatus( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_MODE_NORMAL:								return TCON_LedModeNormal( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_MODE_LOD:									return TCON_LedModeLod( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_OPEN_NUM:									return TCON_LedOpenNum( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_OPEN_POS:									return TCON_LedOpenPos( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:								return TCON_TestPattern( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_TGAM_R:
	case TCON_CMD_TGAM_G:
	case TCON_CMD_TGAM_B:									return TCON_TargetGamma( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_DGAM_R:
	case TCON_CMD_DGAM_G:
	case TCON_CMD_DGAM_B:									return TCON_DeviceGamma( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_DOT_CORRECTION:							return TCON_DotCorrection( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_DOT_CORRECTION_EXTRACT:					return TCON_DotCorrectionExtract( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_WHITE_SEAM_READ:							return TCON_WhiteSeamRead( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_WHITE_SEAM_WRITE:							return TCON_WhiteSeamWrite( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_ELAPSED_TIME:								printf("Not Implemataion.\n");	return 0;
	case TCON_CMD_ACCUMULATE_TIME:							printf("Not Implemataion.\n");	return 0;
	case TCON_CMD_SW_RESET:									return TCON_SwReset( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_EEPROM_READ:								return TCON_EEPRomRead( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_VERSION:									return TCON_Version( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_REG_WRITE:
	case CMD_PFPGA( PFPGA_CMD_REG_WRITE ):					return PFPGA_RegWrite( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_REG_READ:
	case CMD_PFPGA( PFPGA_CMD_REG_READ ):					return PFPGA_RegRead( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_STATUS:
	case CMD_PFPGA( PFPGA_CMD_STATUS ):						return PFPGA_Status( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_MUTE:
	case CMD_PFPGA( PFPGA_CMD_MUTE ):						return PFPGA_Mute( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_UNIFORMITY_DATA:
	case CMD_PFPGA( PFPGA_CMD_UNIFORMITY_DATA ):			return PFPGA_UniformityData( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PFPGA_CMD_VERSION:
	case CMD_PFPGA( PFPGA_CMD_VERSION ):					return PFPGA_Version( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_NAP_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_NAP_VERSION ):			return PLAT_NapVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_SAP_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_SAP_VERSION ):			return PLAT_SapVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_IPC_SERVER_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_IPC_SERVER_VERSION ):	return PLAT_IpcVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_TMS_SERVER_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_TMS_SERVER_VERSION ):	return PLAT_TmsVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_IS_BUSY:
	case CMD_PLATFORM( PLATFORM_CMD_IS_BUSY ):				return PLAT_IsBusy( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_CHANGE_MODE:
	case CMD_PLATFORM( PLATFORM_CMD_CHANGE_MODE ):			return PLAT_ChangeMode( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case IMB_CMD_CHANGE_CONTENTS:
	case CMD_IMB( IMB_CMD_CHANGE_CONTENTS ):				return IMB_ChangeContents( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case IMB_CMD_CHANGE_MODE:
	case CMD_IMB( IMB_CMD_CHANGE_MODE ):					return IMB_ChangeMode( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case IMB_CMD_GET_CURRENT_MODE:
	case CMD_IMB( IMB_CMD_GET_CURRENT_MODE ):				return IMB_GetCurrentMode( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case IMB_CMD_WRITE_FILE:
	case CMD_IMB( IMB_CMD_WRITE_FILE ):						return IMB_WriteFile( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case IMB_CMD_READ_MODE_DESC:
	case CMD_IMB( IMB_CMD_READ_MODE_DESC ):					return IMB_ReadModeDesc( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	default :
		break;
	}

	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_RegWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1] : msb reg, pInBuf[2] : lsb reg, pInBuf[3] : msb data, pInBuf[4] : lsb data
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pInBuf[1];
	uint8_t lsbReg	= pInBuf[2];
	uint8_t msbData	= pInBuf[3];
	uint8_t lsbData	= pInBuf[4];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	// printf( "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", port, slave, inReg, inData );

	result[0] = 0x00;
	result[1] = 0x00;
	result[2] = 0x00;
	result[3] = 0x00;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_RegRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1] : msb reg, pInBuf[2] : lsb reg
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pInBuf[1];
	uint8_t lsbReg	= pInBuf[2];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, iReadData );

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_RegBurstWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	//	Not Implemeation.
	//

	uint8_t result = 0xFF;

	if( 0 ) goto ERROR_TCON;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_Init( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA1, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA1, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA2, 0x0025 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA2, 0x0025 );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_Status( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iWriteData, iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	iWriteData = (int16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( slave, TCON_REG_CHECK_STATUS, (uint16_t*)&iWriteData, 1 ) )
	{
		// NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
		// 	port, slave, TCON_REG_CHECK_STATUS, iWriteData );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_STATUS )) )
	{
		// NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_CHECK_STATUS );
		goto ERROR_TCON;
	}

	result = ((iWriteData == iReadData) ? 1 : 0);
	NxDbgMsg( NX_DBG_INFO, "TCON Status. ( i2c-%d, slave: 0x%02X, status: %d )\n", port, slave, result );

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_DoorStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_DOOR_READ )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_DOOR_READ );
		goto ERROR_TCON;
	}

	result = (uint8_t)iReadData;

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 1 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 1 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 0 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 0 );
		goto ERROR_TCON;
	}

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_LvdsStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_LVDS_STATUS )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LVDS_STATUS );
		goto ERROR_TCON;
	}

	result = (iReadData == 1080 || iReadData == 2160) ? 1 : 0;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_BootingStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_BOOTING_STATUS )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_BOOTING_STATUS );
		goto ERROR_TCON;
	}

	result = (iReadData == 0x07E1 ) ? 1 : 0;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_LedModeNormal( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_PATTERN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_LedModeLod( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	const uint16_t pattern[] = {
		10,		0,		1024,	0,		2160,	4095,	4095,	4095
	};

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(pattern) / sizeof(pattern[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_PATTERN + i, pattern[i] ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_PATTERN + i, pattern[i] );
			goto ERROR_TCON;
		}
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );

		goto ERROR_TCON;
	}

	usleep( 3000000 );	// delay during 3.00sec over ( LOD Scan Time )

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_LedOpenNum( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t iReadData = 0;
	int32_t iErrorNum = 0;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_LOD_100_CHK_DONE ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LOD_100_CHK_DONE );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_ERROR_NUM_OVR ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_NUM_OVR );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		iReadData = i2c.Read( slave, TCON_REG_ERROR_NUM_M1 + i );
		if( 0 > iReadData )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_ERROR_NUM_M1 + i );
			goto ERROR_TCON;
		}

		iErrorNum += iReadData;
	}

	result[0] = (uint8_t)(iErrorNum >> 24) & 0xFF;
	result[1] = (uint8_t)(iErrorNum >> 16) & 0xFF;
	result[2] = (uint8_t)(iErrorNum >>  8) & 0xFF;
	result[3] = (uint8_t)(iErrorNum >>  0) & 0xFF;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );;

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_LedOpenPos( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t iCoordinateX = 0, iCoordinateY = 0;
	int32_t iErrorOutReady;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	usleep( 50000 );

	iErrorOutReady = i2c.Read( slave, TCON_REG_ERROR_OUT_RDY );
	if( iErrorOutReady )
	{
		iCoordinateX = i2c.Read( slave, TCON_REG_X_COORDINATE );
		iCoordinateY = i2c.Read( slave, TCON_REG_Y_COORDINATE );

		result[0] = (uint8_t)((iCoordinateX >> 8 ) & 0xFF);
		result[1] = (uint8_t)((iCoordinateX >> 0 ) & 0xFF);
		result[2] = (uint8_t)((iCoordinateY >> 8 ) & 0xFF);
		result[3] = (uint8_t)((iCoordinateY >> 0 ) & 0xFF);
	}

	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0001 );
	usleep( 50000 );
	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0000 );

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_TestPattern( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iInSize );

	//
	//	pBuf[0] : index, pBuf[1] : function index, pBuf[2] : pattern index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t funcIndex = pInBuf[1];
	uint8_t patternIndex = pInBuf[2];

	int32_t (*pTestPatternFunc[6])( CNX_I2C*, uint8_t, uint8_t );
	pTestPatternFunc[0] = &TestPatternDci;
	pTestPatternFunc[1] = &TestPatternColorBar;
	pTestPatternFunc[2] = &TestPatternFullScreenColor;
	pTestPatternFunc[3] = &TestPatternGrayScale;
	pTestPatternFunc[4] = &TestPatternDot;
	pTestPatternFunc[5] = &TestPatternDiagonal;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( TCON_CMD_PATTERN_RUN == iCmd )
	{
		if( funcIndex != sizeof(pTestPatternFunc)/sizeof(pTestPatternFunc[0]) )
		{
			pTestPatternFunc[funcIndex](&i2c, index, patternIndex);
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0001 ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );
			}
#endif
		}
	}
	else
	{
		if( 0 > i2c.Write( slave, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_XYZ_TO_RGB, 0x0000 );
			goto ERROR_TCON;
		}

		if( funcIndex != sizeof(pTestPatternFunc)/sizeof(pTestPatternFunc[0]) )
		{
			if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_PATTERN, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_PATTERN )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_PATTERN );
				}

				NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );
			}
#endif
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );
			}
#endif
		}
	}

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t	CNX_CinemaManager::TCON_TargetGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1] .. : data ( num of data : iInSize - 1 )
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pInBuf + 1;
	int32_t size	= iInSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_TGAM_R_WDATA + (iCmd - TCON_CMD_TGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0001 << (iCmd - TCON_CMD_TGAM_R);

	int32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[%d] [ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	i, oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize / 4 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t	CNX_CinemaManager::TCON_DeviceGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	//
	//	pInBuf[0] : index, pInBuf[1] .. : data ( num of data : iInSize - 1 )
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pInBuf + 1;
	int32_t size	= iInSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_DGAM_R_WDATA + (iCmd - TCON_CMD_DGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0008 << (iCmd - TCON_CMD_DGAM_R);

	int32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 4; i++ )
		{
			if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, dataReg, iDataSize / 4 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_DotCorrection( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );

	uint64_t iCurTime = NX_GetTickCount();

	//
	//	pInBuf[0] : index, pInBuf[1] : module, pInBuf[2] .. : data ( num of data : iInSize - 1 )
	//
	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t module = pInBuf[1];
	uint8_t *ptr	= pInBuf + 2;
	int32_t size	= iInSize - 2;

	int32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t ccData[9] = { 0x0000, };
	uint16_t opData[16] = { 0x0000, };

	uint16_t iCheckSum = 0;
	int32_t iRetryCount = 0, bFail = false;

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(ptr[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(ptr[2 * i + 1] << 0) & 0x00FF);
		iCheckSum = iCheckSum ^ pData[i];
	}

#if 0	// For Debugging.
	uint16_t *pOriData = (uint16_t*)malloc( sizeof(uint16_t) * 64 * 60 * 9 );
	int offset = 0, oriOffset = 0;

	for( int32_t i = 0; i < 64 * 60; i++ )
	{
        pOriData[oriOffset    ] = ((pData[offset + 7]      ) & 0x3FFF);
        pOriData[oriOffset + 1] = ((pData[offset + 6] >>  2) & 0x3FFF);
        pOriData[oriOffset + 2] = ((pData[offset + 6] << 12) & 0x3000) | ((pData[offset + 5] >>  4) & 0x0FFF);
        pOriData[oriOffset + 3] = ((pData[offset + 5] << 10) & 0x3C00) | ((pData[offset + 4] >>  6) & 0x03FF);
        pOriData[oriOffset + 4] = ((pData[offset + 4] <<  8) & 0x3F00) | ((pData[offset + 3] >>  8) & 0x00FF);
        pOriData[oriOffset + 5] = ((pData[offset + 3] <<  6) & 0x3FC0) | ((pData[offset + 2] >> 10) & 0x003F);
        pOriData[oriOffset + 6] = ((pData[offset + 2] <<  4) & 0x3FF0) | ((pData[offset + 1] >> 12) & 0x000F);
        pOriData[oriOffset + 7] = ((pData[offset + 1] <<  2) & 0x3FFC) | ((pData[offset    ] >> 14) & 0x0003);
        pOriData[oriOffset + 8] = ((pData[offset    ]      ) & 0x3FFF);

		printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			pOriData[oriOffset + 0], pOriData[oriOffset + 1], pOriData[oriOffset + 2], pOriData[oriOffset + 3],
			pOriData[oriOffset + 4], pOriData[oriOffset + 5], pOriData[oriOffset + 6], pOriData[oriOffset + 7],
			pOriData[oriOffset + 8] );

        oriOffset += 9;
        offset += 8;
	}

	if( pOriData ) free( pOriData );
#endif

	printf("port(%d), slave(0x%02x), module(%d)\n", port, slave, module);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	// 0. Driver Clock Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Read CC data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module >> 1 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module >> 1 );
		goto ERROR_TCON;
	}
	usleep(100000);

	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_CC00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_CC00_READ + i );
			goto ERROR_TCON;
		}

		ccData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}

	//
	//	2. Read Optional Data
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		opData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	//
	//	3. Flash Protection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	4. Erase Flash.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(4000000);

	//
	//	5. Write Dot Correction Data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0040) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0040 );
		goto ERROR_TCON;
	}

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
#if I2C_SEPARATE_BURST
		for( int32_t i = 0; i < 30; i++ )
		{
			if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData + iDataSize / 30 * i, iDataSize / 30) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
					port, slave, TCON_REG_FLASH_WDATA, iDataSize / 30 );
				goto ERROR_TCON;
			}
		}
#else
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData, iDataSize ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_WDATA, iDataSize );
			goto ERROR_TCON;
		}
#endif
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			break;
		}
		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		if( iRetryCount == 0 ) {
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

	//
	//	6. Write Screen Correction Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_CC00 + i, ccData[i] ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CC00 + i, ccData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(500000);

	//
	//	7. Write Optional Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[i] ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00 + i, opData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(500000);

	//
	//	8. Flash Protection On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	if( pData ) free( pData );

	printf(">>>>> %s() Done : %lld ms\n", __FUNCTION__, NX_GetTickCount() - iCurTime );

	return 0;

}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_DotCorrectionExtract( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	uint64_t iStartTime = NX_GetTickCount();

	//
	//	pInBuf[0] : index, pInBuf[1] : module index
	//
	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pInBuf[1];
	uint16_t nBaseX = module % 4 * 64;
	uint16_t nBaseY = module / 4 * 60;

	int32_t iResultLen = 64 * 60 * 8 * 2;
	uint8_t *pResult = (uint8_t*)malloc( iResultLen * sizeof(uint8_t) );
	memset( pResult, 0x00, iResultLen * sizeof(uint8_t) );

	uint8_t *pPtr = pResult;

	int32_t cnt = 0;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	printf("port(%d), slave(%d), modlue(%d)\n", port, slave, module);

	for( int32_t i = 0; i < 60; i++ )
	{
		for( int32_t j = 0; j < 64; j++ )
		{
			int16_t ccData14[9] = { 0x0000, };
			int16_t ccData16[8] = { 0x0000, };

#if 1
			uint16_t pos_x = j + nBaseX;
			uint16_t pos_y = i + nBaseY;

			// printf("pos_x(%d), pos_y(%d)\n", pos_x, pos_y);

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_X, pos_x ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_X, pos_x );
				goto ERROR_TCON;
			}

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_Y, pos_y ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_Y, pos_y );
				goto ERROR_TCON;
			}

			usleep(50000);

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				if( 0 > (ccData14[k] = i2c.Read( slave, TCON_REG_ADDR_CC_IN_CC00 + k )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, slave, TCON_REG_ADDR_CC_IN_CC00 + k );
					goto ERROR_TCON;
				}
				ccData14[k] &= 0x3FFF;
			}
#else
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				ccData14[k] = cnt;		// 0101 0101 0101 0101
				ccData14[k] &= 0x3FFF;	// 0001 0101 0101 0101

				cnt = (cnt + 1) % 0x3FFF;
			}
#endif

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData14[0], ccData14[1], ccData14[2], ccData14[3], ccData14[4],
			// 	ccData14[5], ccData14[6], ccData14[7], ccData14[8] );

			//
			//	Convert Data ( 14bit x 9EA --> 16bit x 8EA )
			//
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				ccData16[0] = ((ccData14[7] & 0x0003) << 14) | ((ccData14[8] & 0x3FFF) >>  0);
				ccData16[1] = ((ccData14[6] & 0x000F) << 12) | ((ccData14[7] & 0x3FFC) >>  2);
				ccData16[2] = ((ccData14[5] & 0x003F) << 10) | ((ccData14[6] & 0x3FF0) >>  4);
				ccData16[3] = ((ccData14[4] & 0x00FF) <<  8) | ((ccData14[5] & 0x3FC0) >>  6);
				ccData16[4] = ((ccData14[3] & 0x03FF) <<  6) | ((ccData14[4] & 0x3F00) >>  8);
				ccData16[5] = ((ccData14[2] & 0x0FFF) <<  4) | ((ccData14[3] & 0x3C00) >> 10);
				ccData16[6] = ((ccData14[1] & 0x3FFF) <<  2) | ((ccData14[2] & 0x3000) >> 12);
				ccData16[7] =                                  ((ccData14[0] & 0x3FFF) >>  0);
			}

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData16[0], ccData16[1], ccData16[2], ccData16[3],
			// 	ccData16[4], ccData16[5], ccData16[6], ccData16[7] );

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				pPtr[k * 2    ] = (uint8_t)((ccData16[k] >> 8) & 0xFF);
				pPtr[k * 2 + 1] = (uint8_t)((ccData16[k] >> 0) & 0xFF);
			}
			pPtr += 16;

			//	Print Progress Debugging
			fprintf( stdout, "> %4d / %4d ( %3d %% )\r",
				i * 64 + j + 1, 60 * 64,
				(int)((float)((i * 64) + j + 1) / (float)(64 * 60) * (float)100) );
			fflush( stdout );
		}
	}

ERROR_TCON:
	*iOutSize = iResultLen;
	memcpy( pOutBuf, pResult, *iOutSize );

	if( pResult ) free( pResult );

	printf( "\n>> DotCorrection Extract Done. ( %lld mSec )\n", NX_GetTickCount() - iStartTime );
	return 0;

}

//------------------------------------------------------------------------------
#define IS_TCON_MODULE_TOP(module)		(((module / 4) == 0) ? true : false)
#define IS_TCON_MODULE_BOTTOM(module)	(((module / 4) == 5) ? true : false)
#define IS_TCON_MODULE_LEFT(module)		(((module % 4) == 0) ? true : false)
#define IS_TCON_MODULE_RIGHT(module)	(((module % 4) == 3) ? true : false)

#define TCON_MODULE_TOP			0
#define TCON_MODULE_BOTTOM		1
#define TCON_MODULE_LEFT		2
#define TCON_MODULE_RIGHT		3

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_WhiteSeamRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result = 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t iSeamValue[4] = { 16384, 16384, 16384, 16384 };

	uint16_t iTempValue[4];
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	2. Read top seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_TOP] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_TOP] = iTempValue[TCON_MODULE_TOP];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] );
		goto ERROR_TCON;
	}

	//
	//	3. Read bottom seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_BOTTOM(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA01_READ )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA01_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_BOTTOM] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_BOTTOM] = iTempValue[TCON_MODULE_BOTTOM];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] );
		goto ERROR_TCON;
	}

	//
	//	4. Read left seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_LEFT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA02_READ )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_LEFT] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_LEFT] = iTempValue[TCON_MODULE_LEFT];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] );
		goto ERROR_TCON;
	}

	//
	//	5. Read right seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_RIGHT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA03_READ )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_RIGHT] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_RIGHT] = iTempValue[TCON_MODULE_RIGHT];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;

}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_WhiteSeamWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1:2] : top, pInBuf[3:4] : bottom, pInBuf[5:6] : left, pInBuf[7:8] : right
	//

	uint8_t result = 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t opData[24][16]	= { 0x0000, };
	uint16_t iSeamValue[4];

	for( int32_t i = 0; i < 4; i++ )
	{
		iSeamValue[i] = ((int16_t)(pInBuf[i*2+1] << 8) & 0xFF00) + (int16_t)pInBuf[i*2+2];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	2. Read Optional Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	2-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	2-2. Read Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00_READ + i );
				goto ERROR_TCON;
			}

			opData[module][i] = (uint16_t)(iReadData & 0x0000FFFF);
		}
	}

	//
	//	3. Modify Seam Value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) && !IS_TCON_MODULE_BOTTOM(module) &&
			!IS_TCON_MODULE_LEFT(module) && !IS_TCON_MODULE_RIGHT(module) )
		{
			continue;
		}

		if( IS_TCON_MODULE_TOP(module) )
		{
			opData[module][TCON_MODULE_TOP]    = iSeamValue[TCON_MODULE_TOP];

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
		}

		if( IS_TCON_MODULE_BOTTOM(module) )
		{
			opData[module][TCON_MODULE_BOTTOM]	= iSeamValue[TCON_MODULE_BOTTOM];

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
		}

		if( IS_TCON_MODULE_LEFT(module) )
		{
			opData[module][TCON_MODULE_LEFT]	= iSeamValue[TCON_MODULE_LEFT];

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
		}

		if( IS_TCON_MODULE_RIGHT(module) )
		{
			opData[module][TCON_MODULE_RIGHT]	= iSeamValue[TCON_MODULE_RIGHT];

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
		}
	}

	//
	//	4. Write Flash Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	4-0. Process Skip if NOT (IS_TCON_MODULE_TOP & IS_TCON_MODULE_BOTTOM & IS_TCON_MODULE_LEFT & IS_TCON_MODULE_RIGHT)
		//
		if( !IS_TCON_MODULE_TOP(module) && !IS_TCON_MODULE_BOTTOM(module) &&
			!IS_TCON_MODULE_LEFT(module) && !IS_TCON_MODULE_RIGHT(module) )
		{
			continue;
		}

		//
		//	4-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	4-2. Flash Protection Off.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		//
		//	4-3. Write Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] );
				goto ERROR_TCON;
			}
		}

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(500000);

		//
		//	4-4. Flash Protection On.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);
	}

	//
	//	5. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_AccumulateTime( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );
	UNUSED( pOutBuf );
	UNUSED( iOutSize );

	//
	//	pInBuf[0] : index, pInBuf[1] : module,
	//

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_OptionalData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1] : module
	//

	uint8_t result[32] = { 0xFF, };

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pInBuf[1];

	uint16_t opData[16] = { 0x0000, };

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	2. Pluse Generation for Flash Read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	//	3. Read Optional Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		opData[i] = (uint16_t)(iReadData & 0x0000FFFF);
		usleep(50000);
	}

	//
	//	4. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	Print Debug Message
	//
	printf(">> Optional Data Information : index(%d), module(%d)\n", index, module);

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		printf(" -. REG_F_LED_DATA%02d_READ : 0x%04x\n", i, opData[i]);
	}

	//
	//
	//
	for(int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		result[i * 2 + 0] = (uint8_t)((opData[i] >> 8) & 0xFF);
		result[i * 2 + 1] = (uint8_t)((opData[i] >> 0) & 0xFF);
	}

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

	return 0;

}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_SwReset( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	result = 0x01;

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_EEPRomRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	uint8_t result	= 0xFF;

	int32_t iAddr = 0, iReadSize;
	int32_t iRemainSize;

	uint8_t *pBuf = (uint8_t*)malloc( TCON_EEPROM_DATA_SIZE );
	uint8_t *pPtr;

	int32_t iRet;

	CNX_EEPRom eeprom;
	CNX_EEPRomData parser;
	TCON_EEPROM_INFO *pInfo = NULL;
	uint8_t version[TCON_EEPROM_MAX_VERSION_SIZE];

	FILE *pFile = NULL;
	int32_t bUpdate = true;

	//
	//	0. Check Version
	//
	if( !access( TCON_EEPROM_TREG_BINARY, F_OK) )
	{
		iAddr       = 0;
		pPtr        = pBuf;
		iRemainSize = TCON_EEPROM_VERSION_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n" );
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_VERSION_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_VERSION_SIZE );

		//
		//	0-1. EEPROM Version Parsing.
		//
		iRet = parser.Init( pBuf, TCON_EEPROM_VERSION_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		memcpy( version, pInfo->version, TCON_EEPROM_MAX_VERSION_SIZE );
		parser.Deinit();

		//
		//	0-2. Binary Version Parsing.
		//
		parser.Init( TCON_EEPROM_TREG_BINARY );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		// NX_HexDump( version, sizeof(version) );
		// NX_HexDump( pInfo->version, sizeof(version) );

		if( !strcmp( (char*)pInfo->version, (char*)version ) )
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version match.\n");
			bUpdate = false;
		}
		else
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version missmatch.\n");
		}

		parser.Deinit();
	}

	if( bUpdate )
	{
		//
		//	1. Read EEPRom Data.
		//
		iAddr       = 0;
		pPtr        = pBuf;
		iRemainSize = TCON_EEPROM_DATA_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n");
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_DATA_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_DATA_SIZE );

		//
		//	2. Make Binary File.
		//
		pFile = fopen( TCON_EEPROM_TREG_BINARY, "wb" );
		if( NULL == pFile )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Make Binary File. ( %s )\n", TCON_EEPROM_TREG_BINARY );
			goto ERROR_TCON;
		}

		fwrite( pBuf, 1, TCON_EEPROM_DATA_SIZE, pFile );
		fclose( pFile );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s )\n", TCON_EEPROM_TREG_BINARY );

		//
		//	3. Make T_REG_EEPROM.txt
		//
		iRet = parser.Init( pBuf, TCON_EEPROM_DATA_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.Parse( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Parse(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		parser.WriteTconInfo( pInfo, TCON_EEPROM_TREG_PATH );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s/T_REG_EEPROM.txt )\n", TCON_EEPROM_TREG_PATH );

		sync();

		result = 0x01;		// Update Successful.
	}
	else
	{
		result = 0x00;		// Update is not needed.
	}

	NxDbgMsg( NX_DBG_ERR, "%s(). ( result = 0x%02X )\n", __FUNCTION__, result );

ERROR_TCON:
	parser.Deinit();

	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TCON_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index
	//

	uint8_t result[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pInBuf[1];
	uint8_t lsbReg	= pInBuf[2];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iModeName, iTime;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iModeName = i2c.Read( slave, TCON_REG_FPGA_MODE_NAME )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_MODE_NAME );
		goto ERROR_TCON;
	}

	if( 0 > (iTime = i2c.Read( slave, TCON_REG_FPGA_TIME1 )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_TIME1 );
		goto ERROR_TCON;
	}

	result[0] = (uint8_t)((iModeName >> 24) & 0xFF);
	result[1] = (uint8_t)((iModeName >> 16) & 0xFF);
	result[2] = (uint8_t)((iModeName >>  8) & 0xFF);
	result[3] = (uint8_t)((iModeName >>  0) & 0xFF);

	result[4] = (uint8_t)((iTime >> 24) & 0xFF);
	result[5] = (uint8_t)((iTime >> 16) & 0xFF);
	result[6] = (uint8_t)((iTime >>  8) & 0xFF);
	result[7] = (uint8_t)((iTime >>  0) & 0xFF);

ERROR_TCON:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_RegWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : msb reg, pInBuf[1] : lsb reg, pInBuf[2] : msb data, pInBuf[3] : lsb data
	//

	uint8_t	result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pInBuf[0];
	uint8_t lsbReg	= pInBuf[1];
	uint8_t msbData	= pInBuf[2];
	uint8_t lsbData	= pInBuf[3];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	result[0] = 0x00;
	result[1] = 0x00;
	result[2] = 0x00;
	result[3] = 0x00;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_RegRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : msb reg, pInBuf[1] : lsb reg
	//

	uint8_t	result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pInBuf[0];
	uint8_t lsbReg	= pInBuf[1];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_INFO, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, iReadData );

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_RegBurstWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	//	Not Implemetation
	//

	uint8_t result = 0xFF;

	if( 0 ) goto ERROR_PFPGA;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_Status( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	//	pInBuf : not used
	//

	uint8_t result = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_CHECK_STATUS );
		goto ERROR_PFPGA;
	}

	result = ((PFPGA_VAL_CHECK_STATUS == iReadData) ? 1 : 0);

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_Mute( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : data
	//

	uint8_t result = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t data	= pInBuf[0];

	if( data == 0x00 )
		NX_WaitTime( 1500 );

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_MUTE, data ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_MUTE, data );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_DEBUG, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_MUTE, data );

	if( data == 0x01 )
		NX_WaitTime( 100 );	// 2frame with 24Hz

	result = 0x01;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_UniformityData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );

	//
	//	pInBuf[0] - : data
	//

	uint8_t result = 0xFF;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t *data	= pInBuf;
	int32_t size	= iInSize;

	int32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(data[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(data[2 * i + 1] << 0) & 0x00FF);
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0080) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0080 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, PFPGA_REG_NUC_WDATA, iDataSize / 4 );
			goto ERROR_PFPGA;
		}
	}
#else
	if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, PFPGA_REG_NUC_WDATA, iDataSize );
		goto ERROR_PFPGA;
	}
#endif

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

	result = 0x01;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	free( pData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PFPGA_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	//	pInBuf : not used
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_PF_VERSION )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_PF_VERSION );
		goto ERROR_PFPGA;
	}

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_NapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	*iOutSize = strlen((char*)m_NapVersion) + 1;
	memcpy( pOutBuf, m_NapVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_SapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	*iOutSize = strlen((char*)m_SapVersion) + 1;
	memcpy( pOutBuf, m_SapVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_IpcVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	*iOutSize = strlen((char*)m_IpcVersion) + 1;
	memcpy( pOutBuf, m_IpcVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_TmsVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	*iOutSize = strlen((char*)m_TmsVersion) + 1;
	memcpy( pOutBuf, m_TmsVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_IsBusy( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	pOutBuf[0] = 0x00;
	pOutBuf[1] = 0x00;
	pOutBuf[2] = 0x00;
	pOutBuf[3] = 0x00;
	*iOutSize  = 4;

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_ChangeMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	return IMB_ChangeMode( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IMB_ChangeContents( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
#if 0
	UNUSED( iCmd );
	UNUSED( iInSize );

	uint8_t result = 0xFF;

	int32_t iMode = pInBuf[0];
	if( iMode < 0x0A )
	{
		uint8_t sendData[2] = { 0x00, };
		snprintf( (char*)sendData, sizeof(sendData), "%d", iMode );

		if( !SendRemote( "cinema.change.contents", (const char*)sendData ) )
		{
			result = 0x01;
		}
		else
		{
			printf("Fail, SendRemote. ( node: cinema.change.contents )\n");
			NxDbgMsg( NX_DBG_VBS, "Fail, SendRemote. ( node: cinema.change.contents )\n");
		}
	}

	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
#else
	IMB_ChangeMode( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );

	pOutBuf[0] = 0x01;
	*iOutSize  = 1;

	return 0;
#endif
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IMB_ChangeMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );
	UNUSED( pOutBuf );
	UNUSED( iOutSize );

	int32_t iMode = pInBuf[0];
	int32_t iCurMode = -1;
	int32_t bBooting = true;

	char* pValue;

	uint8_t *pPtr;
	uint8_t inBuf[MAX_PAYLOAD_SIZE];
	uint8_t outBuf[MAX_PAYLOAD_SIZE];
	int32_t iInBufSize  = 0;
	int32_t iOutBufSize = 0;

	pOutBuf[0] = 0xFF;
	pOutBuf[1] = 0xFF;
	pOutBuf[2] = 0xFF;
	pOutBuf[3] = 0xFF;
	*iOutSize  = 4;

	//	1. Read Mode
	{
		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
		m_pPreference->Read( CINEMA_KEY_INTIAL_MODE, &pValue );
		if( NULL != pValue ) iCurMode = atoi( pValue );
		m_pPreference->Close();

		printf("iCurMode( %d ), iMode( %d )\n", iCurMode, iMode );

		if( iCurMode == iMode )
		{
			goto ERROR_INTIALIZE;
		}

		if( !m_pTReg->IsValid( iMode ) )
		{
			goto ERROR_INTIALIZE;
		}
	}

	//	2. Check Booting Status
	{
		for( int32_t i = 0; i < m_iSlaveNum; i++ )
		{
			inBuf[0] = m_iSlave[i];
			iInBufSize  = 1;

			ProcessCommand( TCON_CMD_BOOTING_STATUS, inBuf, iInBufSize, outBuf, &iOutBufSize );

			if( 0x01 != outBuf[0] )
			{
				printf("Fail, ProcessCommand(). ( cabinet: %d, port: %d, slave: %d )",
					TCON_CABINET(m_iSlave[i]), TCON_I2C_PORT(m_iSlave[i]), TCON_I2C_SLAVE(m_iSlave[i]) );

				bBooting = false;
			}
		}

		if( false == bBooting )
		{
			printf("Fail, TCON Booting.\n" );
			goto ERROR_INTIALIZE;
		}
	}

	//	3. PFPGA Mute On
	{
		inBuf[0] = 0x01;
		iInBufSize  = 1;

		ProcessCommand( PFPGA_CMD_MUTE, inBuf, iInBufSize, outBuf, &iOutBufSize );
	}

	//	4. Write P_REG.txt
	{
		uint16_t *pReg = m_pPReg->GetRegister(iMode);
		uint16_t *pDat = m_pPReg->GetData(iMode);

		for( int32_t i = 0; i < m_pPReg->GetDataNum(iMode); i++ )
		{
			pPtr  = inBuf;
			iInBufSize = 0;

			*pPtr++ = ((pReg[i] >> 8) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pReg[i] >> 0) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pDat[i] >> 8) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pDat[i] >> 0) & 0xFF);	iInBufSize++;

			ProcessCommand( PFPGA_CMD_REG_WRITE, inBuf, iInBufSize, outBuf, &iOutBufSize );
		}
	}

	//	5. Write Uniformity
	{
		if( m_pPReg->GetEnable(iMode) )
		{
			if( !m_pUniformity->Parse( PFPGA_USB_UNIFORMITY_FILE ) )
			{
				pPtr = inBuf;
				iInBufSize = 0;

				uint16_t *pDat = m_pUniformity->GetData();
				for( int32_t i = 0; i < CNX_Uniformity::MAX_DATA_NUM; i++ )
				{
					*pPtr++ = ((pDat[i] >> 8) && 0xFF); iInBufSize++;
					*pPtr++ = ((pDat[i] >> 0) && 0xFF); iInBufSize++;
				}

				ProcessCommand( PFPGA_CMD_UNIFORMITY_DATA, inBuf, iInBufSize, outBuf, &iOutBufSize );
			}

		}
	}

	//	6. Write T_REG.txt
	{
		uint16_t *pReg = m_pTReg->GetRegister(iMode);
		uint16_t *pDat = m_pTReg->GetData(iMode);

		for( int32_t i = 0; i < m_pTReg->GetDataNum(iMode); i++ )
		{
			pPtr  = inBuf;
			iInBufSize = 0;

			*pPtr++ = 0x09;						iInBufSize++;
			*pPtr++ = ((pReg[i] >> 8) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pReg[i] >> 0) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pDat[i] >> 8) & 0xFF);	iInBufSize++;
			*pPtr++ = ((pDat[i] >> 0) & 0xFF);	iInBufSize++;

			for( int port = 0; port < 2; port++ )
			{
				if( IsValidPort(port) )
				{
					inBuf[0] = 0x09 | (port << 7);
					ProcessCommand( TCON_CMD_REG_WRITE, inBuf, iInBufSize, outBuf, &iOutBufSize );
				}
			}
		}
	}

	//	7. Write Gamma
	{
		char *pList[32];
		int32_t iListNum = 0;

		int32_t gamma[4] = { -1, -1, -1, -1 };

		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
		m_pPreference->Read( CINEMA_KEY_UPDATE_TGAM0, &pValue );
		if( pValue != NULL ) gamma[0] = atoi( pValue );
		m_pPreference->Read( CINEMA_KEY_UPDATE_TGAM1, &pValue );
		if( pValue != NULL ) gamma[1] = atoi( pValue );
		m_pPreference->Read( CINEMA_KEY_UPDATE_DGAM0, &pValue );
		if( pValue != NULL ) gamma[2] = atoi( pValue );
		m_pPreference->Read( CINEMA_KEY_UPDATE_DGAM1, &pValue );
		if( pValue != NULL ) gamma[3] = atoi( pValue );
		m_pPreference->Close();

		if( 10 > iMode )
		{
			CNX_File::GetList( TCON_EEPROM_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 1)) )
					{
						printf("Skip. Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != gamma[0])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != gamma[1])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != gamma[2])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != gamma[3])) )
					{
						printf("Skip. Already Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pPtr  = inBuf;
					iInBufSize = 0;

					*pPtr++ = 0x09;					iInBufSize++;
					*pPtr++ = m_pGamma->GetTable();	iInBufSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pPtr++ = ((pDat[i] >> 16) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  8) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  0) & 0xFF);	iInBufSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInBufSize, outBuf, &iOutBufSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );
		}
		else
		{
			CNX_File::GetList( TCON_EEPROM_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 1)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 1)) )
					{
						printf("Skip. Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != gamma[0])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != gamma[1])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != gamma[2])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != gamma[3])) )
					{
						printf("Skip. Already Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pPtr  = inBuf;
					iInBufSize = 0;

					*pPtr++ = 0x09;					iInBufSize++;
					*pPtr++ = m_pGamma->GetTable();	iInBufSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pPtr++ = ((pDat[i] >> 16) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  8) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  0) & 0xFF);	iInBufSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInBufSize, outBuf, &iOutBufSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );

			CNX_File::GetList( TCON_USB_GAMMA_PATH, TCON_GAMMA_PATTERN, pList, &iListNum );
			for( int32_t i = 0; i < iListNum; i++ )
			{
				if( !m_pGamma->Parse( pList[i] ) )
				{
					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != 2)) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != 2)) )
					{
						printf("Skip. Update USB Gamma. ( %s )\n", pList[i]);
						continue;
					}

					if( ((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[0] != gamma[0])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[1] != gamma[1])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT0) && (m_pTReg->GetGammaType(iMode)[2] != gamma[2])) ||
						((m_pGamma->GetType() == CNX_Gamma::TYPE_DEVICE) && (m_pGamma->GetTable() == CNX_Gamma::TABLE_LUT1) && (m_pTReg->GetGammaType(iMode)[3] != gamma[3])) )
					{
						printf("Skip. Already Update EEPRom Gamma. ( %s )\n", pList[i]);
						continue;
					}

					int32_t iCmd = (m_pGamma->GetType() == CNX_Gamma::TYPE_TARGET) ? TCON_CMD_TGAM_R : TCON_CMD_DGAM_R;
					iCmd += m_pGamma->GetChannel();

					pPtr  = inBuf;
					iInBufSize = 0;

					*pPtr++ = 0x09;					iInBufSize++;
					*pPtr++ = m_pGamma->GetTable();	iInBufSize++;

					uint32_t *pDat = m_pGamma->GetData();
					for( int32_t j = 0; j < CNX_Gamma::MAX_DATA_NUM; j++ )
					{
						*pPtr++ = ((pDat[i] >> 16) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  8) & 0xFF);	iInBufSize++;
						*pPtr++ = ((pDat[i] >>  0) & 0xFF);	iInBufSize++;
					}

					for( int port = 0; port < 2; port++ )
					{
						if( IsValidPort(port) )
						{
							inBuf[0] = 0x09 | (port << 7);
							ProcessCommand( iCmd, inBuf, iInBufSize, outBuf, &iOutBufSize );
						}
					}
				}
			}
			CNX_File::FreeList( pList, iListNum );
		}
	}

	//	8. Update Gamma Status For Skip
	{
		char szGamma[4][2];
		snprintf( szGamma[0], sizeof(szGamma[0]), "%d", m_pTReg->GetGammaType(iMode)[0] );
		snprintf( szGamma[1], sizeof(szGamma[1]), "%d", m_pTReg->GetGammaType(iMode)[1] );
		snprintf( szGamma[2], sizeof(szGamma[2]), "%d", m_pTReg->GetGammaType(iMode)[2] );
		snprintf( szGamma[3], sizeof(szGamma[3]), "%d", m_pTReg->GetGammaType(iMode)[3] );

		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
		m_pPreference->Write( CINEMA_KEY_UPDATE_TGAM0, szGamma[0]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_TGAM1, szGamma[1]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_DGAM0, szGamma[2]);
		m_pPreference->Write( CINEMA_KEY_UPDATE_DGAM1, szGamma[3]);
		m_pPreference->Dump();
		m_pPreference->Close();
	}

	//	9. SW Reset
	{
		for( int32_t port = 0; port < 2; port++ )
		{
			if( IsValidPort(port) )
			{
				inBuf[0] = 0x09 | (port << 7);
				iInBufSize  = 1;

				ProcessCommand( TCON_CMD_SW_RESET, inBuf, iInBufSize, outBuf, &iOutBufSize );
			}
		}
	}

	//	10. PFPGA Mute Off
	{
		inBuf[0] = 0x00;
		iInBufSize  = 1;

		ProcessCommand( PFPGA_CMD_MUTE, inBuf, iInSize, outBuf, &iOutBufSize );
	}

	{
		char szValue[64];
		snprintf(szValue, sizeof(szValue), "%d", iMode );

		m_pPreference->Open( CINEMA_PREFERENCE_CONFIG );
		m_pPreference->Write( CINEMA_KEY_INTIAL_MODE, szValue );
		m_pPreference->Dump();
		m_pPreference->Close();
	}

	*iOutSize  = 4;

	pOutBuf[0] = 0x00;
	pOutBuf[1] = 0x00;
	pOutBuf[2] = 0x00;
	pOutBuf[3] = 0x01;

ERROR_INTIALIZE:
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IMB_GetCurrentMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	if( !m_pPreference->Open( CINEMA_PREFERENCE_CONFIG ) )
	{
		char *pValue = NULL;
		if( !m_pPreference->Read( CINEMA_KEY_INTIAL_MODE, &pValue ) )
		{
			if( NULL != pValue )
			{
				int32_t iMode = atoi( pValue );

				result[0] = (iMode >> 24) & 0xFF;
				result[1] = (iMode >> 16) & 0xFF;
				result[2] = (iMode >>  8) & 0xFF;
				result[3] = (iMode >>  0) & 0xFF;
			}
		}

		m_pPreference->Close();
	}

	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IMB_WriteFile( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	char szName[256] = { 0x00, };
	memcpy( szName, pInBuf, sizeof(szName) );

	char szPath[1024] = { 0x00, };
	if( !strcmp( szName, "T_REG.txt" ) )	snprintf( szPath, sizeof(szPath), "%s/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "P_REG.txt" ) )	snprintf( szPath, sizeof(szPath), "%s/%s", PFPGA_USB_PREG_PATH, szName );

	if( !strcmp( szName, "TGAM0_R.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "TGAM0_G.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "TGAM0_B.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "TGAM1_R.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "TGAM1_G.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "TGAM1_B.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM0_R.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM0_G.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM0_B.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM1_R.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM1_G.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "DGAM1_B.txt") )	snprintf( szPath, sizeof(szPath), "%s/LUT/%s", TCON_USB_TREG_PATH, szName );
	if( !strcmp( szName, "UC_COEF.txt") )	snprintf( szPath, sizeof(szPath), "%s/%s", PFPGA_USB_PREG_PATH, szName );

	if( 0 == strlen(szPath) )
	{
		return -1;
	}

	int32_t iWriteSize;
	FILE *hFile = fopen( szPath, "wb" );

	if( hFile )
	{
		iWriteSize = fwrite( pInBuf + 256, 1, iInSize - 256, hFile );
		if( iWriteSize == iInSize-256 )
		{
			result[0] = 0x00;
			result[1] = 0x00;
			result[2] = 0x00;
			result[3] = 0x01;
		}

		fclose( hFile );
	}

	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

	if( !strcmp( szName, "T_REG.txt" ) )
	{
		m_pTReg->Update( TCON_USB_TREG_FILE );
		m_pTReg->Update( TCON_EEPROM_TREG_FILE );
		m_pTReg->Dump();
	}

	if( !strcmp( szName, "P_REG.txt" ) )
	{
		m_pPReg->Update( PFPGA_USB_PREG_FILE );
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::IMB_ReadModeDesc( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );
	UNUSED( iOutSize );

	char *pDstBuf = (char*)pOutBuf;
	if( !pDstBuf )
		return -1;

	for( int32_t i = 0; i < 20; i++ )
	{
		char *pDesc = m_pTReg->GetDescriptor( i );
		memset( pDstBuf, 0x00, 256 );

		if( pDesc )
		{
			snprintf( pDstBuf, 256, "%s", pDesc );
		}

		pDstBuf += 256;
	}

	*iOutSize = 256 * 20;
	return 0;
}

//------------------------------------------------------------------------------
//
//	Test Pattern Functions
//

//------------------------------------------------------------------------------
static int32_t TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	2901,	2171,	100		},	//	Red-1
		{	10,		0,		1024,	0,		2160,	2417,	3493,	1222	},	//	Green-1
		{	10,		0,		1024,	0,		2160,	2014,	1416,	3816	},	//	Blue-1
		{	10,		0,		1024,	0,		2160,	2911,	3618,	3890	},	//	Cyan-1
		{	10,		0,		1024,	0,		2160,	3289,	2421,	3814	},	//	Magenta-1
		{	10,		0,		1024,	0,		2160,	3494,	3853,	1221	},	//	Yellow-1
		{	10,		0,		1024,	0,		2160,	2738,	2171,	1233	},	//	Red-2
		{	10,		0,		1024,	0,		2160,	2767,	3493,	2325	},	//	Green-2
		{	10,		0,		1024,	0,		2160,	1800,	1416,	3203	},	//	Blue-2
		{	10,		0,		1024,	0,		2160,	3085,	3590,	3756	},	//	Cyan-2
		{	10,		0,		1024,	0,		2160,	3062,	2421,	3497	},	//	Magenta-2
		{	10,		0,		1024,	0,		2160,	3461,	3777,	2065	},	//	Yellow-2
		{	10,		0,		1024,	0,		2160,	3883,	3960,	4092	},	//	White-1
		{	10,		0,		1024,	0,		2160,	3794,	3960,	3890	},	//	White-2
		{	10,		0,		1024,	0,		2160,	3893,	3960,	3838	},	//	White-3
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1024,	0,		2160,	4095,	4095,	4095	},	// 6 Color Bar
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	White	100%
		{	10,		0,		1024,	0,		2160,	3685,	3685,	3685	},	//	Gray	90%
		{	10,		0,		1024,	0,		2160,	3276,	2948,	2948	},	//	Gray	80%
		{	10,		0,		1024,	0,		2160,	2866,	2866,	2866	},	//	Gray	70%
		{	10,		0,		1024,	0,		2160,	2457,	2457,	2457	},	//	Gray	60%
		{	10,		0,		1024,	0,		2160,	2047,	2047,	2047	},	//	Gray	50%
		{	10,		0,		1024,	0,		2160,	1638,	1638,	1638	},	//	Gray	40%
		{	10,		0,		1024,	0,		2160,	1228,	1228,	1228	},	//	Gray	30%
		{	10,		0,		1024,	0,		2160,	819,	819,	819		},	//	Gray	20%
		{	10,		0,		1024,	0,		2160,	409,	409,	409		},	//	Gray	1c0%
		{	10,		0,		1024,	0,		2160,	0,		0,		0		},	//	Black	0%
		{	10,		0,		1024,	0,		2160,	4095,	0,		0		},	//	Red		100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	0		},	//	Green	100%
		{	10,		0,		1024,	0,		2160,	0,		0,		4095	},	//	Blue	100%
		{	10,		0,		1024,	0,		2160,	4095,	0,		4095	},	//	Magenta	100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	4095	},	//	Cyan	100%
		{	10,		0,		1024,	0,		2160,	4095,	4095,	0		},	//	Yellow 	00%
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28
		{	2,		0,		1024,	0,		2160	},							//	Gray 16-Step
		{	3,		0,		1024,	0,		2160	},							//	Gray 32-Step
		{	4,		0,		1024,	0,		2160	},							//	Gray 64-Step
		{	5,		0,		1024,	0,		2160	},							//	Gray 128-Step
		{	6,		0,		1024,	0,		2160	},							//	Gray 256-Step
		{	7,		0,		1024,	0,		2160	},							//	Gray 512-Step
		{	9,		7,		1024,	0,		2160	},							//	Gray 2048-Step
		{	9,		1,		1024,	0,		2160	},							//	Red 2048-Step
		{	9,		2,		1024,	0,		2160	},							//	Green 2048-Step
		{	9,		4,		1024,	0,		2160	},							//	Blue 2048-Step
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	17,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	104,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_ERR, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static void MakeDirectory( const char *pDir )
{
	char buf[1024];
	char *pBuf = buf;

	memcpy( buf, pDir, sizeof(buf) -1 );
	buf[sizeof(buf)-1] = 0x00;

	while( *pBuf )
	{
		if( '/' == *pBuf )
		{
			*pBuf = 0x00;
			if( 0 != access( buf, F_OK ) && (pBuf != buf) )
			{
				printf("Make Directory. ( %s )\n", buf);
				mkdir( buf, 0777 );
			}
			*pBuf = '/';
		}
		pBuf++;
	}

	if( 0 != access( buf, F_OK) )
	{
		mkdir( buf, 0777 );
	}
}

//------------------------------------------------------------------------------
static int32_t SendRemote( const char *pSock, const char *pMsg )
{
	int32_t sock, len;
	struct sockaddr_un addr;

	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf("Fail, socket().\n");
		return -1;
	}

	addr.sun_family  = AF_UNIX;
	addr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( addr.sun_path + 1, pSock );

	len = 1 + strlen(pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
	{
		printf("Fail, connect(). ( node: %s )\n", pSock);
		close( sock );
		return -1;
	}

	if( 0 > write(sock, (const char*)pMsg, strlen(pMsg)) )
	{
		printf("Fail, write().\n");
		close( sock );
		return -1;
	}

	close( sock );
	return 0;
}

//------------------------------------------------------------------------------
CNX_CinemaManager*	CNX_CinemaManager::m_pstInstance = NULL;
CNX_Mutex			CNX_CinemaManager::m_hInstanceLock;

CNX_CinemaManager* CNX_CinemaManager::GetInstance( void )
{
	CNX_AutoLock lock( &CNX_CinemaManager::m_hInstanceLock );

	if( NULL == m_pstInstance )
	{
		m_pstInstance = new CNX_CinemaManager();
	}

	return m_pstInstance;
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::ReleaseInstance( void )
{
	CNX_AutoLock lock( &CNX_CinemaManager::m_hInstanceLock );

	if( m_pstInstance )
	{
		delete m_pstInstance;
		m_pstInstance = NULL;
	}
}
