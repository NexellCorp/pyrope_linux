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

#include <NX_IPCCommand.h>
#include <NX_TMSServer.h>
#include <CNX_BaseClass.h>
#include <NX_Utils.h>

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

	int32_t ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize );
	int32_t IMB_QueCommand( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize );

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 6];

	void 		(*m_cbProcessCallback)( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize, void *pObj );
	void		*m_pObj;

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

//------------------------------------------------------------------------------
CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

CNX_TMSServer::CNX_TMSServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
	, m_cbProcessCallback( NULL )
	, m_pObj( NULL )
{
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
		goto ErrorExit;
	}

	m_bThreadRun = true;

	if( 0 != Start() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Start().\n" );
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
void CNX_TMSServer::RegisterProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	m_cbProcessCallback = cbCallback;
	m_pObj = pObj;
}

//------------------------------------------------------------------------------
void CNX_TMSServer::ThreadProc()
{
	static uint8_t payloadBuf[MAX_PAYLOAD_SIZE+6];

	while( m_bThreadRun )
	{
		int32_t iReadSize = 0;
		uint32_t iKey = 0;
		uint8_t *pPayload = payloadBuf;
		uint16_t iPayloadSize = 0;

		int32_t clientSocket = WaitClient();
		if( !clientSocket )
		{
			continue;
		}
		else if( 0 > clientSocket )	//	Error
		{
			goto ERROR;
		}

		do {
			uint8_t tempData;
			iReadSize = ReadData( clientSocket, &tempData, 1 );
			if( 1 != iReadSize && 0 > iReadSize )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, ReadData(). ( line: %d, iReadSize: %d )\n", __LINE__, iReadSize );
				if( 0 < iReadSize ) NX_HexDump( &tempData, iReadSize );
				goto ERROR;
			}

			iKey = (iKey << 8) | tempData;
			*(pPayload++) = tempData;
			if( iKey == GDC_KEY(tempData) )
			{
				break;
			}
		} while( m_bThreadRun );

		iReadSize = ReadData( clientSocket, pPayload, 2 );
		if( 2 != iReadSize && 0 > iReadSize )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, ReadData(). ( line: %d, iReadSize: %d )\n", __LINE__, iReadSize );
			if( 0 < iReadSize ) NX_HexDump( pPayload, iReadSize );
			goto ERROR;
		}

		iPayloadSize = MAKE_LENGTH_2BYTE( pPayload[0], pPayload[1] );
		if( iPayloadSize != 0 )
		{
			iReadSize = ReadData( clientSocket, pPayload + 2, iPayloadSize );
			if( 0 >= iReadSize || iReadSize != iPayloadSize )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, ReadData(). ( line: %d, iReadSize: %d, expected: %d )\n",
					__LINE__, iReadSize, iPayloadSize );
				if( 0 < iReadSize ) NX_HexDump( pPayload+2, iReadSize );
				goto ERROR;
			}

			pPayload += 2;
		}

		if( NULL == m_cbProcessCallback )
		{
			ProcessCommand( clientSocket, iKey, pPayload, iPayloadSize );
		}
		else
		{
			uint8_t outBuf[256] = { 0x00, };
			int32_t iOutSize = sizeof(outBuf);
			int32_t iSendSize = 0;

			NX_HexDump( payloadBuf, 4 + 2 + iPayloadSize, "Recv: " );
			printf("\n");

			m_cbProcessCallback( iKey, pPayload, iPayloadSize, outBuf, &iOutSize, m_pObj );
			iSendSize = GDC_MakePacket( SEC_KEY(iKey), outBuf, iOutSize, m_SendBuf, sizeof(m_SendBuf) );
			WriteData( clientSocket, m_SendBuf, iSendSize );

			NX_HexDump( m_SendBuf, iSendSize, "Send: " );
			printf("\n");
		}

ERROR:
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
				NxDbgMsg( NX_DBG_ERR, "Fail, accept().\n" );
				return -1;
			}

			NxDbgMsg( NX_DBG_DEBUG, "Connect Success. ( %d )\n", clnSocket );
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
			NxDbgMsg( NX_DBG_ERR, "Fail, poll().\n" );
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
		NxDbgMsg( NX_DBG_ERR, "Fail, connect(). ( node: %s )\n", pSockName);
		printf("Fail, connect(). ( node: %s )\n", pSockName);
		close( sock );
		return -1;
	}

	if( 0 > WriteData(sock, (uint8_t*)pMsg, strlen(pMsg)) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, write().\n");
		printf("Fail, write().\n");
		close( sock );
		return -1;
	}

	close( sock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize )
{
	int32_t iCmd = key & 0xFF;

	switch( iCmd )
	{
		case GDC_COMMAND( CMD_TYPE_IMB, IMB_CMD_QUE ):
			return IMB_QueCommand( fd, iCmd, (uint8_t*)pPayload, payloadSize );
		default:
			return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::IMB_QueCommand( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize )
{
	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
	UNUSED( iSize );

	uint8_t result = 0xFF;
	int32_t sendSize;

	uint8_t sendData[4] = { 0x00, };
	sprintf( (char*)sendData, "%d", pBuf[0] );

	if( !SendRemote( "cinema.tms", (const char*)sendData ) )
	{
		result = 0x01;
	}
	else
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
		printf("Fail, SendRemote. ( node: cinema.tms )\n");
	}

	sendSize = GDC_MakePacket( SEC_KEY(iCmd), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	WriteData( fd, m_SendBuf, sendSize );
	NX_HexDump( m_SendBuf, sendSize );

	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
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

//------------------------------------------------------------------------------
void NX_TMSServerProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->RegisterProcessCallback( cbCallback, pObj );
}
