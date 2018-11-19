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

#ifndef __CNX_CINEMAMANAGER_H__
#define __CNX_CINEMAMANAGER_H__

#include <stdio.h>
#include <pthread.h>

#include <CNX_Base.h>
#include <CNX_CinemaControl.h>

class CNX_CinemaManager
{
public:
	static CNX_CinemaManager* GetInstance( void );
	static void ReleaseInstance( void );

public:
	void	SetNapVersion( uint8_t *pVersion, int32_t iSize );
	void	SetSapVersion( uint8_t *pVersion, int32_t iSize );
	void	SetIpcVersion( uint8_t *pVersion, int32_t iSize );
	void	SetTmsVersion( uint8_t *pVersion, int32_t iSize );

	int32_t IsBusy();
	int32_t SendCommand( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

private:
	CNX_CinemaManager();
	~CNX_CinemaManager();

	static void* ThreadCommandStub( void *pObj );
	void ThreadCommandProc( void );

private:
	int32_t ProcessCommand( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	TCON Commands
	int32_t TCON_RegWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_RegRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_RegBurstWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	int32_t TCON_Init( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_Status( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_DoorStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_LvdsStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_BootingStatus( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_LedModeNormal( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_LedModeLod( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_LedOpenNum( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_LedOpenPos( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_TestPattern( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t	TCON_TargetGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t	TCON_DeviceGamma( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_DotCorrection( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_DotCorrectionExtract( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_WhiteSeamRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_WhiteSeamWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_OptionalData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_SwReset( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_EEPRomRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_Mute( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	PFPGA Commands
	int32_t PFPGA_RegWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_RegRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_RegBurstWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	int32_t PFPGA_Status( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_Mute( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_UniformityData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_WriteConfig( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	Platform Commands
	int32_t PLAT_NapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_SapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_IpcVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_TmsVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_IsBusy( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_ScreenType( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_CheckCabinet( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	int32_t PLAT_ConfigUpload( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_ConfigDownload( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_ConfigDelete( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

private:
	int32_t TestPatternDci(  uint8_t index, uint8_t pattern );
	int32_t TestPatternColorBar( uint8_t index, uint8_t pattern );
	int32_t TestPatternFullScreenColor( uint8_t index, uint8_t pattern );
	int32_t TestPatternGrayScale( uint8_t index, uint8_t pattern );
	int32_t TestPatternDot( uint8_t index, uint8_t pattern );
	int32_t TestPatternDiagonal( uint8_t index, uint8_t pattern );

	void	WaitTime( uint64_t iWaitTime, const char *pMsg = NULL );

private:
	enum { NX_RET_DONE = 0, NX_RET_ERROR = -1, NX_RET_RESOURCE_BUSY = -2 };
	enum { MAX_LOD_MODULE = 12, MAX_STR_SIZE = 1024 };

	static CNX_CinemaManager*	m_pstInstance;
	static CNX_Mutex			m_hInstanceLock;

	CNX_CinemaControl*	m_pCinema;

	CNX_Mutex		m_hLock;
	pthread_t		m_hThreadCommand;
	int32_t			m_bRun;

	int32_t			m_iCmd;
	uint8_t*		m_pInBuf;
	int32_t			m_iInSize;
	uint8_t*		m_pOutBuf;
	int32_t			m_iOutSize;

	uint8_t			m_NapVersion[MAX_STR_SIZE];		// Version for NAP
	uint8_t			m_SapVersion[MAX_STR_SIZE];		// Version for SAP
	uint8_t			m_IpcVersion[MAX_STR_SIZE];		// Version for IPC Server
	uint8_t			m_TmsVersion[MAX_STR_SIZE];		// Version for TMS Server

	enum { MAX_TEST_PATTERN = 6 };
	int32_t		(CNX_CinemaManager::*m_pTestPatternFunc[MAX_TEST_PATTERN])( uint8_t, uint8_t );

	enum { SCREEN_TYPE_P25 = 0, SCREEN_TYPE_P33 = 1 };
	int32_t		m_iScreenType;

private:
	CNX_CinemaManager (const CNX_CinemaManager &Ref);
	CNX_CinemaManager &operator=(const CNX_CinemaManager &Ref);
};

#endif	// __CNX_CINEMAMANAGER_H__
