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
#include <CNX_CinemaManager.h>

#include <NX_CinemaCommand.h>
#include <NX_CinemaRegister.h>
#include <NX_Utils.h>
#include <NX_Version.h>

#define NX_DTAG	"[CNX_CinemaManager]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
#define I2C_DEBUG			0
#define I2C_SEPARATE_BURST	1

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

#define NX_ENABLE_CHECK_SCREEN_ALL		false
#define NX_ENABLE_CHECK_SCREEN_BOOT		false
#define NX_ENABLE_CHECK_SCREEN_DELAY	false

#define TCON_BURST_RETRY_COUNT			3

#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

#define TCON_EEPROM_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_EEPROM"
#define TCON_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_USB"
#define PFPGA_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/PFPGA"

#define TCON_EEPROM_BINARY_FILE			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.bin"

//------------------------------------------------------------------------------
CNX_CinemaManager::CNX_CinemaManager()
	: m_hThreadCommand( 0x00 )
	, m_bRun( false )
	, m_iCmd( 0x0000 )
#if NX_ENABLE_CHECK_SCREEN_BOOT
	, m_iScreenType (CheckScreenType())
#else
	, m_iScreenType (SCREEN_TYPE_P25)
#endif
{
	m_pTestPatternFunc[0] = &CNX_CinemaManager::TestPatternDci;
	m_pTestPatternFunc[1] = &CNX_CinemaManager::TestPatternColorBar;
	m_pTestPatternFunc[2] = &CNX_CinemaManager::TestPatternFullScreenColor;
	m_pTestPatternFunc[3] = &CNX_CinemaManager::TestPatternGrayScale;
	m_pTestPatternFunc[4] = &CNX_CinemaManager::TestPatternDot;
	m_pTestPatternFunc[5] = &CNX_CinemaManager::TestPatternDiagonal;

	memset( m_NapVersion, 0x00, sizeof(m_NapVersion) );
	memset( m_SapVersion, 0x00, sizeof(m_SapVersion) );
	memset( m_IpcVersion, 0x00, sizeof(m_IpcVersion) );
	memset( m_TmsVersion, 0x00, sizeof(m_TmsVersion) );

	NxDbgMsg( NX_DBG_INFO, "--------------------------------------------------------------------------------\n" );
	NxDbgMsg( NX_DBG_INFO, ">> Build Information:  %08lld-%06lld ( %s, %s )\n",
		NX_DATE(), NX_TIME(), __TIME__, __DATE__ );
	NxDbgMsg( NX_DBG_INFO, "--------------------------------------------------------------------------------\n" );

	MakeDirectory( TCON_EEPROM_RESULT_PATH );
	MakeDirectory( TCON_USB_RESULT_PATH );
	MakeDirectory( PFPGA_USB_RESULT_PATH );
}

