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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gdc_protocol.h>
#include <SockUtils.h>

#include <NX_TMSServer.h>
#include <NX_TMSClient.h>
#include <NX_Utils.h>

#define NX_DTAG	"[TMS Client]"
#include <NX_DbgMsg.h>

#define MAX_PAYLOAD_SIZE	65535

class CNX_TMSClient
{
public:
	CNX_TMSClient(){}
	~CNX_TMSClient(){}

	static CNX_TMSClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t SendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t *pBuf, int32_t *iSize );

private:
	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );

private:
	//	Rx/Tx Buffer
	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes )
	uint8_t m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t m_ReceiveBuf[MAX_PAYLOAD_SIZE + 6];

private:
	static CNX_TMSClient *m_psInstance;
};

CNX_TMSClient* CNX_TMSClient::m_psInstance = NULL;

//------------------------------------------------------------------------------
//
//	TMS Control APIs
//

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
		NxErrMsg( "Error : TCP_Connect (%s:%d)\n", pIpAddr, TMS_SERVER_PORT );
		return -1;
	}

	sendSize = GDC_MakePacket( KEY_GDC(iCmd), pBuf, *iSize, m_SendBuf, sizeof(m_SendBuf) );
	if( 0 > sendSize )
	{
		NxErrMsg( "Error: GDC_MakePacket().\n" );
		close( clntSock );
		return -1;
	}

	NX_HexDump( m_SendBuf, sendSize, "Send: " );
	printf("\n");

	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != GDC_ParsePacket( m_ReceiveBuf, recvSize, &key, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : GDC_ParsePacket().\n");
		close( clntSock );
		return -1;
	}

	NX_HexDump( m_ReceiveBuf, recvSize, "Recv: " );
	printf("\n");

	//
	//	Condition of pBuf
	//	 1. The pBuf must not be NULL.
	//	 2. The pBuf's size must be same payload's size. ( 65535 bytes )
	//   3. The iSize is not payload's size.
	//	    The iSize is real data size in pBuf.
	//
	if( NULL == pBuf )
	{
		NxErrMsg( "Error : Buffer is NULL.\n" );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*iSize = payloadSize;

	close( clntSock );
	return 0;
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
int32_t NX_TMSSendCommand( const char *pIpAddr, int32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SendCommand( pIpAddr, iCmd, pBuf, iSize );
}
