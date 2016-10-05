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
#include <CNX_OpenSSL.h>
#include <gdc_protocol.h>
#include <SockUtils.h>

#include <NX_GDCServer.h>

#define NX_DTAG	"[GDC Server]"
#include <NX_DbgMsg.h>

#define MAX_PAYLOAD_SIZE		65535

//------------------------------------------------------------------------------
class CNX_GDCServer
	: protected CNX_Thread
{
public:
	CNX_GDCServer();
	~CNX_GDCServer();

	static CNX_GDCServer* GetInstance();
	static void ReleaseInstance();

	int32_t StartServer( int32_t iPort,  const char *pCertFile, const char *pPrivFile );
	void StopServer();

	void RegisterEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ) , void *pParam );

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	//	Local Socket
	int32_t WaitClient();
	int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size);
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );

	int32_t ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize );

private:
	bool		m_bThreadRun;
	pthread_t	m_hThread;

	int32_t		m_hSocket;
	uint8_t		m_PayloadBuf[MAX_PAYLOAD_SIZE+6];
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE+6];

	CNX_OpenSSL *m_pServerSSL;

	int32_t		(*m_cbEventCallback)( void *pObj, int32_t iEventCode , void *pData, int32_t iSize );
	void		*m_hEventParam;

private:
	//	For Singletone
	static CNX_GDCServer	*m_psInstance;
};

//------------------------------------------------------------------------------
CNX_GDCServer* CNX_GDCServer::m_psInstance = NULL;

CNX_GDCServer::CNX_GDCServer()
	: m_bThreadRun( false )
	, m_hThread( 0x00 )
	, m_hSocket (-1)
	, m_pServerSSL( NULL )
	, m_cbEventCallback( NULL )
	, m_hEventParam( NULL )
{
	memset( m_PayloadBuf, 0x00, sizeof(m_PayloadBuf) );
	memset( m_SendBuf, 0x00, sizeof(m_SendBuf) );
}

//------------------------------------------------------------------------------
CNX_GDCServer::~CNX_GDCServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
int32_t CNX_GDCServer::StartServer( int32_t iPort,  const char *pCertFile, const char *pPrivFile )
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

	m_pServerSSL = new CNX_OpenSSL();
	if( NULL == m_pServerSSL )
	{
		NxErrMsg( "Fail, Create CNX_OpenSSL().\n" );
		goto ErrorExit;
	}

	if( 0 > m_pServerSSL->ReadCertificate( pCertFile ) ||
		0 > m_pServerSSL->ReadPrivateKey( pPrivFile ) )
	{
		NxErrMsg( "Fail, ReadCertificate() or ReadPrivateKey().\n" );
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

	if( NULL != m_pServerSSL )
	{
		delete m_pServerSSL;
		m_pServerSSL = NULL;
	}

	if( 0 < m_hSocket )
	{
		close( m_hSocket );
		m_hSocket = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
void CNX_GDCServer::StopServer()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		Stop();
	}
}

//------------------------------------------------------------------------------
void CNX_GDCServer::RegisterEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	m_cbEventCallback = callback;
	m_hEventParam = pParam;
}

//------------------------------------------------------------------------------
void CNX_GDCServer::ThreadProc()
{
	int32_t iSize;

	while( m_bThreadRun )
	{
		uint8_t *pBuf = m_PayloadBuf;
		int32_t clientSocket = WaitClient();
		if( 0 > clientSocket )	//	Error
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
				if( iSize != 1 )
				{
					NxErrMsg("Read Error!!!\n");
					break;
				}

				iKey = (iKey << 8) | tempData;
				if( iKey == GDC_KEY(tempData) )
				{
					break;
				}
			} while( m_bThreadRun );

			iSize = ReadData( clientSocket, pBuf, 2 );
			if( 0 > iSize )
			{
				NxErrMsg("Read Error!!!\n");
				break;
			}

			iPayloadSize = MAKE_LENGTH_2BYTE( pBuf[0], pBuf[1] );
			if( iPayloadSize != 0 )
			{
				iSize = ReadData( clientSocket, pBuf + 2, iPayloadSize );
				if( 0 >= iSize || iSize != iPayloadSize )
				{
					NxErrMsg("Read Error!!!\n");
					break;
				}

				pPayload = pBuf + 2;
			}

			// printf("-->> Receive Data( 0x%08x, %d )\n", iKey, iPayloadSize );

			if( 0 != ProcessCommand( clientSocket, iKey, pPayload, iPayloadSize ) )
				break;
		}

		if( clientSocket > 0 )
			close( clientSocket );
	}

	if( m_pServerSSL ) delete m_pServerSSL;
}