//------------------------------------------------------------------------------
CNX_CinemaManager::~CNX_CinemaManager()
{
	if( m_hThreadCommand )
	{
		pthread_join( m_hThreadCommand, NULL );
		m_hThreadCommand = 0x00;
	}
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetNapVersion( uint8_t *pVersion, int32_t iSize )
{
	UNUSED(iSize);

	char szVersion[1024] = { 0x00, };
	memcpy( szVersion, pVersion, iSize );

	// NAP VERSION + BUILD DATE of CinemaManager
	snprintf( (char*)m_NapVersion, sizeof(m_NapVersion), "%s ( %08lld )", szVersion, NX_DATE() );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetSapVersion( uint8_t *pVersion, int32_t iSize )
{
	UNUSED(iSize);

	char szVersion[1024] = { 0x00, };
	memcpy( szVersion, pVersion, iSize );

	snprintf( (char*)m_SapVersion, sizeof(m_SapVersion), "%s", szVersion );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetIpcVersion( uint8_t *pVersion, int32_t iSize )
{
	UNUSED(iSize);

	char szVersion[1024] = { 0x00, };
	memcpy( szVersion, pVersion, iSize );

	snprintf( (char*)m_IpcVersion, sizeof(m_IpcVersion), "%s", szVersion );
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::SetTmsVersion( uint8_t *pVersion, int32_t iSize )
{
	UNUSED(iSize);

	char szVersion[1024] = { 0x00, };
	memcpy( szVersion, pVersion, iSize );

	snprintf( (char*)m_TmsVersion, sizeof(m_TmsVersion), "%s", szVersion );
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
	//
	//	TCON Commands
	//
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
	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:							return TCON_OptionalData( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_SW_RESET:									return TCON_SwReset( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_EEPROM_READ:								return TCON_EEPRomRead( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_MUTE:										return TCON_Mute( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case TCON_CMD_VERSION:									return TCON_Version( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );

	//
	//	PFPGA Commands
	//
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

	//
	//	Platform Commands
	//
	case PLATFORM_CMD_NAP_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_NAP_VERSION ):			return PLAT_NapVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_SAP_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_SAP_VERSION ):			return PLAT_SapVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_IPC_SERVER_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_IPC_SERVER_VERSION ):	return PLAT_IpcVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_TMS_SERVER_VERSION:
	case CMD_PLATFORM( PLATFORM_CMD_TMS_SERVER_VERSION ):	return PLAT_TmsVersion( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );
	case PLATFORM_CMD_SCREEN_TYPE:
	case CMD_PLATFORM( PLATFORM_CMD_SCREEN_TYPE ):			return PLAT_ScreenType( iCmd, pInBuf, iInSize, pOutBuf, iOutSize );

	default :
		break;
	}

	return -1;
}


//------------------------------------------------------------------------------
//
//	TCON Commands
//

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

	NxDbgMsg( NX_DBG_DEBUG, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

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

	NxDbgMsg( NX_DBG_DEBUG, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
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

	iWriteData = (uint16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
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
	uint8_t bRead	= pInBuf[1];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( bRead )
	{
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_DOOR_READ )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_CHECK_DOOR_READ );
			goto ERROR_TCON;
		}

		if( 0 > i2c.Write( slave, TCON_REG_RESERVED1, 2046 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_RESERVED1, 2046 );
			goto ERROR_TCON;
		}

		result = (uint8_t)iReadData;
	}
	else
	{
		if( 0 > i2c.Write( slave, TCON_REG_RESERVED1, 0 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_RESERVED1, 0 );
			goto ERROR_TCON;
		}

		if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 0 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CLOSE_DOOR, 0 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 1 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CLOSE_DOOR, 1 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 0 ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CLOSE_DOOR, 0 );
			goto ERROR_TCON;
		}

		result = 0x01;
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

	NxDbgMsg( NX_DBG_DEBUG, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, expected: 0x%04x or 0x%04x )\n",
			port, slave, TCON_REG_LVDS_STATUS, iReadData, 1080, 2160 );

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

	NxDbgMsg( NX_DBG_DEBUG, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, expected: 0x%04x )\n",
			port, slave, TCON_REG_BOOTING_STATUS, iReadData, 0x07E1 );

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

	const uint16_t PatternDataP25[] = {
		10,		0,		1024,	0,		2160,	4095,	4095,	4095
	};

	const uint16_t PatternDataP33[] = {
		10,		0,		1056,	0,		2160,	4095,	4095,	4095
	};

	int32_t iPatternSize = (int32_t)(sizeof(PatternDataP25) / sizeof(PatternDataP25[0]));
	const uint16_t *pPatternData =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < iPatternSize; i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_PATTERN + i, pPatternData[i] ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_PATTERN + i, pPatternData[i] );
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
	memcpy( pOutBuf, &result, *iOutSize );

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
	//	pInBuf[0] : index, pInBuf[1] : function index, pInBuf[2] : pattern index
	//

	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t funcIndex = pInBuf[1];
	uint8_t patternIndex = pInBuf[2];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( TCON_CMD_PATTERN_RUN == iCmd )
	{
		if( funcIndex != MAX_TEST_PATTERN )
		{
			NxDbgMsg( NX_DBG_INFO, "ScreenType( %d ), Port( %d ), Slave( 0x%02X ), FuncIndex( %d ), PatternIndex( %d )",
				m_iScreenType, port, slave, funcIndex, patternIndex );

			(this->*m_pTestPatternFunc[funcIndex])(&i2c, index, patternIndex);
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

				NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
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

		if( funcIndex != MAX_TEST_PATTERN )
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

				NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
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
int32_t CNX_CinemaManager::TCON_TargetGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
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

		// NxDbgMsg( NX_DBG_DEBUG, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
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

		// NxDbgMsg( NX_DBG_DEBUG, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
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
int32_t CNX_CinemaManager::TCON_DeviceGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
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

		// NxDbgMsg( NX_DBG_DEBUG, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
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

		// NxDbgMsg( NX_DBG_DEBUG, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
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
	int32_t iRetryCount = 0, bFail;

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
			bFail = false;
			break;
		}
		// NxDbgMsg( NX_DBG_DEBUG, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
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
	uint16_t nBaseX = 0;
	uint16_t nBaseY = 0;

	int32_t iResultLen = 0;
	uint8_t *pResult = NULL;
	uint8_t *pPtr = NULL;

	int32_t cnt = 0;

	int32_t iReadData;
	int32_t iCol = 0;
	int32_t iRow = 0;

	iResultLen = 1;
	pResult = (uint8_t*)malloc( iResultLen * sizeof(uint8_t) );
	pResult[0] = 0xFF;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_PITCH_INFO )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_PITCH_INFO );
		goto ERROR_TCON;
	}

	switch( m_iScreenType )
	{
		case SCREEN_TYPE_P33:	iCol = 48;	iRow = 45;	break;
		default:				iCol = 64;	iRow = 60;	break;
	}

	printf("port( %d ), slave( %d ), modlue( %d ), screenType( %s )\n", port, slave, module,
		(m_iScreenType == SCREEN_TYPE_P33) ? "P3.3" : "P2.5" );

	nBaseX = module % 4 * iCol;
	nBaseY = module / 4 * iRow;

	if( pResult ) free( pResult );

	iResultLen = iCol * iRow * 8 * 2;
	pResult = (uint8_t*)malloc( iResultLen * sizeof(uint8_t) );
	memset( pResult, 0x00, iResultLen * sizeof(uint8_t) );

	pPtr = pResult;

	for( int32_t i = 0; i < iRow; i++ )
	{
		for( int32_t j = 0; j < iCol; j++ )
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
				i * iCol + j + 1, iRow * iCol,
				(int)((float)((i * iCol) + j + 1) / (float)(iCol * iRow) * (float)100) );
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
	//	1. Read EEPROM Version
	//
	{
		//
		//	1-1. Read EEPROM Data
		//
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

			// fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_VERSION_SIZE);
			// fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_VERSION_SIZE );

		//
		//	1-2. Parse EEPROM Version.
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
		//	1-3. Check Version Validation.
		//
		if( version[0] == 0x00 || version[0] == 0xFF )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Wrong Version.\n" );
			goto ERROR_TCON;
		}
	}

	//
	//	2. Check Version
	//
	if( !access( TCON_EEPROM_BINARY_FILE, F_OK) )
	{
		//
		//	2-1. Binary Version Parsing.
		//
		parser.Init( TCON_EEPROM_BINARY_FILE );
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

		//
		//	2-2. Check Version
		//
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

	//
	//	3. Update Binary
	//
	if( bUpdate )
	{
		//
		//	3-1. Read EEPRom Data.
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
		//	3-2. Make Binary File.
		//
		pFile = fopen( TCON_EEPROM_BINARY_FILE, "wb" );
		if( NULL == pFile )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Make Binary File. ( %s )\n", TCON_EEPROM_BINARY_FILE );
			goto ERROR_TCON;
		}

		fwrite( pBuf, 1, TCON_EEPROM_DATA_SIZE, pFile );
		fclose( pFile );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s )\n", TCON_EEPROM_BINARY_FILE );

		//
		//	3-3. Make T_REG_EEPROM.txt
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

		parser.WriteTconInfo( pInfo, TCON_EEPROM_RESULT_PATH );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s/T_REG_EEPROM.txt )\n", TCON_EEPROM_RESULT_PATH );

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
int32_t CNX_CinemaManager::TCON_Mute( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( iInSize );

	//
	//	pInBuf[0] : index, pInBuf[1] : data
	//
	uint8_t result	= 0xFF;

	uint8_t index	= pInBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);
	uint8_t data	= pInBuf[1];

	if( data == 0x00 )
		WaitTime( 500, __FUNCTION__ );

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, data ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, data );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_DEBUG, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, data );

	// already delayed in PFPGA Mute.
	if( data == 0x01 )
		WaitTime( 0, __FUNCTION__ );

	result = 0x01;

