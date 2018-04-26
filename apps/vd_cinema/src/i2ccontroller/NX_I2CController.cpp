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

#include <NX_I2CController.h>
#include <CNX_BaseClass.h>
#include <NX_IPCCommand.h>	//command definitions

#include "NX_Command.h"	//methods for cmd

#include "NX_Utils.h"
#include <stdlib.h>     /* srand, rand */
#include "NX_Version.h"
#define NX_DTAG	"[NX_I2CController]"
#include <NX_DbgMsg.h>

#define TESTMODE 1

static pthread_mutex_t pLock = PTHREAD_MUTEX_INITIALIZER;

enum
{
	SUCCEED = 0 ,
	I2C_ERR = -1 ,
	BUSY_STATE = -2 ,
	BURST_BUSY_STATE = -3 ,
	NO_CMD_FOUND = -4
};


class CNX_I2CController : protected CNX_Thread
{
private:
	CNX_I2CController();
	~CNX_I2CController();

public:
	static CNX_I2CController* GetInstance();
	static void ReleaseInstance();

	//All I2C operation is done inside this method
	int32_t SendData(uint32_t cmd, void *pPayload, uint32_t payloadSize, uint8_t** pResult , uint32_t* iResultSize);

	void SetState(int32_t iState);
	int32_t GetState();

	void SetOwner(void* pOwner);
	void* GetOwner();

	static void *ThreadStubRelease( void *arg )
	{
		CNX_I2CController *pObj = (CNX_I2CController*) arg;
		pObj->ThreadProcRelease();
		return (void*)0xDeadC0de;
	}
	void ThreadProcRelease();

private:
	//bool m_IsRunning;						//--> parent class
	virtual void ThreadProc();			//--> implements parent class

	//int32_t IsThreadRunning();

	//int32_t BroadCastCommand(int32_t fd, uint32_t cmd, void *pPayload, int32_t payloadSize, uint8_t* result); // use thread methods....
	//some vars for BroadCastCommand's return

	int32_t m_iState;			//if this class is being used now, m_iState = true
	void* m_pOwner;				//for Burst command
	static CNX_I2CController* m_pInstance;
};

//------------------------------------------------------------------------------
CNX_I2CController::CNX_I2CController()
{
	m_iState = 0;
	m_pOwner = NULL;
}

//------------------------------------------------------------------------------
CNX_I2CController::~CNX_I2CController()
{
}

//------------------------------------------------------------------------------
void CNX_I2CController::SetOwner(void* pOwner)
{
	m_pOwner = pOwner;
}

//------------------------------------------------------------------------------
void* CNX_I2CController::GetOwner()
{
	return m_pOwner;
}

//------------------------------------------------------------------------------
void CNX_I2CController::SetState(int32_t iState)
{
	m_iState = iState;
}

//------------------------------------------------------------------------------
int32_t CNX_I2CController::GetState()
{
	return m_iState;
}

//------------------------------------------------------------------------------
void CNX_I2CController::ThreadProcRelease()
{
	while(1)
	{
		usleep(1000000);	// 1 sec
		if(0 == m_pInstance->GetState())
		{
			m_pInstance->SetState(1);
			break;
		}
	}
}



CNX_I2CController* CNX_I2CController::m_pInstance = NULL;
//------------------------------------------------------------------------------
CNX_I2CController* CNX_I2CController::GetInstance()
{
	if(NULL == m_pInstance)
	{
		m_pInstance = new CNX_I2CController();
	}

	return (CNX_I2CController*)m_pInstance;
}

