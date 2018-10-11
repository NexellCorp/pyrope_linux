//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
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
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_I2C.h>
#include <CNX_EEPRom.h>
#include <CNX_EEPRomData.h>
#include <NX_CinemaRegister.h>
#include <SockUtils.h>
#include <ipc_protocol.h>

#include <NX_CinemaCommand.h>
#include <NX_IPCServer.h>
#include <CNX_Base.h>
#include <CNX_CinemaManager.h>

#include <NX_Utils.h>
#include <NX_Version.h>

#define NX_DTAG	"[IPC Server]"
#include <NX_DbgMsg.h>

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

#define MAX_TIMEOUT		3000

//------------------------------------------------------------------------------
//
//	IPC Server APIs
//
class CNX_IPCServer : protected CNX_Thread
{
public:
	CNX_IPCServer();
	~CNX_IPCServer();

	static CNX_IPCServer* GetInstance();
	static void ReleaseInstance();

public:
	int32_t StartServer();
	void StopServer();

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	//	Local Socket
	int32_t WaitClient();
	int32_t ReadData(int32_t iSock, uint8_t *pBuf, int32_t iSize);

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	pthread_t	m_hThread;

	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Command(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 8];
	uint8_t		m_RecvBuf[MAX_PAYLOAD_SIZE + 8];

	CNX_CinemaManager* m_pCinema;

private:
	//	For Singletone
	static CNX_IPCServer	*m_psInstance;
};

CNX_IPCServer* CNX_IPCServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
CNX_IPCServer::CNX_IPCServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
{
	char szVersion[1024] = { 0x00, };
	snprintf( (char*)szVersion, sizeof(szVersion), "%s ( %08lld-%06lld )", NX_VERSION_IPC_SERVER, NX_DATE(), NX_TIME() );

	m_pCinema = CNX_CinemaManager::GetInstance();
	m_pCinema->SetIpcVersion( (uint8_t*)szVersion, strlen(szVersion) );
}

//------------------------------------------------------------------------------
CNX_IPCServer::~CNX_IPCServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::StartServer()
{
	if( m_bThreadRun )
		return 1;

	m_hSocket = LS_Open( IPC_SERVER_FILE );
	if( m_hSocket < 0 )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, LS_Open().\n" );
		return -1;
	}

	if( 0 > listen(m_hSocket, 5) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail : listen (err = %d)\n", errno );
		goto ERROR_IPC;
	}

	m_bThreadRun = true;

	if( 0 != Start() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Start().\n" );
		goto ERROR_IPC;
	}

	return 0;