ERROR_TCON:
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
	memcpy( pOutBuf, result, *iOutSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	PFPGA Commands
//

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

	NxDbgMsg( NX_DBG_DEBUG, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	result[0] = 0x00;
	result[1] = 0x00;
	result[2] = 0x00;
	result[3] = 0x00;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

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

	NxDbgMsg( NX_DBG_DEBUG, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, iReadData );

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

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

	NxDbgMsg( NX_DBG_ERR, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, expected: 0x%04x )\n",
			port, slave, PFPGA_REG_CHECK_STATUS, iReadData, PFPGA_VAL_CHECK_STATUS );

	result = ((PFPGA_VAL_CHECK_STATUS == iReadData) ? 1 : 0);

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
		WaitTime( 400, __FUNCTION__ );

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
		WaitTime( 100, __FUNCTION__ );	// 2frame with 24Hz

	result = 0x01;

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

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

	uint8_t result[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadMajor = 0x0000, iReadMinor = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadMajor = i2c.Read( slave, PFPGA_REG_PF_VERSION )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_PF_VERSION );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadMinor = i2c.Read( slave, PFPGA_REG_PF_RESERVED1 )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_PF_RESERVED1 );
		goto ERROR_PFPGA;
	}

	// Major Version
	result[0] = (uint8_t)((iReadMajor >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadMajor >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadMajor >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadMajor >>  0) & 0xFF);

	// Minor Version
	result[4] = (uint8_t)((iReadMinor >> 24) & 0xFF);
	result[5] = (uint8_t)((iReadMinor >> 16) & 0xFF);
	result[6] = (uint8_t)((iReadMinor >>  8) & 0xFF);
	result[7] = (uint8_t)((iReadMinor >>  0) & 0xFF);

