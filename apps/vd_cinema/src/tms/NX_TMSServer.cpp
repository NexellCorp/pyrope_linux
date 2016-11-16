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
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_BaseClass.h>
#include <tms_protocol.h>
#include <SockUtils.h>

#include <NX_TMSServer.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			10000
#define MAX_PAYLOAD_SIZE	65535

//------------------------------------------------------------------------------
class CNX_TMSServer
	: protected CNX_Thread
{
public:
	CNX_TMSServer();
	~CNX_TMSServer();

	static CNX_TMSServer* GetInstance();
	static void ReleaseInstance();

	int32_t	StartServer( int32_t iPort );
	void	StopServer();

	void	RegisterCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ) , void *pParam );

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	int32_t WaitClient();

	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );

	int32_t ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize );

private:
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	int32_t		(*m_cbCallback)( void *pObj, int32_t iEventCode , void *pData, int32_t iSize );
	void		*m_hParam;

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

//------------------------------------------------------------------------------
CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

CNX_TMSServer::CNX_TMSServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
	, m_cbCallback( NULL )
	, m_hParam( NULL )
{
}

//------------------------------------------------------------------------------
CNX_TMSServer::~CNX_TMSServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
void CNX_TMSServer::RegisterCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	m_cbCallback = callback;
	m_hParam = pParam;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::StartServer( int32_t iPort )
{
	if( true == m_bThreadRun )
	{
		return 1;
	}

	m_hSocket = TCP_Open( iPort );
	if( m_hSocket < 0 )
	{
		NxErrMsg( "Fail, TCP_Open().\n" );
		return -1;
	}

	if( 0 > listen(m_hSocket, 5) )
	{
		NxErrMsg( "Fail : listen (err = %d)\n", errno );
		goto ErrorExit;
	}

	m_bThreadRun = true;

	if( 0 != Start() )
	{
		NxErrMsg( "Fail, Start().\n" );
		goto ErrorExit;
	}

	return 0;

ErrorExit:
	m_bThreadRun = false;

	if( 0 < m_hSocket )
	{
		close( m_hSocket );
		m_hSocket = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
void CNX_TMSServer::StopServer()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		Stop();
	}
}

//------------------------------------------------------------------------------
void CNX_TMSServer::ThreadProc()
{
	int32_t iSize;
	uint8_t	payloadBuf[MAX_PAYLOAD_SIZE+6];

	while( m_bThreadRun )
	{
		int32_t clientSocket = WaitClient();
		if( !clientSocket )
		{
			continue;
		}
		else if( 0 > clientSocket )
		{
			break;
		}

		while( m_bThreadRun )
		{
			uint32_t iKey = 0;
			uint8_t *pPayload = NULL;
			uint16_t iPayloadSize = 0;

			do {
				uint8_t tempData;
				iSize = ReadData( clientSocket, &tempData, 1 );
				if( 1 > iSize )
				{
					NxErrMsg( "Fail, ReadData().\n" );
					break;
				}

				iKey = (iKey << 8) | tempData;
				if( iKey == TMS_KEY_VALUE )
				{
					break;
				}
			} while( m_bThreadRun );

			if( 1 > iSize )
			{
				break;
			}

			iSize = ReadData( clientSocket, payloadBuf, 2 );
			if( 2 > iSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				break;
			}

			iPayloadSize = TMS_GET_LENGTH( payloadBuf[0], payloadBuf[1] );
			if( iPayloadSize != 0 )
			{
				iSize = ReadData( clientSocket, payloadBuf + 2, iPayloadSize );
				if( 0 >= iSize || iSize != iPayloadSize )
				{
					NxErrMsg( "Fail, ReadData().\n" );
					break;
				}

				pPayload = payloadBuf + 2;
			}

			if( !ProcessCommand( clientSocket, iKey, pPayload, iPayloadSize ) )
				break;
		}

		if( 0 < clientSocket )
		{
			close( clientSocket );
			clientSocket = -1;
		}
	}
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::WaitClient()
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
				NxErrMsg( "Fail, accept().\n" );
				return -1;
			}

			printf( "Connect Success. ( %d )\n", clnSocket );
			return clnSocket;
		}
	}
	else if( 0 > ret )
	{
		NxErrMsg( "Fail, poll().\n");
		return -1;
	}

	NxDbgMsg( NX_DBG_VBS, "Timeout. WaitClient().\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	int32_t readSize, totalSize=0;
	do {
		int32_t ret;
		struct pollfd hPoll;
		
		hPoll.fd		= fd;
		hPoll.events	= POLLIN | POLLERR;
		hPoll.revents	= 0;
		ret = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );		
		if( 0 < ret ) 
		{
			readSize = read( fd, pBuf, iSize );

			if( 0 >= readSize )
			{
				return -1;
			}

			iSize     -= readSize;
			pBuf      += readSize;
			totalSize += readSize;
		}
		else if( 0 > ret )
		{
			NxErrMsg( "Fail, poll().\n" );
			return -1;
		}
		else
		{
			NxDbgMsg( NX_DBG_VBS, "Timeout. ReadData().\n");
			return 0;
		}
	} while(iSize > 0);

	return totalSize;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize )
{
	int32_t iSendSize = 0;
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];

	if( key != TMS_KEY_VALUE ) {
		return -1;
	}

	if( payloadSize != 4 ) {
		return -1;
	}

	uint8_t *pInBuf = (uint8_t*)pPayload;
	uint32_t iCmd	= TMS_GET_COMMAND( pInBuf[0], pInBuf[1] );
	uint8_t iResult = 0xFF;

	switch( iCmd )
	{
		case 0x0302:
		{
			iResult = 0x00;

			printf(">>>>> 0x%02x 0x%02x\n", pInBuf[2], pInBuf[3] );

			iSendSize = TMS_MakePacket( SEC_KEY_VALUE, iCmd, &iResult, sizeof(iResult), sendBuf, sizeof(sendBuf) );
			write( fd, sendBuf, iSendSize );
			break;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_TMSServer* CNX_TMSServer::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_TMSServer();
	}
	return (CNX_TMSServer*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_TMSServer::ReleaseInstance()
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
int32_t NX_TMSServerStart( int32_t iPort )
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	return hTms->StartServer( iPort );
}

//------------------------------------------------------------------------------
void NX_TMSServerStop()
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->StopServer();
}

//------------------------------------------------------------------------------
void NX_TMSRegisterCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->RegisterCallback( callback, pParam );
}