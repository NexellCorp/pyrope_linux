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

#include <stdint.h>
#include <pthread.h>

#include <CNX_Preference.h>
#include <CNX_TReg.h>
#include <CNX_PReg.h>
#include <CNX_Gamma.h>
#include <CNX_Uniformity.h>

class CNX_CinemaManager
{
public:
	static CNX_CinemaManager* GetInstance( void );
	static void ReleaseInstance( void );

public:
	int32_t Init();
	int32_t Deinit();

	void	SetNapVersion( const char *pVersion );
	void	SetSapVersion( const char *pVersion );
	void	SetIpcVersion( const char *pVersion );
	void	SetTmsVersion( const char *pVersion );

	int32_t GetSlave();
	int32_t GetCabinet();

	int32_t IsValidPort( int32_t iPort );
	int32_t IsBusy();

	int32_t SendCommand( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

private:
	CNX_CinemaManager();
	~CNX_CinemaManager();

	static void*	ThreadInitialStub( void *pObj );
	void			ThreadInitialProc( void );

	static void*	ThreadCommandStub( void *pObj );
	void			ThreadCommandProc( void );

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
	int32_t TCON_AccumulateTime( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_OptionalData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_SwReset( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_EEPRomRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t TCON_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	PFPGA Commands
	int32_t PFPGA_RegWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_RegRead( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_RegBurstWrite( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	int32_t PFPGA_Status( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_Mute( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_UniformityData( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PFPGA_Version( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	Platform Commands
	int32_t PLAT_NapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_SapVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_IpcVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_TmsVersion( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t PLAT_IsBusy( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	int32_t PLAT_ChangeMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

	//	IMB Commands
	int32_t IMB_ChangeContents( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t IMB_ChangeMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t IMB_GetCurrentMode( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t IMB_WriteFile( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );
	int32_t IMB_ReadModeDesc( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize );

private:
	enum { NX_RET_DONE = 0, NX_RET_ERROR = -1, NX_RET_RESOURCE_BUSY = -2 };
	enum { MAX_SLAVE_NUM = 255 };
	enum { MAX_STR_SIZE = 1024 };

	static CNX_CinemaManager*	m_pstInstance;
	static CNX_Mutex			m_hInstanceLock;

	CNX_Mutex		m_hLock;
	pthread_t		m_hThreadCommand;
	pthread_t		m_hThreadInitial;
	int32_t			m_bRun;

	int32_t			m_iCmd;
	uint8_t*		m_pInBuf;
	int32_t			m_iInSize;
	uint8_t*		m_pOutBuf;
	int32_t			m_iOutSize;

	uint8_t			m_iSlave[MAX_SLAVE_NUM];
	int32_t			m_iSlaveNum;

	uint8_t			m_NapVersion[MAX_STR_SIZE];		// Version for NAP
	uint8_t			m_SapVersion[MAX_STR_SIZE];		// Version for SAP
	uint8_t			m_IpcVersion[MAX_STR_SIZE];		// Version for IPC Server
	uint8_t			m_TmsVersion[MAX_STR_SIZE];		// Version for TMS Server

	int32_t			m_iCurMode;

	CNX_Preference*	m_pPreference;
	CNX_TReg*		m_pTReg;
	CNX_PReg*		m_pPReg;
	CNX_Gamma*		m_pGamma;
	CNX_Uniformity*	m_pUniformity;
	// CNX_PixelCorrection* m_pPiexelCorrection;

private:
	CNX_CinemaManager (const CNX_CinemaManager &Ref);
	CNX_CinemaManager &operator=(const CNX_CinemaManager &Ref);
};

#endif	// __CNX_CINEMACONTROL_H__