ERROR_PFPGA:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, result, *iOutSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	Platform Commands
//

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_NapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	// pInBuf : not used
	//

	*iOutSize = strlen((const char*)m_NapVersion);
	memcpy( pOutBuf, m_NapVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_SapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	// pInBuf : not used
	//

	*iOutSize = strlen((const char*)m_SapVersion);
	memcpy( pOutBuf, m_SapVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_IpcVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	//
	// pInBuf : not used
	//

	*iOutSize = strlen((const char*)m_IpcVersion);
	memcpy( pOutBuf, m_IpcVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_TmsVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	*iOutSize = strlen((const char*)m_TmsVersion);
	memcpy( pOutBuf, m_TmsVersion, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::PLAT_ScreenType( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize )
{
	UNUSED( iCmd );
	UNUSED( pInBuf );
	UNUSED( iInSize );

	uint8_t result;

#if NX_ENABLE_CHECK_SCREEN_BOOT
	result = m_iScreenType;
#else
	result = m_iScreenType = CheckScreenType();
#endif

	NxDbgMsg( NX_DBG_INFO, ">>> %s(): ScreenType( %s )\n",
		__FUNCTION__, (result == SCREEN_TYPE_P33) ? "P3.3" : "P2.5" );

ERROR_PLAT:
	*iOutSize = sizeof(result);
	memcpy( pOutBuf, &result, *iOutSize );

	return 0;
}

//------------------------------------------------------------------------------
//
//	Test Pattern Functions
//

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
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

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1056,	0,		2160,	2901,	2171,	100		},	//	Red-1
		{	10,		0,		1056,	0,		2160,	2417,	3493,	1222	},	//	Green-1
		{	10,		0,		1056,	0,		2160,	2014,	1416,	3816	},	//	Blue-1
		{	10,		0,		1056,	0,		2160,	2911,	3618,	3890	},	//	Cyan-1
		{	10,		0,		1056,	0,		2160,	3289,	2421,	3814	},	//	Magenta-1
		{	10,		0,		1056,	0,		2160,	3494,	3853,	1221	},	//	Yellow-1
		{	10,		0,		1056,	0,		2160,	2738,	2171,	1233	},	//	Red-2
		{	10,		0,		1056,	0,		2160,	2767,	3493,	2325	},	//	Green-2
		{	10,		0,		1056,	0,		2160,	1800,	1416,	3203	},	//	Blue-2
		{	10,		0,		1056,	0,		2160,	3085,	3590,	3756	},	//	Cyan-2
		{	10,		0,		1056,	0,		2160,	3062,	2421,	3497	},	//	Magenta-2
		{	10,		0,		1056,	0,		2160,	3461,	3777,	2065	},	//	Yellow-2
		{	10,		0,		1056,	0,		2160,	3883,	3960,	4092	},	//	White-1
		{	10,		0,		1056,	0,		2160,	3794,	3960,	3890	},	//	White-2
		{	10,		0,		1056,	0,		2160,	3893,	3960,	3838	},	//	White-3
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1024,	0,		2160,	4095,	4095,	4095	},	// 6 Color Bar
	};

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1056,	0,		2160,	4095,	4095,	4095	},	// 6 Color Bar
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
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

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1056,	0,		2160,	4095,	4095,	4095	},	//	White	100%
		{	10,		0,		1056,	0,		2160,	3685,	3685,	3685	},	//	Gray	90%
		{	10,		0,		1056,	0,		2160,	3276,	2948,	2948	},	//	Gray	80%
		{	10,		0,		1056,	0,		2160,	2866,	2866,	2866	},	//	Gray	70%
		{	10,		0,		1056,	0,		2160,	2457,	2457,	2457	},	//	Gray	60%
		{	10,		0,		1056,	0,		2160,	2047,	2047,	2047	},	//	Gray	50%
		{	10,		0,		1056,	0,		2160,	1638,	1638,	1638	},	//	Gray	40%
		{	10,		0,		1056,	0,		2160,	1228,	1228,	1228	},	//	Gray	30%
		{	10,		0,		1056,	0,		2160,	819,	819,	819		},	//	Gray	20%
		{	10,		0,		1056,	0,		2160,	409,	409,	409		},	//	Gray	1c0%
		{	10,		0,		1056,	0,		2160,	0,		0,		0		},	//	Black	0%
		{	10,		0,		1056,	0,		2160,	4095,	0,		0		},	//	Red		100%
		{	10,		0,		1056,	0,		2160,	0,		4095,	0		},	//	Green	100%
		{	10,		0,		1056,	0,		2160,	0,		0,		4095	},	//	Blue	100%
		{	10,		0,		1056,	0,		2160,	4095,	0,		4095	},	//	Magenta	100%
		{	10,		0,		1056,	0,		2160,	0,		4095,	4095	},	//	Cyan	100%
		{	10,		0,		1056,	0,		2160,	4095,	4095,	0		},	//	Yellow 	00%
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
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

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28
		{	2,		0,		1056,	0,		2160	},							//	Gray 16-Step
		{	3,		0,		1056,	0,		2160	},							//	Gray 32-Step
		{	4,		0,		1056,	0,		2160	},							//	Gray 64-Step
		{	5,		0,		1056,	0,		2160	},							//	Gray 128-Step
		{	6,		0,		1056,	0,		2160	},							//	Gray 256-Step
		{	7,		0,		1056,	0,		2160	},							//	Gray 512-Step
		{	9,		7,		1056,	0,		2160	},							//	Gray 2048-Step
		{	9,		1,		1056,	0,		2160	},							//	Red 2048-Step
		{	9,		2,		1056,	0,		2160	},							//	Green 2048-Step
		{	9,		4,		1056,	0,		2160	},							//	Blue 2048-Step
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	17,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	17,		0,		1056,	0,		2160,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t PatternDataP25[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	104,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t PatternDataP33[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	104,	0,		1056,	0,		2160,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1056,	0,		2160,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t (*pPatternData)[8] =
		(m_iScreenType == SCREEN_TYPE_P33) ? PatternDataP33 : PatternDataP25;

	const uint16_t *pData = pPatternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(PatternDataP25[0]) / sizeof(PatternDataP25[0][0]));

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

			NxDbgMsg( NX_DBG_DEBUG, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}


//------------------------------------------------------------------------------
//
//	Various Functions
//

//------------------------------------------------------------------------------
int32_t CNX_CinemaManager::CheckScreenType()
{
	int32_t iScreenType = SCREEN_TYPE_P25;

	int32_t iDefScreenSel;
	int32_t iExpectScreenPitch = -1;
	int32_t iScreenInfo = -1;
	int32_t bMissmatch = false;

	{
		CNX_I2C i2c_2( PFPGA_I2C_PORT );

		if( 0 > i2c_2.Open() )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", PFPGA_I2C_PORT );
			goto ERROR_I2C;
		}

		if( 0 > (iDefScreenSel = i2c_2.Read( PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL ) ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL );
			goto ERROR_I2C;
		}

		if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, 0x0003) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, 0x0003 );
			goto ERROR_I2C;
		}
	}

	{
		for( int32_t j = 0; j < 2; j++ )
		{
			CNX_I2C i2c_2( PFPGA_I2C_PORT );

			if( 0 > i2c_2.Open() )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", PFPGA_I2C_PORT );
				goto ERROR_I2C;
			}

			if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, j ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, j );
				goto ERROR_I2C;
			}

