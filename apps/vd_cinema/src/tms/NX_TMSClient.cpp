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
#include <NX_Utils.h>

#define NX_DTAG	"[TMS Client]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			10000
#define MAX_PAYLOAD_SIZE	65535

class CNX_TMSClient
{
public:
	CNX_TMSClient(){}
	~CNX_TMSClient(){}

	static CNX_TMSClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t SetSource( const char *pIpAddr, int32_t iPort, uint8_t value1, uint8_t value2 );

private:
	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );

private:
	static CNX_TMSClient *m_psInstance;
};

//------------------------------------------------------------------------------
CNX_TMSClient* CNX_TMSClient::m_psInstance = NULL;


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_TMSClient* CNX_TMSClient::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_TMSClient();
	}
	return (CNX_TMSClient*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_TMSClient::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize )
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
			// printf( "Timeout. ReadData().\n" );
			return 0;
		}
	} while(iSize > 0);

	return totalSize;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::SetSource( const char *pIpAddr, int32_t iPort, uint8_t value1, uint8_t value2 )
{
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	uint32_t cmd = 0x0302;
	uint8_t sendData[2] = { value1, value2 };
	uint8_t recvData[16];
	
	clntSock = TCP_Connect( pIpAddr, iPort );
	if( 0 > clntSock )
	{
		printf("Fail, TCP_Connect().\n");
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &sendData, sizeof(sendData), recvData, sizeof(recvData) );
	WriteData( clntSock, recvData, sendSize );

	recvSize = read( clntSock, recvData, sizeof(recvData) );
	if( 0 != TMS_ParsePacket( recvData, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		printf("Error, TMS_ParsePacket().\n");
		close( clntSock );
		return -1;
	}

	NX_HexDump( payload, payloadSize );

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
//
//	External APIs
//

//------------------------------------------------------------------------------
int32_t NX_TMSSetSource( const char *pIpAddr, int32_t iPort, uint8_t value1, uint8_t value2 )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SetSource( pIpAddr, iPort, value1, value2 );
}