//------------------------------------------------------------------------------
void CNX_I2CController::ReleaseInstance()
{
	// if (m_IsRunning)
	// {
	// 	Stop();
	// }

	if(NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

//------------------------------------------------------------------------------
void CNX_I2CController::ThreadProc()
{
	//do BroadCastCommand for one port of I2C
	//do something for one port
}

//------------------------------------------------------------------------------
int32_t CNX_I2CController::SendData(uint32_t cmd, void *pPayload, uint32_t payloadSize, uint8_t** pResult , uint32_t* iResultSize)
{
	int32_t iRet = SUCCEED;
	uint8_t* temp = (uint8_t*)pPayload;
#if TESTMODE
	if (TCON_CMD_STATUS != cmd && CINEMA_CMD_BURST_START != cmd && CINEMA_CMD_BURST_STOP != cmd)
	{
		NxDbgMsg(NX_DBG_INFO, "SendData +++++\n" );
		NxDbgMsg(NX_DBG_INFO, "cmd = 0x%08x\n", cmd );
		NxDbgMsg(NX_DBG_INFO, "payloadSize = %u\n", payloadSize );
		NxDbgMsg(NX_DBG_INFO, "payload : \n" );

		for(uint32_t i = 0 ; i < payloadSize ; i++)
		{
			NxDbgMsg(NX_DBG_INFO, "%02x\n" , temp[i]  );
		}
		NxDbgMsg(NX_DBG_INFO, "===============================\n" );
	}


	switch(cmd)
	{
	//
	//	TCON Commands
	//
	case CINEMA_CMD_BURST_START:
		iRet =  CMD_BURSTStart( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;
	case CINEMA_CMD_BURST_STOP:
		iRet =  CMD_BURSTStop( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;
	case TCON_CMD_STATUS:
		iRet =  TCON_Status( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;
	default :
		*iResultSize = 4;
		*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*iResultSize) ) ;
		if(NULL == *pResult) NxDbgMsg(NX_DBG_INFO, "I2CController NX_SendData , malloc err\n");

		uint8_t* pTemp = *pResult;
		pTemp[0] = 0x00;
		pTemp[1] = 0x00;
		pTemp[2] = 0x00;
		pTemp[3] = 0x01;
	}

	if (TCON_CMD_STATUS != cmd && CINEMA_CMD_BURST_START != cmd && CINEMA_CMD_BURST_STOP != cmd)
	{
		NxDbgMsg(NX_DBG_INFO, "=============================== after I2C\n" );
		NxDbgMsg(NX_DBG_INFO, " iResultSize = %u\n", (*iResultSize) );
		NxDbgMsg(NX_DBG_INFO, " pResult : \n" );
		temp = *pResult;
		for(uint32_t i = 0 ; i < (*iResultSize) ; i++)
		{
			NxDbgMsg(NX_DBG_INFO, "%02x\n" , temp[i]  );
		}
		NxDbgMsg(NX_DBG_INFO, "===============================\n" );
		NxDbgMsg(NX_DBG_INFO, "SendData -----\n" );
	}
	SetState(0);
	return iRet;

#endif

	switch(cmd)
	{
	//
	//	TCON Commands
	//
	case CINEMA_CMD_BURST_START:
		iRet =  CMD_BURSTStart( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case CINEMA_CMD_BURST_STOP:
		iRet =  CMD_BURSTStop( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;


	case TCON_CMD_REG_WRITE:
		iRet =  TCON_RegWrite( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_REG_READ:
		iRet =  TCON_RegRead( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_REG_BURST_WRITE:
		iRet =  TCON_RegBurstWrite( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_INIT:
		iRet =  TCON_Init( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_STATUS:
		iRet =  TCON_Status( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_DOOR_STATUS:
		iRet =  TCON_DoorStatus( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_LVDS_STATUS:
		iRet =  TCON_LvdsStatus( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_BOOTING_STATUS:
		iRet =  TCON_BootingStatus( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_MODE_NORMAL:
		iRet =  TCON_LedModeNormal( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_MODE_LOD:
		iRet =  TCON_LedModeLod( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_OPEN_NUM:
		iRet =  TCON_LedOpenNum( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_OPEN_POS:
		iRet =  TCON_LedOpenPos( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:
		iRet =  TCON_TestPattern( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_TGAM_R:
	case TCON_CMD_TGAM_G:
	case TCON_CMD_TGAM_B:
		iRet =  TCON_TargetGamma( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_DGAM_R:
	case TCON_CMD_DGAM_G:
	case TCON_CMD_DGAM_B:
		iRet =  TCON_DeviceGamma( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_PIXEL_CORRECTION:
		iRet =  TCON_PIXELCorrection( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_PIXEL_CORRECTION_EXTRACT:
		iRet =  TCON_PIXELCorrectionExtract( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_WHITE_SEAM_READ:
		iRet =  TCON_WhiteSeamRead( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_WHITE_SEAM_WRITE:
		iRet =  TCON_WhiteSeamWrite( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:
		iRet =  TCON_OptionalData( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_SW_RESET:
		iRet =  TCON_SwReset( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_EEPROM_READ:
		iRet =  TCON_EEPRomRead( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case TCON_CMD_VERSION:
		iRet =  TCON_Version( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	//
	//	PFPGA Commands
	//
	case PFPGA_CMD_REG_WRITE:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_REG_WRITE ):
		iRet =  PFPGA_RegWrite( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PFPGA_CMD_REG_READ:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_REG_READ ):
		iRet =  PFPGA_RegRead( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PFPGA_CMD_STATUS:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_STATUS ):
		iRet =  PFPGA_Status( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PFPGA_CMD_MUTE:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_MUTE ):
		iRet =  PFPGA_Mute( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PFPGA_CMD_UNIFORMITY_DATA:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_UNIFORMITY_DATA ):
		iRet =  PFPGA_UniformityData( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PFPGA_CMD_VERSION:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_VERSION ):
		iRet =  PFPGA_Version( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	//
	//	Platform Commands
	//
	case PLATFORM_CMD_NAP_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_NAP_VERSION ):
		iRet =  PLAT_NapVersion( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PLATFORM_CMD_SAP_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_SAP_VERSION ):
		iRet =  PLAT_SapVersion( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PLATFORM_CMD_IPC_SERVER_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_IPC_SERVER_VERSION ):
		iRet =  PLAT_IpcServerVersion( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	case PLATFORM_CMD_IPC_CLIENT_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_IPC_CLIENT_VERSION ):
		iRet =  PLAT_IpcClientVersion( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	//
	//	IMB Commands
	//
	case IMB_CMD_CHANGE_CONTENTS:
	case GDC_COMMAND( CMD_TYPE_IMB, IMB_CMD_CHANGE_CONTENTS ):
		iRet = IMB_ChangeContents( cmd, (uint8_t*)pPayload, payloadSize , pResult , iResultSize);
		break;

	default:
		SetState(0);
		return NO_CMD_FOUND;
	}

#if TESTMODE
	NxDbgMsg(NX_DBG_INFO, "=============================== after I2C\n" );
	NxDbgMsg(NX_DBG_INFO, " ResultSize = %u\n", (*iResultSize) );
	NxDbgMsg(NX_DBG_INFO, " pResult : \n" );
	temp = *pResult;
	for(uint32_t i = 0 ; i < (*iResultSize) ; i++)
	{
		NxDbgMsg(NX_DBG_INFO, "%02x\n" , temp[i]  );
	}
	NxDbgMsg(NX_DBG_INFO, "===============================\n" );
	NxDbgMsg(NX_DBG_INFO, "SendData -----\n" );
#endif

	SetState(0);
	return iRet;
}



//------------------------------------------------------------------------------
//returns fail < 0 , succeed : 0
int32_t NX_SendData(void* pOwner, uint32_t cmd, void *pPayload, uint32_t payloadSize, uint8_t** pResult , uint32_t* iResultSize)
{
	pthread_mutex_lock( &pLock );

	CNX_I2CController* pI2CController = CNX_I2CController::GetInstance();

	if(pI2CController->GetState())
	{
		*iResultSize = 4;
		*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*iResultSize) ) ;
		uint8_t* tempBuf = *pResult;
		tempBuf[0] = 0xFF;
		tempBuf[1] = 0xFF;
		tempBuf[2] = 0xFF;
		tempBuf[3] = 0xFE;

		pthread_mutex_unlock(&pLock);
		return BUSY_STATE;
	}
	pI2CController->SetState(1);

	if(NULL != pI2CController->GetOwner())
	{
		if(pOwner != pI2CController->GetOwner())
		{
			*iResultSize = 4;
			*pResult = (uint8_t*) malloc( sizeof(uint8_t) * (*iResultSize) ) ;
			uint8_t* tempBuf = *pResult;
			tempBuf[0] = 0xFF;
			tempBuf[1] = 0xFF;
			tempBuf[2] = 0xFF;
			tempBuf[3] = 0xFE;

			pthread_mutex_unlock(&pLock);
			pI2CController->SetState(0);
			return BURST_BUSY_STATE;
		}
		if(CINEMA_CMD_BURST_STOP == cmd)	// BURST COMMAND END
		{
			pI2CController->SetOwner(NULL);
		}
	}
	else
	{
		if(CINEMA_CMD_BURST_START == cmd)	//BURST COMMAND START
		{
			pI2CController->SetOwner(pOwner);	//after done Burst.. SetOwner(NULL)
		}
	}

	pthread_mutex_unlock(&pLock);
	return pI2CController->SendData(cmd, pPayload, payloadSize, pResult , iResultSize);
}

//------------------------------------------------------------------------------
//will be used only one time in main.cpp's terminate routine of N.AP Server
int32_t NX_ReleaseInstance()
{
	pthread_mutex_lock( &pLock );
	CNX_I2CController* pI2CController = CNX_I2CController::GetInstance();
	//
	// should wait until release is possible...
	//

	if(pI2CController->GetState())
	{
		//thread to wait
		pthread_t thread;
		pthread_create( &thread, NULL, CNX_I2CController::ThreadStubRelease, pI2CController );
		pthread_join( thread, NULL );
	}

	pthread_mutex_unlock(&pLock);

	pI2CController->ReleaseInstance();
	return 0;
}