//------------------------------------------------------------------------------
int32_t CNX_GDCServer::WaitClient()
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

	// int32_t hPoll;
	// struct pollfd	pollEvent;
	//
	// do{
	// 	//	Wait Event form UART
	// 	pollEvent.fd		= m_hSocket;
	// 	pollEvent.events	= POLLIN | POLLERR;
	// 	pollEvent.revents	= 0;
	// 	hPoll = poll( (struct pollfd*)&pollEvent, 1, 3000 );

	// 	if( hPoll < 0 ) {
	// 		return -1;
	// 	}
	// 	else if( hPoll > 0 ) {
	// 		break;
	// 	}
	// }while( m_ExitLoop );

	clntAddrSize = sizeof( clntAddr );
	clientSocket = accept( m_hSocket, (struct sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize );

	if ( -1 == clientSocket )
	{
		NxErrMsg( "Error : accept (err = %d)\n", errno );
		return -1;
	}
	else
	{
		printf("Connect Success.\n");
	}
	
	return clientSocket;
}

//------------------------------------------------------------------------------
int32_t CNX_GDCServer::ReadData(int32_t fd, uint8_t *pBuf, int32_t size)
{
	int32_t readSize, totalSize=0;
	do {
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
int32_t CNX_GDCServer::WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_GDCServer::ProcessCommand( int32_t fd, uint32_t key, void *pPayload, int32_t payloadSize )
{
	int32_t iWriteSize = 0;
	switch( key )
	{
		case GDC_KEY(1):
		{
			if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, EVENT_RECEIVE_CERTIFICATE, pPayload, payloadSize );

			// CNX_OpenSSL *pClientSSL = new CNX_OpenSSL();
			// pClientSSL->ReadCertificate( (uint8_t*)pPayload, payloadSize );
			// delete pClientSSL;

			uint8_t *pCertBuf = NULL;
			int32_t iCertSize = m_pServerSSL->GetCertificate( &pCertBuf );

			iWriteSize = GDC_MakePacket( SEC_KEY(1), (void*)pCertBuf, iCertSize, m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > iWriteSize )
			{
				if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, ERROR_MAKE_PACKET, NULL,  0 );
				return -1;
			}

			WriteData( fd, (uint8_t*)m_SendBuf, iWriteSize );
			if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, EVENT_ACK_CERTIFICATE, m_SendBuf, iWriteSize );

			return 0;
		}
		break;

		case GDC_KEY(2):
		{
			if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, EVENT_RECEIVE_PLANE_DATA, pPayload, payloadSize );

			uint8_t *pSignData = (uint8_t*)malloc(256);
			int32_t iSignSize;
			if( 0 > m_pServerSSL->Sign( (uint8_t*)pPayload, payloadSize, &pSignData, &iSignSize) )
			{
				if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, ERROR_SIGN_PLANE_TEXT, NULL,  0 );

				printf("Fail, Sign().\n");
				free( pSignData );
				
				return -1;
			}

			iWriteSize = GDC_MakePacket( SEC_KEY(2), (void*)pSignData, iSignSize, m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > iWriteSize )
			{
				if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, ERROR_MAKE_PACKET, NULL,  0 );
				return -1;
			}

			WriteData( fd, (uint8_t*)m_SendBuf, iWriteSize );
			free( pSignData );
			if( m_cbEventCallback ) m_cbEventCallback( m_hEventParam, EVENT_ACK_SIGN_PLANE_TEXT, m_SendBuf, iWriteSize );

			return 0;
		}
		break;

		case GDC_KEY(3):
		{
			if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, EVENT_RECEIVE_MARRIAGE_OK, pPayload, payloadSize );

			iWriteSize = GDC_MakePacket( SEC_KEY(3), NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > iWriteSize )
			{
				if( m_cbEventCallback )	m_cbEventCallback( m_hEventParam, ERROR_MAKE_PACKET, NULL,  0 );
				return -1;
			}

			WriteData( fd, (uint8_t*)m_SendBuf, iWriteSize );
			if( m_cbEventCallback ) m_cbEventCallback( m_hEventParam, EVENT_ACK_MARRIAGE_OK, m_SendBuf, iWriteSize );

			return 1;
		}
		break;
	}

	return -1;
}


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_GDCServer* CNX_GDCServer::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_GDCServer();
	}
	return (CNX_GDCServer*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_GDCServer::ReleaseInstance()
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
int32_t NX_MarriageServerStart( int32_t iPort,  const char *pCertFile, const char *pPrivFile )
{
	CNX_GDCServer *hGdc = CNX_GDCServer::GetInstance();
	return hGdc->StartServer( iPort, pCertFile, pPrivFile );
}

//------------------------------------------------------------------------------
void NX_MarriageServerStop()
{
	CNX_GDCServer *hGdc = CNX_GDCServer::GetInstance();
	hGdc->StopServer();
}

//------------------------------------------------------------------------------
void NX_MarraigeEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	CNX_GDCServer *hGdc = CNX_GDCServer::GetInstance();
	hGdc->RegisterEventCallback( callback, pParam );
}