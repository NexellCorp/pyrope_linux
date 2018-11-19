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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <CNX_I2C.h>
#include <NX_Utils.h>

#include "CNX_CinemaControl.h"
#include "NX_CinemaRegister.h"

#define NX_DTAG	"[CNX_CinemaControl]"
#include <NX_DbgMsg.h>

#define NX_ENABLE_CHECK_SCREEN_ALL		false
#define NX_ENABLE_CHECK_SCREEN_DELAY	false

//------------------------------------------------------------------------------
#define IS_TCON_BROADCAST(A)	(((A & 0x7F) == 0x09))
#define IS_TCON(A)				(((A & 0x7F) >= 0x10))
#define IS_TCON_LEFT(A)			(((A & 0x7F) >= 0x10) && (((A & 0x80) >> 7) == 0x00))
#define IS_TCON_RIGHT(A)		(((A & 0x7F) >= 0x10) && (((A & 0x80) >> 7) == 0x01))
#define IS_PFPGA(A)				(((A & 0x7F) == 0x0A))

#define IS_I2C_TCON_LEFT(A)		(A == 0)
#define IS_I2C_TCON_RIGHT(A)	(A == 1)
#define IS_I2C_PFPGA(A)			(A == 2)

static int32_t WriteData( int32_t iPort, uint16_t iSlave, uint16_t iReg, uint16_t iData );
static int32_t WriteData( int32_t iPort, uint16_t iSlave, uint16_t iReg, uint16_t *pData, int32_t iSize );
static int32_t ReadData( int32_t iPort, uint16_t iSlave, uint16_t iReg );

//------------------------------------------------------------------------------
CNX_CinemaControl::CNX_CinemaControl()
	: m_bRun( false )
	, m_pCabinet( (uint8_t*)malloc(MAX_CABINET_NUM) )
	, m_iCabinetNum( 0 )
	, m_iScreenType( SCREEN_TYPE_P25 )
{
	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		m_hThread[i] = 0;
	}
}