#if NX_ENABLE_CHECK_SCREEN_DELAY
			NxDbgMsg( NX_DBG_DEBUG, ">>> Expected Model : %s\n", j ? "P3.3" : "P2.5" );
			WaitTime(500, __FUNCTION__);
#endif

			for( int32_t i = 0; i < 255; i++ )
			{
				if( (i & 0x7F) < 0x10 )
					continue;

				int32_t port  = (i & 0x80) >> 7;
				uint8_t slave = (i & 0x7F);
				int32_t iReadData;
				int32_t iScreenPitch;

				CNX_I2C i2c( port );

				if( 0 > i2c.Open() )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", port );
					goto ERROR_I2C;
				}

				if( 0 > (iReadData = i2c.Read( slave, TCON_REG_PITCH_INFO ) ) )
				{
					continue;
				}

				NxDbgMsg( NX_DBG_DEBUG, "Detected. ( i2c-%d, slave: 0x%02x, model: %d )\n", port, slave, j );

				iScreenInfo = iReadData;
				iScreenPitch = iReadData / 1000;

#if NX_ENABLE_CHECK_SCREEN_ALL
				if( 0 > iExpectScreenPitch )
					iExpectScreenPitch = iScreenPitch;

				if( iExpectScreenPitch != iScreenPitch )
				{
					NxDbgMsg( NX_DBG_WARN, "Warn, Missmatch ScreenType. ( i2c-%d, slave: 0x%02x, data: %d, expected: %dxxx )\n",
						port, slave, iReadData, iExpectScreenPitch );

					bMissmatch = true;
				}