ERROR_IPC:
	m_bThreadRun = false;

	if( 0 < m_hSocket )
	{
		close( m_hSocket );
		m_hSocket = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
void CNX_IPCServer::StopServer()
{
	if( m_bThreadRun )
	{
		m_bThreadRun = false;
		Stop();
	}
}

//------------------------------------------------------------------------------
void CNX_IPCServer::ThreadProc()
{
	int32_t iReadSize;
	uint32_t iKey = 0, iCmd = 0;
	uint8_t *pPayload;
	int32_t iPayloadSize;

	uint8_t outBuf[MAX_PAYLOAD_SIZE];
	int32_t iOutSize;
	int32_t iSendSize;

	while( m_bThreadRun )
	{
		uint8_t *pRecvBuf = m_RecvBuf;
		int32_t iClientSocket = WaitClient();
		if( !iClientSocket )
		{
			continue;
		}
		else if( 0 > iClientSocket )
		{
			break;
		}

		//	Find Key Code
		do{
			uint8_t temp;
			iReadSize = ReadData( iClientSocket, &temp, 1 );
			if( iReadSize != 1 )
			{
				NxErrMsg("Read Error!!!\n");
				goto ERROR_IPC;
			}
			iKey = (iKey<<8) | temp;
			if( iKey == KEY_NXP )
			{
				break;
			}
		} while(1);

		//	Key
		*pRecvBuf++ = (( iKey >> 24) & 0xFF);
		*pRecvBuf++ = (( iKey >> 16) & 0xFF);
		*pRecvBuf++ = (( iKey >>  8) & 0xFF);
		*pRecvBuf++ = (( iKey      ) & 0xFF);

		//	Command
		iReadSize = ReadData( iClientSocket, pRecvBuf, 2 );
		if( 2 != iReadSize )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read Command. ( read: %d, expected: 2 )\n", iReadSize );
			goto ERROR_IPC;
		}

		iCmd = MAKE_COMMAND( pRecvBuf[0], pRecvBuf[1] );
		pRecvBuf += 2;

		//	Read Length
		iReadSize = ReadData( iClientSocket, pRecvBuf, 2 );
		if( 2 != iReadSize )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read Length. ( read: %d, expected: 2 )\n", iReadSize );
			goto ERROR_IPC;
		}

		iPayloadSize = MAKE_LENGTH( pRecvBuf[0], pRecvBuf[1] );
		pRecvBuf += 2;

		//	Size Check
		if( iPayloadSize+8 > (int32_t)sizeof(m_RecvBuf) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Check Payload Size.\n" );
			goto ERROR_IPC;
		}

		//	Read all data
		if( iPayloadSize != 0 )
		{
			iReadSize = ReadData( iClientSocket, pRecvBuf, iPayloadSize );
			if( iReadSize != iPayloadSize )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read Payload. ( read: %d, expected: %d )\n", iReadSize, iPayloadSize );
				goto ERROR_IPC;
			}
		}

		pPayload = pRecvBuf;

		NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
		NxDbgMsg( NX_DBG_VBS, "iCmd = 0x%08x, readSize = %d\n", iCmd, iReadSize );

		if( !m_pCinema->IsBusy() )
		{
			m_pCinema->SendCommand( iCmd, pPayload, iPayloadSize, outBuf, &iOutSize );
		}
		else
		{
			outBuf[0] = 0xFF;
			outBuf[1] = 0xFF;
			outBuf[2] = 0xFF;
			outBuf[3] = 0xFE;
			iOutSize  = 4;
		}

		iSendSize = IPC_MakePacket( KEY_SEC, iCmd, outBuf, iOutSize, m_SendBuf, sizeof(m_SendBuf) );
		write( iClientSocket, m_SendBuf, iSendSize );

ERROR_IPC:
		if( 0 < iClientSocket )
		{
			close( iClientSocket );
			iClientSocket = -1;
		}
	}
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::WaitClient()
{
	int32_t ret;
	struct pollfd hPoll;

	hPoll.fd		= m_hSocket;
	hPoll.events	= POLLIN | POLLERR;
	hPoll.revents	= 0;
	ret = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );

	if( 0 < ret )
	{
		if( hPoll.revents & POLLIN )
		{
			int32_t clnSocket;
			struct sockaddr_un clntAddr;
			int32_t clntAddrSize;

			clntAddrSize = sizeof( clntAddr );
			clnSocket = accept( m_hSocket, (struct sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize );

			if( 0 > clnSocket )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, accept().\n" );
				return -1;
			}

			return clnSocket;
		}
	}
	else if( 0 > ret )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, poll().\n");
		return -1;
	}

	NxDbgMsg( NX_DBG_VBS, "Timeout. WaitClient().\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::ReadData(int32_t iSock, uint8_t *pBuf, int32_t iSize)
{
	int32_t iReadSize, iTotalSize=0;
	do
	{
		iReadSize = read( iSock, pBuf, iSize );
		if( iReadSize < 0 )
			return -1;

		iSize      -= iReadSize;
		pBuf       += iReadSize;
		iTotalSize += iReadSize;
	} while(iSize > 0);
	return iTotalSize;
}


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_IPCServer* CNX_IPCServer::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_IPCServer();
	}
	return (CNX_IPCServer*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_IPCServer::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}


//------------------------------------------------------------------------------
//
//	External APIs
//

//------------------------------------------------------------------------------
int32_t NX_IPCServerStart()
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	return hIpc->StartServer();
}

//------------------------------------------------------------------------------
void NX_IPCServerStop()
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	hIpc->StopServer();
}

//------------------------------------------------------------------------------
void NX_SetNapVersion( uint8_t *pVersion, int32_t iSize )
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	CNX_CinemaManager *hCinema = CNX_CinemaManager::GetInstance();

	UNUSED( hIpc );

	hCinema->SetNapVersion( pVersion, iSize );
}

//------------------------------------------------------------------------------
void NX_SetSapVersion( uint8_t *pVersion, int32_t iSize )
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	CNX_CinemaManager *hCinema = CNX_CinemaManager::GetInstance();

	UNUSED( hIpc );

	hCinema->SetSapVersion( pVersion, iSize );
}
