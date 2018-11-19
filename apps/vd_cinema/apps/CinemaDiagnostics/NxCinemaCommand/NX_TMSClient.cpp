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

#include "stdafx.h"

#include <io.h>
#include <stdio.h>
#include <string.h>

#include "gdc_protocol.h"
#include "NX_TMSClient.h"

//------------------------------------------------------------------------------
#define MAKE_KEY(C0, C1, C2, C3)	\
		( ((uint32_t)(uint8_t)(C3)      ) |	\
		  ((uint32_t)(uint8_t)(C2) << 8 ) |	\
		  ((uint32_t)(uint8_t)(C1) << 16) |	\
		  ((uint32_t)(uint8_t)(C0) << 24) )

#define	KEY_GDC(N)			MAKE_KEY('G','D','C',N)

#define TMS_SERVER_PORT				7434

//------------------------------------------------------------------------------
class CNX_TMSClient
{
public:
	CNX_TMSClient(){}
	~CNX_TMSClient(){}

	static CNX_TMSClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t SendPacket( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t RecvPacket( int32_t fd, uint8_t *pBuf, int32_t iSize );

	int32_t SendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t *pBuf, int32_t *iSize );
	int32_t SendCommand( const char *pIpAddr, void *pObj, int32_t (*cbProcess)(int32_t, void*) );

private:
	//	Rx/Tx Buffer
	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes )
	uint8_t m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t m_RecvBuf[MAX_PAYLOAD_SIZE + 6];

private:
	static CNX_TMSClient *m_psInstance;
};

CNX_TMSClient* CNX_TMSClient::m_psInstance = NULL;

//------------------------------------------------------------------------------
int32_t TCP_Connect( const char *ipAddr, short port )
{
	WSADATA wsaData;
	int32_t clntSock;
	struct sockaddr_in svrAddr;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
        TRACE("Load WinSock 2.2 DLL Error");
	}

	clntSock  = socket( AF_INET, SOCK_STREAM, 0);
	if( -1 == clntSock)
	{
		return -1;
	}

	memset( &svrAddr, 0, sizeof( svrAddr) );
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = inet_addr(ipAddr);
	svrAddr.sin_port = htons( port );

	if( SOCKET_ERROR == connect( clntSock, (struct sockaddr*)&svrAddr, sizeof(svrAddr) ) )
	{
		TRACE( "Fail, connect(). ( errno: %d )\n", errno);
		closesocket( clntSock );
		WSACleanup();
		return -1;
	}
	else
	{
		return clntSock;
	}
}

//------------------------------------------------------------------------------
void TCP_Close( int32_t sock )
{
	if( 0 <= sock )
	{
		closesocket(sock);
		WSACleanup();
	}
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::SendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	int32_t clntSock;
	int32_t sendSize, recvSize;
	int16_t payloadSize;
	uint32_t key;
	void *payload;

	clntSock = TCP_Connect( pIpAddr, TMS_SERVER_PORT );
	if( -1 == clntSock)
	{
		TRACE( "Error : TCP_Connect (%s:%d)\n", pIpAddr, TMS_SERVER_PORT );
		return -1;
	}

	sendSize = GDC_MakePacket( KEY_GDC(iCmd), pBuf, *iSize, m_SendBuf, sizeof(m_SendBuf) );
	if( 0 > sendSize )
	{
		TRACE( "Error: GDC_MakePacket().\n" );
		TCP_Close( clntSock );
		return -1;
	}

	NX_TMSSendPacket( clntSock, m_SendBuf, sendSize );

	recvSize = NX_TMSRecvPacket( clntSock, m_RecvBuf, sizeof(m_RecvBuf) );
	if( 0 != GDC_ParsePacket( m_RecvBuf, recvSize, &key, &payload, &payloadSize ) )
	{
		TRACE( "Error : GDC_ParsePacket().\n");
		TCP_Close( clntSock );
		return -1;
	}

	//
	//	Condition of pBuf
	//	 1. The pBuf must not be NULL.
	//	 2. The pBuf's size must be same payload's size. ( 65535 bytes )
	//	 3. The iSize is not payload's size.
	//	    The iSize is real data size in pBuf.
	//
	if( NULL == pBuf )
	{
		TRACE( "Error : Buffer is NULL.\n" );
		TCP_Close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*iSize = payloadSize;

	TCP_Close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::SendCommand( const char *pIpAddr, void *pObj, int32_t (*cbProcess)(int32_t, void *) )
{
	int32_t clntSock = TCP_Connect( pIpAddr, TMS_SERVER_PORT );
	if( -1 == clntSock)
	{
		TRACE( "Error : TCP_Connect (%s:%d)\n", pIpAddr, TMS_SERVER_PORT );
		return -1;
	}

	int32_t iRet = 0;
	if( cbProcess )
		iRet = cbProcess( clntSock, pObj );

	TCP_Close( clntSock );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::SendPacket( int32_t fd, uint8_t* pBuf, int32_t iSize )
{
	return send( fd, (char*)pBuf, iSize, NULL );
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::RecvPacket( int32_t fd, uint8_t* pBuf, int32_t iSize )
{
	int32_t iRecvSize = 0;
	do {
		int32_t iReadSize = recv( fd, (char*)(pBuf + iRecvSize), iSize - iRecvSize, NULL );
		iRecvSize += iReadSize;
	} while( iRecvSize < (((pBuf[4] << 8) | pBuf[5]) + 6) );

	return iRecvSize;
}


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_TMSClient* CNX_TMSClient::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_TMSClient();
	}
	return (CNX_TMSClient*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_TMSClient::ReleaseInstance()
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
int32_t NX_TMSSendPacket( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SendPacket( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t NX_TMSRecvPacket( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->RecvPacket( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t NX_TMSSendCommand( const char *pIpAddr, int32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SendCommand( pIpAddr, iCmd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t NX_TMSSendCommand( const char *pIpAddr, void *pObj, int32_t (*cbProcess)(int32_t iSock, void *pObj) )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SendCommand( pIpAddr, pObj, cbProcess );
}