#else
				iExpectScreenPitch = iScreenPitch;
				break;
#endif
			}

#if NX_ENABLE_CHECK_SCREEN_ALL
#else
			if( 0 <= iExpectScreenPitch )
				break;
#endif
		}
	}

	{
		NxDbgMsg( NX_DBG_INFO, ">>> ScreenInfo( %d ), ScreenPitch( %d ), bMissmatch( %d )\n",
			iScreenInfo, iExpectScreenPitch, bMissmatch );
	}

	{
		CNX_I2C i2c_2( PFPGA_I2C_PORT );
		if( 0 > i2c_2.Open() )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Open(). ( i2c-%d )\n", PFPGA_I2C_PORT );
			goto ERROR_I2C;
		}

		if( bMissmatch == true || 0 > iExpectScreenPitch )
		{
			NxDbgMsg( NX_DBG_WARN, ">>> Unknown ScreenType. --> Set Default ScreenType P2.5\n" );

			if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, 0x0000) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, 0x0000 );
				goto ERROR_I2C;
			}

			if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, iDefScreenSel) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, iDefScreenSel );
				goto ERROR_I2C;
			}

			iScreenType = SCREEN_TYPE_P25;
		}
		else
		{
			NxDbgMsg( NX_DBG_WARN, ">>> ScreenType is %s ( %d )\n", (iExpectScreenPitch == 33) ? "P3.3" : "P2.5", iExpectScreenPitch );

			if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, (iExpectScreenPitch == 33) ? 0x0001 : 0x0000) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, (iExpectScreenPitch == 33) ? 0x0001 : 0x0000 );
				goto ERROR_I2C;
			}

			if( 0 > i2c_2.Write( PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, (iExpectScreenPitch == 33) ? 0x0003 : iDefScreenSel) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, (iExpectScreenPitch == 33) ? 0x0003 : iDefScreenSel );
				goto ERROR_I2C;
			}

			iScreenType = (iExpectScreenPitch == 33) ? SCREEN_TYPE_P33 : SCREEN_TYPE_P25;
		}
	}

	return iScreenType;

