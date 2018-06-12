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
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <SockUtils.h>
#include <gdc_protocol.h>

#include <NX_CinemaCommand.h>
#include <NX_TMSServer.h>
#include <CNX_Base.h>
#include <CNX_CinemaManager.h>

#include <NX_Utils.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			10000
#define MAX_PAYLOAD_SIZE	65535

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

static int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
static int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );
static int32_t SendRemote( const char *pSockName, const char *pMsg );

//------------------------------------------------------------------------------
class CNX_TMSServer
	: protected CNX_Thread
{
public:
	CNX_TMSServer();
	~CNX_TMSServer();

	static CNX_TMSServer* GetInstance();
	static void ReleaseInstance();

	int32_t	StartServer();
	void	StopServer();

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	int32_t WaitClient();

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t		m_RecvBuf[MAX_PAYLOAD_SIZE + 6];

	CNX_CinemaManager* m_pCinema;

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

//------------------------------------------------------------------------------
CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

CNX_TMSServer::CNX_TMSServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
{
	m_pCinema = CNX_CinemaManager::GetInstance();
}

//------------------------------------------------------------------------------
CNX_TMSServer::~CNX_TMSServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::StartServer()
{
	if( true == m_bThreadRun )
	{
		return 1;
	}

	m_hSocket = TCP_Open( TMS_SERVER_PORT );
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
	int32_t iReadSize;
	uint32_t iKey = 0, iCmd = 0;
	uint8_t *pPayload;
	uint16_t iPayloadSize;

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
			goto ERROR;
		}

		do {
			uint8_t temp;
			iReadSize = ReadData( iClientSocket, &temp, 1 );
			if( iReadSize != 1 )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				break;
			}

			iKey = (iKey << 8) | temp;
			if( iKey == KEY_GDC(temp) )
			{
				break;
			}
		} while( m_bThreadRun );

		if( 1 > iReadSize )
		{
			goto ERROR;
		}

		//	Key
		*pRecvBuf++ = (( iKey >> 24) & 0xFF);
		*pRecvBuf++ = (( iKey >> 16) & 0xFF);
		*pRecvBuf++ = (( iKey >>  8) & 0xFF);
		*pRecvBuf++ = (( iKey      ) & 0xFF);

		iCmd = iKey & 0xFF;

		//	Read Length
		iReadSize = ReadData( iClientSocket, pRecvBuf, 2 );
		if( 2 != iReadSize )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, ReadData().\n" );
			goto ERROR;
		}

		iPayloadSize = MAKE_LENGTH( pRecvBuf[0], pRecvBuf[1] );
		pRecvBuf += 2;

		//	Size Check
		if( iPayloadSize+8 > (int32_t)sizeof(m_RecvBuf) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Data Size.\n" );
			goto ERROR;
		}

		//	Read all data
		if( iPayloadSize != 0 )
		{
			iReadSize = ReadData( iClientSocket, pRecvBuf, iPayloadSize );
			if( iReadSize != iPayloadSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				goto ERROR;
			}
		}

		pPayload = pRecvBuf;

		NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
		NxDbgMsg( NX_DBG_VBS, "iCmd = 0x%08x, readSize = %d\n", iCmd, iReadSize );

		if( !m_pCinema->IsBusy() )
		{
			SendRemote( "cinema.tms", "run" );
			m_pCinema->SendCommand( iCmd, pPayload, iPayloadSize, outBuf, &iOutSize );
			SendRemote( "cinema.tms", "stop" );
		}
		else
		{
			outBuf[0] = 0xFF;
			outBuf[1] = 0xFF;
			outBuf[2] = 0xFF;
			outBuf[3] = 0xFE;
			iOutSize = 4;
		}

		iSendSize = GDC_MakePacket( KEY_SEC(iCmd), outBuf, iOutSize, m_SendBuf, sizeof(m_SendBuf) );
		WriteData( iClientSocket, m_SendBuf, iSendSize );

ERROR:
		if( 0 < iClientSocket )
		{
			close( iClientSocket );
			iClientSocket = -1;
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

			// printf( "Connect Success. ( %d )\n", clnSocket );
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
static int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize )
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
static int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
static int32_t SendRemote( const char *pSockName, const char *pMsg )
{
	int32_t sock, len;
	struct sockaddr_un addr;

	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, socket().\n");
		printf("Fail, socket().\n");
		return -1;
	}

	addr.sun_family  = AF_UNIX;
	addr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( addr.sun_path + 1, pSockName );

	len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, connect(). ( node: %s )\n", pSockName);
		printf("Fail, connect(). ( node: %s )\n", pSockName);
		close( sock );
		return -1;
	}

	if( 0 > WriteData(sock, (uint8_t*)pMsg, strlen(pMsg)) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, write().\n");
		printf("Fail, write().\n");
		close( sock );
		return -1;
	}

	close( sock );
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
int32_t NX_TMSServerStart()
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	return hTms->StartServer();
}

//------------------------------------------------------------------------------
void NX_TMSServerStop()
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->StopServer();
}
