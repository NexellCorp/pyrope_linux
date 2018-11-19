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

#ifndef __CNX_CINEMACONTROL_H__
#define __CNX_CINEMACONTROL_H__

#include "CNX_Base.h"

//------------------------------------------------------------------------------
#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

//------------------------------------------------------------------------------
class CNX_CinemaControl
{
public:
	static CNX_CinemaControl* GetInstance( void );
	static void ReleaseInstance( void );

public:
	int32_t IsBusy( void );

	//
	//	Port & Slave Base
	//
	int32_t Write( int32_t iPort, uint8_t iSlave, uint16_t iReg, uint16_t iData );
	int32_t Write( int32_t iPort, uint8_t iSlave, uint16_t iReg, uint16_t *pData, int32_t iSize );
	int32_t Read( int32_t iPort, uint8_t iSlave, uint16_t iReg );

	//
	//	Index( Port + Slave ) Base
	//
	int32_t Write( uint8_t iIndex, uint16_t iReg, uint16_t iData );
	int32_t Write( uint8_t iIndex, uint16_t iReg, uint16_t *pData, int32_t iSize );
	int32_t Read( uint8_t iIndex, uint16_t iReg );

	//
	//	Cabinet
	//
	int32_t GetCabinet( uint8_t **ppIndex );
	int32_t GetScreenType( void );

	int32_t IsCabinetValidPort( int32_t iPort );
	int32_t GetCabinetPort( int32_t iIndex );
	uint8_t GetCabinetSlave( int32_t iIndex );
	int32_t GetCabinetNumber( int32_t iIndex );
	uint8_t GetCabinetIndex( int32_t iNumber );

private:
	CNX_CinemaControl();
	~CNX_CinemaControl();

	static void* ThreadStub( void *pObj );
	void ThreadProc( void );

private:
	enum { NX_RET_DONE = 0, NX_RET_ERROR = -1, NX_RET_RESOURCE_BUSY = -2 };
	enum { NX_I2C_TCON_LEFT = 0, NX_I2C_TCON_RIGHT = 1, NX_I2C_PFPGA = 2, MAX_I2C_NUM = 3 };
	enum { MAX_CABINET_NUM = 255 };
	enum { TCON_BASE_OFFSET = 16, TCON_P33_RIGHT_OFFSET = 200 };

	static CNX_CinemaControl*	m_pstInstance;

	CNX_Mutex		m_hLock;
	pthread_t		m_hThread[MAX_I2C_NUM];
	int32_t			m_bRun;

	uint8_t			m_iIndex;
	uint16_t		m_iReg;
	uint16_t*		m_pData;
	int32_t			m_iSize;

	int32_t			m_iResult[MAX_I2C_NUM];

	uint8_t*		m_pCabinet;
	int32_t			m_iCabinetNum;

	enum { SCREEN_TYPE_P25 = 0, SCREEN_TYPE_P33 = 1 };
	int32_t			m_iScreenType;

private:
	CNX_CinemaControl (const CNX_CinemaControl &Ref);
	CNX_CinemaControl &operator=(const CNX_CinemaControl &Ref);
};

#endif	// __CNX_CINEMACONTROL_H__