ERROR_I2C:
	NxDbgMsg( NX_DBG_WARN, ">>> Unknown ScreenType. ( reason: i2c fail ) --> Set Default ScreenType P2.5\n" );
	return SCREEN_TYPE_P25;
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::MakeDirectory( const char *pDir )
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
		printf("Make Directory. ( %s )\n", buf);
		mkdir( buf, 0777 );
	}
}

//------------------------------------------------------------------------------
void CNX_CinemaManager::WaitTime( uint64_t iWaitTime, const char *pMsg )
{
	uint64_t iCurTime = NX_GetTickCount();
	uint64_t iTimeout = iCurTime + iWaitTime;
	uint64_t iMiliSecond = 0;

	char msg[256] = "";
	if( NULL != pMsg )
		snprintf(msg, sizeof(msg), "[%s] ", pMsg);

#if 0
	fprintf(stdout, "Wait time : %llu sec.\n", iWaitTime / 1000);
	fflush(stdout);
#else
	NxDbgMsg( NX_DBG_DEBUG, "%sWait time : %llu mSec.\n", msg, iWaitTime);
#endif

	while( iCurTime < iTimeout )
	{
#if 0
		fprintf( stdout, "Wait %llu mSec\r", iMiliSecond );
		fflush(stdout);
#else
		NxDbgMsg( NX_DBG_DEBUG, "%sWait %llu mSec\r", msg, iMiliSecond * 100);
#endif

		iCurTime = NX_GetTickCount();
		usleep(100000);
		iMiliSecond++;
	}

#if 0
	printf("\n");
#endif
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