//------------------------------------------------------------------------------
CNX_CinemaControl::~CNX_CinemaControl()
{
	if( m_pCabinet )
	{
		free( m_pCabinet );
		m_pCabinet = NULL;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::IsBusy( void )
{
	CNX_AutoLock lock( &m_hLock );
	return m_bRun;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Write( int32_t iPort, uint8_t iSlave, uint16_t iReg, uint16_t iData )
{
	return Write( (iPort << 7) | iSlave, iReg, iData );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Write( int32_t iPort, uint8_t iSlave, uint16_t iReg, uint16_t *pData, int32_t iSize )
{
	return Write( (iPort << 7) | iSlave, iReg, pData, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Read( int32_t iPort, uint8_t iSlave, uint16_t iReg )
{
	return Read( (iPort << 7) | iSlave, iReg );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Write( uint8_t iIndex, uint16_t iReg, uint16_t iData )
{
	return Write( iIndex, iReg, &iData, 1 );
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Write( uint8_t iIndex, uint16_t iReg, uint16_t *pData, int32_t iSize )
{
	int32_t iRet = 0;

	{
		CNX_AutoLock lock( &m_hLock );
		if( true == m_bRun )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Resource Busy.\n" );
			return NX_RET_RESOURCE_BUSY;
		}
		m_bRun = true;
	}

	m_iIndex     = iIndex;
	m_iReg       = iReg;
	m_pData      = pData;
	m_iSize      = iSize;
	m_iResult[0] =
	m_iResult[1] =
	m_iResult[2] = 0;

	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		if( !IS_TCON_BROADCAST(iIndex) && !IS_TCON(iIndex) && !IS_PFPGA(iIndex) )
		{
			NxDbgMsg( NX_DBG_WARN, "Fail, Invalid Index. ( port: %d, slave: 0x%02X )\n",
				GetCabinetPort(iIndex), GetCabinetSlave(iIndex) );

			iRet = NX_RET_ERROR;
			break;
		}

		if(	(IS_TCON_BROADCAST(iIndex)	&&  IS_I2C_PFPGA(i)		) ||
			(IS_TCON_LEFT(iIndex)		&& !IS_I2C_TCON_LEFT(i)	) ||
			(IS_TCON_RIGHT(iIndex)		&& !IS_I2C_TCON_RIGHT(i)) ||
			(IS_PFPGA(iIndex)			&& !IS_I2C_PFPGA(i)		) )
		{
			continue;
		}

		if( IS_TCON_BROADCAST(iIndex)	&& !IsCabinetValidPort(i) )
		{
			continue;
		}

		if( 0 != pthread_create( &m_hThread[i], NULL, this->ThreadStub, this ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, pthread_create().\n" );

			iRet = NX_RET_ERROR;
			break;
		}
	}

	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		if( m_hThread[i] )
		{
			pthread_join( m_hThread[i], NULL );
			m_hThread[i] = 0x00;

			iRet = ((0 > iRet) || (0 > m_iResult[i])) ? NX_RET_ERROR : NX_RET_DONE;
		}
	}

	{
		CNX_AutoLock lock( &m_hLock );
		m_bRun = false;
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::Read( uint8_t iIndex, uint16_t iReg )
{
	int32_t iRet = 0;

	{
		CNX_AutoLock lock( &m_hLock );
		if( true == m_bRun )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Resource Busy.\n" );
			return NX_RET_RESOURCE_BUSY;
		}
		m_bRun = true;
	}

	m_iIndex     = iIndex;
	m_iReg       = iReg;
	m_pData      = NULL;
	m_iSize      = -1;
	m_iResult[0] =
	m_iResult[1] =
	m_iResult[2] = 0;

	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		if( !IS_TCON_BROADCAST(iIndex) && !IS_TCON(iIndex) && !IS_PFPGA(iIndex) )
		{
			NxDbgMsg( NX_DBG_WARN, "Fail, Invalid Index. ( port: %d, slave: 0x%02X )\n",
				GetCabinetPort(iIndex), GetCabinetSlave(iIndex) );

			iRet = NX_RET_ERROR;
			break;
		}

		if(	(IS_TCON_BROADCAST(iIndex)	&&  IS_I2C_PFPGA(i)		) ||
			(IS_TCON_LEFT(iIndex)		&& !IS_I2C_TCON_LEFT(i)	) ||
			(IS_TCON_RIGHT(iIndex)		&& !IS_I2C_TCON_RIGHT(i)) ||
			(IS_PFPGA(iIndex)			&& !IS_I2C_PFPGA(i)		) )
		{
			continue;
		}

		if( IS_TCON_BROADCAST(iIndex)	&& !IsCabinetValidPort(i) )
		{
			continue;
		}

		if( 0 != pthread_create( &m_hThread[i], NULL, this->ThreadStub, this ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, pthread_create().\n" );

			iRet = NX_RET_ERROR;
			break;
		}
	}

	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		if( m_hThread[i] )
		{
			pthread_join( m_hThread[i], NULL );
			m_hThread[i] = 0x00;

			// Last Read Data is Valid Data.
			iRet = ((0 > iRet) || (0 > m_iResult[i])) ? NX_RET_ERROR : m_iResult[i];
		}
	}

	{
		CNX_AutoLock lock( &m_hLock );
		m_bRun = false;
	}

	return iRet;
}

//------------------------------------------------------------------------------
void *CNX_CinemaControl::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((CNX_CinemaControl*)pObj)->ThreadProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_CinemaControl::ThreadProc( void )
{
	for( int32_t i = 0; i < MAX_I2C_NUM; i++ )
	{
		if( pthread_self() == m_hThread[i] )
		{
			m_iResult[i] = (0 <= m_iSize) ?
				WriteData( i, GetCabinetSlave(m_iIndex), m_iReg, m_pData, m_iSize ) :
				ReadData( i, GetCabinetSlave(m_iIndex), m_iReg );
		}
	}
}

//------------------------------------------------------------------------------
static int32_t SortScreenTypeP25( const void *pIndex1, const void *pIndex2 )
{
	uint8_t index1 = *(const uint8_t*)pIndex1;
	uint8_t index2 = *(const uint8_t*)pIndex2;

	if( (index1 & 0x7F) < (index2 & 0x7F) )
	{
		return -1;
	}
	else if( (index1 & 0x7F) > (index2 & 0x7F) )
	{
		return 1;
	}
	else
	{
		if( index1 < index2 )
		{
			return 1;
		}
		else if( index1 > index2 )
		{
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t SortScreenTypeP33( const void *pIndex1, const void *pIndex2 )
{
	uint8_t index1 = *(const uint8_t*)pIndex1;
	uint8_t index2 = *(const uint8_t*)pIndex2;

	if( ((index1 & 0x80) >> 7) == ((index2 & 0x80) >> 7) )
	{
		if( index1 < index2 )
		{
			return -1;
		}
		else if( index1 > index2 )
		{
			return 1;
		}
	}
	else
	{
		if( index1 < index2 )
		{
			return -1;
		}
		else if( index1 > index2 )
		{
			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::GetCabinet( uint8_t **ppIndex )
{
	CNX_AutoLock lock( &m_hLock );

	int32_t iWriteData, iReadData = 0x0000;

	memset( m_pCabinet, 0x00, MAX_CABINET_NUM );
	m_iCabinetNum = 0;

#if 1
	for( uint8_t i = 0x00; i < 0xFF; i++ )
	{
		if( (i & 0x7F) < 0x10 )
			continue;

		iWriteData = (uint16_t)NX_GetRandomValue( 0x0000, 0x07FFF );
		if( 0 > WriteData( GetCabinetPort(i), GetCabinetSlave(i), TCON_REG_CHECK_STATUS, (uint16_t)iWriteData ) )
			continue;

		if( 0 > (iReadData = ReadData( GetCabinetPort(i), GetCabinetSlave(i), TCON_REG_CHECK_STATUS ) ) )
			continue;

		if( iWriteData == iReadData )
		{
			printf("Add Cabinet. ( number: %d, port: %d, slave: 0x%02x )\n",
				GetCabinetNumber(i),
				GetCabinetPort(i),
				GetCabinetSlave(i) );

			m_pCabinet[m_iCabinetNum] = i;
			m_iCabinetNum++;
		}
		else
		{
			printf("Wrong Cabinet. ( number: %d, port: %d, slave: 0x%02x, written: 0x%04x, read: 0x%04x )\n",
				GetCabinetNumber(i),
				GetCabinetPort(i),
				GetCabinetSlave(i),
				iWriteData,
				iReadData );
		}
	}

#else
	//
	// Cabinet ID Test Code.
	//
	m_iScreenType = SCREEN_TYPE_P33;

	m_pCabinet[m_iCabinetNum++] = 0xB6;
	m_pCabinet[m_iCabinetNum++] = 0xB7;
	m_pCabinet[m_iCabinetNum++] = 0xB8;
	m_pCabinet[m_iCabinetNum++] = 0x39;
	m_pCabinet[m_iCabinetNum++] = 0x3A;
	m_pCabinet[m_iCabinetNum++] = 0x3B;
	m_pCabinet[m_iCabinetNum++] = 0xBC;
	m_pCabinet[m_iCabinetNum++] = 0xBD;
	m_pCabinet[m_iCabinetNum++] = 0xBE;
	m_pCabinet[m_iCabinetNum++] = 0xBF;

	m_pCabinet[m_iCabinetNum++] = 0x31;
	m_pCabinet[m_iCabinetNum++] = 0x32;
	m_pCabinet[m_iCabinetNum++] = 0x33;
	m_pCabinet[m_iCabinetNum++] = 0x34;
	m_pCabinet[m_iCabinetNum++] = 0x35;
#endif

	//
	// Sorting Cabinet Index
	//
	qsort( m_pCabinet, m_iCabinetNum, sizeof(uint8_t),
		(SCREEN_TYPE_P33 == m_iScreenType) ? SortScreenTypeP33 : SortScreenTypeP25 );

	*ppIndex = m_pCabinet;

	for( int32_t i = 0; i < m_iCabinetNum; i++ )
	{
		NxDbgMsg( NX_DBG_INFO, "Detect Cabinet. ( type: %s, index: 0x%02x, port: %d, slave: 0x%02x, cabinet: %d )\n",
			(SCREEN_TYPE_P33 == m_iScreenType) ? "P33" : "P25",
			m_pCabinet[i],
			GetCabinetPort(m_pCabinet[i]),
			GetCabinetSlave(m_pCabinet[i]),
			GetCabinetNumber(m_pCabinet[i])
		);
	}

	return m_iCabinetNum;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::GetScreenType( void )
{
	CNX_AutoLock lock( &m_hLock );

	int32_t iDefScreenSel;
	int32_t iExpectScreenPitch = -1;
	int32_t iScreenInfo = -1;
	int32_t bMissmatch = false;

	m_iScreenType = SCREEN_TYPE_P25;

	{
		if( 0 > (iDefScreenSel = ReadData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL ) ) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL );
			goto ERROR_I2C;
		}

		if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, 0x0003) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, 0x0003 );
			goto ERROR_I2C;
		}
	}

	{
		for( int32_t j = 0; j < 2; j++ )
		{
			if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, j ) )
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

				int32_t iReadData;
				int32_t iScreenPitch;

				if( 0 > (iReadData = ReadData( GetCabinetPort(i), GetCabinetSlave(i), TCON_REG_PITCH_INFO ) ) )
				{
					continue;
				}

				NxDbgMsg( NX_DBG_DEBUG, "Detected. ( i2c-%d, slave: 0x%02x, model: %d )\n", GetCabinetPort(i), GetCabinetSlave(i), j );

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
		if( bMissmatch == true || 0 > iExpectScreenPitch )
		{
			NxDbgMsg( NX_DBG_WARN, ">>> Unknown ScreenType. --> Set Default ScreenType P2.5\n" );

			if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, 0x0000) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, 0x0000 );
				goto ERROR_I2C;
			}

			if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, iDefScreenSel) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, iDefScreenSel );
				goto ERROR_I2C;
			}

			m_iScreenType = SCREEN_TYPE_P25;
		}
		else
		{
			NxDbgMsg( NX_DBG_WARN, ">>> ScreenType is %s ( %d )\n", (iExpectScreenPitch == 33) ? "P3.3" : "P2.5", iExpectScreenPitch );

			if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, (iExpectScreenPitch == 33) ? 0x0001 : 0x0000) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_MODEL, (iExpectScreenPitch == 33) ? 0x0001 : 0x0000 );
				goto ERROR_I2C;
			}

			if( 0 > WriteData( PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, (iExpectScreenPitch == 33) ? 0x0003 : iDefScreenSel) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					PFPGA_I2C_PORT, PFPGA_I2C_SLAVE, PFPGA_REG_PF_SCREEN_SEL, (iExpectScreenPitch == 33) ? 0x0003 : iDefScreenSel );
				goto ERROR_I2C;
			}

			m_iScreenType = (iExpectScreenPitch == 33) ? SCREEN_TYPE_P33 : SCREEN_TYPE_P25;
		}
	}

	return m_iScreenType;

