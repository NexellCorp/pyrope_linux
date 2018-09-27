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
#include <NX_Version.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			10000
#define MAX_PAYLOAD_SIZE	65535

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

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
	void	RegisterProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj );

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	int32_t WaitClient();

	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t SendRemote( const char *pSockName, const char *pMsg );

	// int32_t ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize );
	// int32_t IMB_QueCommand( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize );

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t		m_RecvBuf[MAX_PAYLOAD_SIZE + 6];

	void 		(*m_cbProcessCallback)( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize, void *pObj );
	void		*m_pObj;

	CNX_CinemaManager* m_pCinema;

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
CNX_TMSServer::CNX_TMSServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
	, m_cbProcessCallback( NULL )
	, m_pObj( NULL )
{
	char szVersion[1024] = { 0x00, };
	snprintf( (char*)szVersion, sizeof(szVersion), "%s ( %08lld-%06lld )", NX_VERSION_TMS_SERVER, NX_DATE(), NX_TIME() );

	m_pCinema = CNX_CinemaManager::GetInstance();
	m_pCinema->SetTmsVersion( (uint8_t*)szVersion, strlen(szVersion) );
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
		NxDbgMsg( NX_DBG_ERR, "Fail, TCP_Open().\n" );
		return -1;
	}

	if( 0 > listen(m_hSocket, 5) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail : listen (err = %d)\n", errno );
		goto ERROR_TMS;
	}

	m_bThreadRun = true;

	if( 0 != Start() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Start().\n" );
		goto ERROR_TMS;
	}

	return 0;

ERROR_TMS:
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
void CNX_TMSServer::RegisterProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	m_cbProcessCallback = cbCallback;
	m_pObj = pObj;
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
			goto ERROR_TMS;
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
			goto ERROR_TMS;
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
			goto ERROR_TMS;
		}

		iPayloadSize = MAKE_LENGTH( pRecvBuf[0], pRecvBuf[1] );
		pRecvBuf += 2;

		//	Size Check
		if( iPayloadSize+8 > (int32_t)sizeof(m_RecvBuf) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Data Size.\n" );
			goto ERROR_TMS;
		}

		//	Read all data
		if( iPayloadSize != 0 )
		{
			iReadSize = ReadData( iClientSocket, pRecvBuf, iPayloadSize );
			if( iReadSize != iPayloadSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				goto ERROR_TMS;
			}
		}

		pPayload = pRecvBuf;

		NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
		NxDbgMsg( NX_DBG_VBS, "iCmd = 0x%08x, readSize = %d\n", iCmd, iReadSize );

		NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand. ( curTime: %lld mSec )\n",  NX_GetTickCount() );

		if( !m_pCinema->IsBusy() )
		{
			uint8_t sendData[4] = { 0x00, };
			sprintf( (char*)sendData, "%d", pPayload[0] );

			if( !SendRemote( "cinema.tms", (const char*)sendData ) )
			{
				outBuf[0] = 0x01;
				iOutSize = 1;
			}
			else
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
				printf("Fail, SendRemote. ( node: cinema.tms )\n");
				outBuf[0] = 0xFF;
				iOutSize = 1;
			}
		}
		else
		{
			outBuf[0] = 0xFE;
			iOutSize = 1;
		}

		iSendSize = GDC_MakePacket( KEY_SEC(iCmd), outBuf, iOutSize, m_SendBuf, sizeof(m_SendBuf) );
		WriteData( iClientSocket, m_SendBuf, iSendSize );

		NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );

ERROR_TMS:
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
				NxDbgMsg( NX_DBG_ERR, "Fail, accept().\n" );
				return -1;
			}

			// printf( "Connect Success. ( %d )\n", clnSocket );
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
int32_t CNX_TMSServer::SendRemote( const char *pSockName, const char *pMsg )
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

// //------------------------------------------------------------------------------
// int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize )
// {
// 	int32_t iCmd = key & 0xFF;

// 	switch( iCmd )
// 	{
// 		case GDC_COMMAND( CMD_TYPE_IMB, IMB_CMD_QUE ):
// 			return IMB_QueCommand( fd, iCmd, (uint8_t*)pPayload, payloadSize );
// 		default:
// 			return -1;
// 	}

// 	return 0;
// }

// //------------------------------------------------------------------------------
// int32_t CNX_TMSServer::IMB_QueCommand( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize )
// {
// 	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
// 	UNUSED( iSize );

// 	uint8_t result = 0xFF;
// 	int32_t sendSize;

// 	uint8_t sendData[4] = { 0x00, };
// 	sprintf( (char*)sendData, "%d", pBuf[0] );

// 	if( !SendRemote( "cinema.tms", (const char*)sendData ) )
// 	{
// 		result = 0x01;
// 	}
// 	else
// 	{
// 		NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
// 		printf("Fail, SendRemote. ( node: cinema.tms )\n");
// 	}

// 	sendSize = GDC_MakePacket( KEY_SEC(iCmd), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

// 	WriteData( fd, m_SendBuf, sendSize );
// 	NX_HexDump( m_SendBuf, sendSize );

// 	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
// 	return 0;
// }


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

//------------------------------------------------------------------------------
void NX_TMSServerProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->RegisterProcessCallback( cbCallback, pObj );
}
