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
	int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size);

	int32_t IMB_ChangeContents( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize );

	int32_t ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize );

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t		m_ReceiveBuf[MAX_PAYLOAD_SIZE + 6];

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
	int32_t readSize;
	uint32_t key=0, cmd=0, len;
	uint8_t *pPayload;
	int32_t payloadSize;

	while( m_bThreadRun )
	{
		uint8_t *pBuf = m_ReceiveBuf;
		int32_t clientSocket = WaitClient();
		if( 0 > clientSocket )	//	Error
		{
			break;
		}

		//
		//	Find Key Code
		//
		do{
			uint8_t tmp;
			readSize = ReadData( clientSocket, &tmp, 1 );
			if( readSize != 1 )
			{
				NxErrMsg("Read Error!!!\n");
				printf("read Error!\n");
				goto ERROR;
			}
			key = (key<<8) | tmp;
			if( key == GDC_KEY(tmp) )
			{
				break;
			}
		}while(1);

		//	Read Length
		readSize = ReadData(clientSocket, pBuf, 2);
		if( readSize <= 0 ){
			NxErrMsg("Error : Read Length\n");
			goto ERROR;
		}
		len = MAKE_LENGTH_2BYTE(pBuf[0], pBuf[1]);

		//	Size Check
		if( len+2 > sizeof(m_ReceiveBuf) )
		{
			NxErrMsg("Error : Data size\n");
			goto ERROR;
		}

		//	Read all data
		readSize = ReadData ( clientSocket, pBuf+2, len );
		if( readSize <= 0 )
		{
			NxErrMsg("Error : Read Data\n");
			goto ERROR;
		}

		pPayload = pBuf + 2;
		payloadSize = len;

		ProcessCommand( clientSocket, key, pPayload, payloadSize );

ERROR:
		if( clientSocket > 0 )
			close( clientSocket );
	}
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::WaitClient()
{
	//	client socket
	int32_t clientSocket;
	struct sockaddr_un clntAddr;
	int32_t clntAddrSize;

	if( -1 == listen(m_hSocket, 5) )
	{
		NxErrMsg( "Error : listen (err = %d)\n", errno );
		return -1;
	}

	clntAddrSize = sizeof( clntAddr );
	clientSocket = accept( m_hSocket, (struct sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize );

	if ( -1 == clientSocket )
	{
		NxErrMsg( "Error : accept (err = %d)\n", errno );
		return -1;
	}
	return clientSocket;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ReadData(int32_t fd, uint8_t *pBuf, int32_t size)
{
	int32_t readSize, totalSize=0;
	do
	{
		readSize = read( fd, pBuf, size );
		if( readSize < 0 )
			return -1;

		size -= readSize;
		pBuf += readSize;
		totalSize += readSize;
	} while(size > 0);
	return totalSize;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::IMB_ChangeContents( int32_t fd, uint32_t iCmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	uint8_t result = 0xFF;
	int32_t sendSize;

	if( pBuf[0] == 0x00 || pBuf[0] == 0x01 )
	{
		if( !SendRemote( "cinema.change.contents", (pBuf[0] == 0x00) ? "0" : "1" ) )
		{
			result = 0x01;
		}
		else
		{
			printf("Fail, SendRemote. ( node: cinema.change.contents )\n");
		}
	}

ERROR_RESERVED:
	sendSize = GDC_MakePacket( SEC_KEY(iCmd), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize )
{
	int32_t iWriteSize = 0;
	int32_t iCmd = key & 0xFF;
	uint8_t sendBuf[MAX_PAYLOAD_SIZE + 6];

	switch( iCmd )
	{
		case GDC_COMMAND( CMD_TYPE_IMB, IMB_CMD_CHANGE_CONTENTS ):
			return IMB_ChangeContents( fd, iCmd, (uint8_t*)pPayload, payloadSize );
		default:
			return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t SendRemote( const char *pSockName, const char *pMsg )
{
	int32_t sock, len;
	struct sockaddr_un addr;

	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf("Fail, socket().\n");
		return -1;
	}

	addr.sun_family  = AF_UNIX;
	addr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( addr.sun_path + 1, pSockName );

	len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
	{
		printf("Fail, connect(). ( node: %s )\n", pSockName);
		close( sock );
		return -1;
	}

	if( 0 > write(sock, pMsg, strlen(pMsg)) )
	{
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