ERROR_I2C:
	NxDbgMsg( NX_DBG_WARN, ">>> Unknown ScreenType. ( reason: i2c fail ) --> Set Default ScreenType P2.5\n" );
	m_iScreenType = SCREEN_TYPE_P25;
	return SCREEN_TYPE_P25;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::IsCabinetValidPort( int32_t iPort )
{
	for( int32_t i = 0; i < m_iCabinetNum; i++ )
	{
		if( iPort == GetCabinetPort(m_pCabinet[i]) )
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::GetCabinetPort( int32_t iIndex )
{
	return (iIndex & 0x80) >> 7;
}

//------------------------------------------------------------------------------
uint8_t CNX_CinemaControl::GetCabinetSlave( int32_t iIndex )
{
	return (iIndex & 0x7F);
}

//------------------------------------------------------------------------------
int32_t CNX_CinemaControl::GetCabinetNumber( int32_t iIndex )
{
	int32_t iNumber = -1;

	switch( m_iScreenType )
	{
	case SCREEN_TYPE_P25:
		iNumber = ((iIndex & 0x7F) - TCON_BASE_OFFSET);
		break;
	case SCREEN_TYPE_P33:
		iNumber = ((iIndex & 0x7F) - TCON_BASE_OFFSET) + (((iIndex & 0x80) >> 7) * TCON_P33_RIGHT_OFFSET);
		break;
	default:
		break;
	}

	return iNumber;
}

//------------------------------------------------------------------------------
uint8_t CNX_CinemaControl::GetCabinetIndex( int32_t iNumber )
{
	uint8_t iIndex = 0x00;

	switch( m_iScreenType )
	{
	case SCREEN_TYPE_P25:
		iIndex = ((iNumber % TCON_BASE_OFFSET) < 8) ?
			(uint8_t)(iNumber + TCON_BASE_OFFSET) : (uint8_t)((iNumber | 0x80) + TCON_BASE_OFFSET);
		break;
	case SCREEN_TYPE_P33:
		iIndex = (iNumber < TCON_P33_RIGHT_OFFSET) ?
			(uint8_t)(iNumber + TCON_BASE_OFFSET) : (uint8_t)(((iNumber-TCON_P33_RIGHT_OFFSET) | 0x80) + TCON_BASE_OFFSET);
		break;
	default:
		break;
	}

	return iIndex;
}

//------------------------------------------------------------------------------
static int32_t WriteData( int32_t iPort, uint16_t iSlave, uint16_t iReg, uint16_t iData )
{
	CNX_I2C i2c( iPort );

	if( 0 > i2c.Open() )
	{
		// printf( "Fail, Open(). ( i2c-%d )\n", iPort );
		return -1;
	}

	if( 0 > i2c.Write( iSlave, iReg, iData ) )
	{
		// printf( "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
		// 	iPort, iSlave, iReg, iData );
		return -1;
	}

	i2c.Close();

	return 0;
}

//------------------------------------------------------------------------------
static int32_t WriteData( int32_t iPort, uint16_t iSlave, uint16_t iReg, uint16_t *pData, int32_t iSize )
{
	CNX_I2C i2c( iPort );

	if( 0 > i2c.Open() )
	{
		// printf( "Fail, Open(). ( i2c-%d )\n", iPort );
		return -1;
	}

	if( 0 > i2c.Write( iSlave, iReg, pData, iSize ) )
	{
		// printf( "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x%s )\n",
		// 	iPort, iSlave, iReg, pData[0], (1 < iSize) ? ", ..." : "" );
		return -1;
	}

	i2c.Close();

	return 0;
}

//------------------------------------------------------------------------------
static int32_t ReadData( int32_t iPort, uint16_t iSlave, uint16_t iReg )
{
	CNX_I2C i2c( iPort );

	if( 0 > i2c.Open() )
	{
		// printf( "Fail, Open(). ( i2c-%d )\n", iPort );
		return -1;
	}

	int32_t iReadData;
	if( 0 > (iReadData = i2c.Read( iSlave, iReg )) )
	{
		// printf("Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	iPort, iSlave, iReg );
		return -1;
	}

	i2c.Close();

	return iReadData;
}

//------------------------------------------------------------------------------
CNX_CinemaControl*	CNX_CinemaControl::m_pstInstance = NULL;

CNX_CinemaControl* CNX_CinemaControl::GetInstance( void )
{
	if( NULL == m_pstInstance )
	{
		m_pstInstance = new CNX_CinemaControl();
	}

	return m_pstInstance;
}

//------------------------------------------------------------------------------
void CNX_CinemaControl::ReleaseInstance( void )
{
	if( m_pstInstance )
	{
		delete m_pstInstance;
		m_pstInstance = NULL;
	}
}
