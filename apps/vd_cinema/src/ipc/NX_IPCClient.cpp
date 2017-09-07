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

#include <ipc_protocol.h>
#include <SockUtils.h>

#include <NX_IPCServer.h>
#include <NX_IPCClient.h>
#include <NX_IPCCommand.h>

#include <NX_Version.h>

#define NX_DTAG	"[IPC Client]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
//
//	IPC Client APIs
//
class CNX_IPCClient
{
public:
	CNX_IPCClient(){}
	virtual ~CNX_IPCClient(){}

	static CNX_IPCClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t SendCommand( uint32_t iCmd, uint8_t *pBuf, int32_t *iSize );

private:
	int32_t Send( uint32_t iCmd, uint8_t *pBuf, int32_t *iSize );
	int32_t GetVersion( uint32_t iCmd, uint8_t *pBuf, int32_t *iSize );

private:
	//	Rx/Tx Buffer
	//	Key(4bytes) + Length(2bytes) + Command(2bytes) + Payload(MAX 65533bytes )
	uint8_t m_SendBuf[MAX_PAYLOAD_SIZE + 8];
	uint8_t m_ReceiveBuf[MAX_PAYLOAD_SIZE + 8];

private:
	//	For Singleton
	static CNX_IPCClient	*m_psInstance;

private:
	CNX_IPCClient (const CNX_IPCClient &Ref);
	CNX_IPCClient &operator=(const CNX_IPCClient &Ref);
};

CNX_IPCClient* CNX_IPCClient::m_psInstance = NULL;


//------------------------------------------------------------------------------
//
//	IPC Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::SendCommand( uint32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	if( (iCmd == PLATFORM_CMD_IPC_CLIENT_VERSION) ||
		(iCmd == GDC_COMMAND(CMD_TYPE_PLATFORM, PLATFORM_CMD_IPC_CLIENT_VERSION)) )
	{
		return GetVersion( iCmd, pBuf, iSize );
	}

	return Send( iCmd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::Send( uint32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect( IPC_SERVER_FILE );
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE );
		return -1;
	}

	sendSize = IPC_MakePacket( NXP_KEY_VALUE, iCmd, pBuf, *iSize, m_SendBuf, sizeof(m_SendBuf) );
	if( 0 > sendSize )
	{
		NxErrMsg( "Error: IPC_MakePacket().\n" );
		close( clntSock );
		return -1;
	}
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != IPC_ParsePacket( m_ReceiveBuf, recvSize, &key, &iCmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : IPC_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	//
	//	Condition of pBuf
	//	 1. The pBuf must not be NULL.
	//	 2. The pBuf's size must be same payload's size. ( 65533 bytes )
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
int32_t CNX_IPCClient::GetVersion( uint32_t /*iCmd*/, uint8_t *pBuf, int32_t *iSize )
{
	uint8_t version[1024];
	snprintf( (char*)version, sizeof(version), "%s ( %s, %s )", NX_VERSION_IPC_CLIENT, __TIME__, __DATE__ );

	int32_t payloadSize = (int32_t)strlen((const char*)version);

	memcpy( pBuf, version, payloadSize );
	*iSize = payloadSize;

	return 0;
}


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_IPCClient* CNX_IPCClient::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_IPCClient();
	}
	return (CNX_IPCClient*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_IPCClient::ReleaseInstance()
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
int32_t NX_IPCSendCommand( int32_t iCmd, uint8_t *pBuf, int32_t *iSize )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->SendCommand( iCmd, pBuf, iSize );
}
