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
#include <poll.h>
#define MAX_TIMEOUT			10000

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
	int32_t SendCommand( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize );

private:
	int32_t Send( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize );
	int32_t GetVersion( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize );

private:
	//	Rx/Tx Buffer  --> changed to malloc
	//	Key(4bytes) + Length(2bytes) + Command(2bytes) + Payload(MAX 65533bytes )
	// uint8_t m_SendBuf[MAX_PAYLOAD_SIZE + 8];
	// uint8_t m_ReceiveBuf[MAX_PAYLOAD_SIZE + 8];
	uint32_t ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize );
	uint32_t WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize );
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
int32_t CNX_IPCClient::SendCommand( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize )
{
	return Send( nCmd, pBuf, nBufSize );
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::Send( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize )
{
	if( NULL == *pBuf )
	{
		NxErrMsg( "Error : Buffer is NULL.\n" );
		return -1;
	}

	int32_t iClntSock;
	uint32_t nSendSize;
	uint32_t nReadSize;
	uint32_t nWriteResult;

	uint8_t* pSendBuf = NULL;
	uint8_t* pReceiveBuf = NULL;
	uint32_t nSendBufLength, nReceiveBufLength;

	uint8_t keyBuf[4];
	uint8_t cmdBuf[2];
	uint8_t lengthBuf[4];
	uint32_t iRecKey , iRecCmd;

	iClntSock = LS_Connect( IPC_SERVER_FILE );
	if( -1 == iClntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE );
		return -1;
	}

	nSendBufLength = 4 + 2 + 4 + *nBufSize; // Key(4) + cmd(2) + length(2) + payloadsize
	pSendBuf = (uint8_t*)malloc( sizeof(uint8_t) * nSendBufLength );
	nSendSize = IPC_MakePacket( NXP_KEY_VALUE, nCmd, *pBuf, *nBufSize, pSendBuf, nSendBufLength );
	if( 0 == nSendSize )
	{
		NxErrMsg( "Error: IPC_MakePacket().\n" );
		goto ERR;
	}

	//NX_HexDump( pSendBuf, nSendSize, "Send: " );

	nWriteResult = WriteData( iClntSock, pSendBuf, nSendSize );
	if( nSendSize != nWriteResult )
	{
		NxErrMsg( "Error: WriteData().\n" );
		goto ERR;
	}

	nReadSize = ReadData( iClntSock, keyBuf, 4 );
	if( 4 > nReadSize )
	{
		NxErrMsg( "Error: ReadData key .\n" );
		goto ERR;
	}
	iRecKey =  keyBuf[0]<<24 | keyBuf[1]<<16 | keyBuf[2]<<8 | keyBuf[3];

	if(SEC_KEY_VALUE != iRecKey)
	{
		NxErrMsg( "Error: key is incorrect.\n" );
		goto ERR;
	}

	nReadSize = ReadData( iClntSock, cmdBuf, 2 );
	if( 2 > nReadSize )
	{
		NxErrMsg( "Error: ReadData cmd .\n" );
		goto ERR;
	}
	iRecCmd = cmdBuf[0] << 8 | cmdBuf[1];

	nReadSize = ReadData( iClntSock, lengthBuf, 4 );
	if( 4 > nReadSize )
	{
		NxErrMsg( "Error: ReadData length .\n" );
		goto ERR;
	}
	nReceiveBufLength = lengthBuf[0]<<24 | lengthBuf[1]<<16 | lengthBuf[2]<<8 | lengthBuf[3];

	if(0 == nReceiveBufLength)
	{
		NxErrMsg("\nError: value length is 0!!!!\nError: value length is 0!!!!\nError: value length is 0!!!!\n\n");
	}

	pReceiveBuf = (uint8_t*)malloc( sizeof(uint8_t) * nReceiveBufLength );
	if(NULL == pReceiveBuf)
	{
		NxErrMsg( "Error: malloc payload .\n" );
		goto ERR;
	}

	nReadSize = ReadData( iClntSock , pReceiveBuf , nReceiveBufLength );
	if( nReceiveBufLength != nReadSize)
	{
		NxErrMsg( "Error: ReadData payload .\n" );
		goto ERR;
	}

	*nBufSize = nReceiveBufLength;
	*pBuf = (uint8_t*) realloc(*pBuf , sizeof(uint8_t) * (*nBufSize) );
	if(NULL == *pBuf)
	{
		NxErrMsg("realloc failed\n");
	}
	memcpy(*pBuf , pReceiveBuf , *nBufSize);


	close( iClntSock );
	if(NULL != pSendBuf) free(pSendBuf);
	if(NULL != pReceiveBuf) free(pReceiveBuf);
	return 0;

	ERR:
	close( iClntSock );
	if(NULL != pSendBuf) free(pSendBuf);
	if(NULL != pReceiveBuf) free(pReceiveBuf);
	return -1;
}

//------------------------------------------------------------------------------
uint32_t CNX_IPCClient::WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
{
	uint32_t totalSize = 0;
	int32_t writeSize = 0;
	uint8_t* tempBuf = pBuf;
	do {
		writeSize = write( fd, tempBuf, nSize );

		if( 0 >= writeSize )
		{
			NxErrMsg( "Fail, write().\n" );
			return totalSize;
		}

		nSize     -= writeSize;
		tempBuf   += writeSize;
		totalSize += writeSize;
	} while(nSize > 0);

	return totalSize;
}

//------------------------------------------------------------------------------
uint32_t CNX_IPCClient::ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
{
	uint32_t nTotalSize = 0;
	int64_t iReadSize = 0;
	uint8_t* tempBuf = pBuf;
	do {
		int32_t ret;
		struct pollfd hPoll;

		hPoll.fd		= fd;
		hPoll.events	= POLLIN | POLLERR;
		hPoll.revents	= 0;
		ret = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );

		if( 0 < ret )
		{
			iReadSize = read( fd, tempBuf, nSize );
			if( 0 >= iReadSize )
			{
				NxErrMsg("CNX_IPCClient::ReadData read err\n");
				return nTotalSize;
			}

			nSize     -= (uint32_t)iReadSize;
			tempBuf   += iReadSize;
			nTotalSize += (uint32_t)iReadSize;
		}
		else if( 0 > ret )
		{
			NxErrMsg( "Fail, poll().\n" );
			return nTotalSize;
		}
		else
		{
			NxErrMsg( "Timeout. ReadData().\n");
			return nTotalSize;
		}
	} while(nSize > 0);

	return nTotalSize;
}


//------------------------------------------------------------------------------
int32_t CNX_IPCClient::GetVersion( uint32_t /*nCmd*/, uint8_t** pBuf, uint32_t* nBufSize )
{
	(void*)nBufSize;
	uint8_t version[1024];
	snprintf( (char*)version, sizeof(version), "%s ( %s, %s )", NX_VERSION_IPC_CLIENT, __TIME__, __DATE__ );

	uint32_t payloadSize = (uint32_t)strlen((const char*)version);

	memcpy( *pBuf, version, payloadSize );

	*nBufSize = payloadSize;
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
int32_t NX_IPCSendCommand( uint32_t nCmd, uint8_t** pBuf, uint32_t* nBufSize )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->SendCommand( nCmd, pBuf, nBufSize );
}
