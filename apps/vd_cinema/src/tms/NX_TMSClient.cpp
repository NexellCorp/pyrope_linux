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

#include <tms_protocol.h>
#include <SockUtils.h>

#include <NX_TMSServer.h>
#include <NX_TMSClient.h>
#include <NX_Utils.h>

#define NX_DTAG	"[TMS Client]"
#include <NX_DbgMsg.h>

#include <poll.h>
#define MAX_TIMEOUT			10000

class CNX_TMSClient
{
public:
	CNX_TMSClient(){}
	~CNX_TMSClient(){}

	static CNX_TMSClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t SendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t** pBuf, uint32_t* nBufSize );

private:
	uint32_t ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize );
	uint32_t WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize );

private:
	static CNX_TMSClient *m_psInstance;
};

CNX_TMSClient* CNX_TMSClient::m_psInstance = NULL;

//------------------------------------------------------------------------------
//
//	TMS Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_TMSClient::SendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t** pBuf, uint32_t* nBufSize )
{
	//
	//	Condition of pBuf
	//	 1. The pBuf must not be NULL.
	//	 2. The nBufSize is real data size in pBuf. ( just Value[], not including length or anything )
	//
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


	iClntSock = TCP_Connect( pIpAddr, TMS_SERVER_PORT );
	if( -1 == iClntSock)
	{
		NxErrMsg( "Error : TCP_Connect (%s:%d)\n", pIpAddr, TMS_SERVER_PORT );
		return -1;
	}


	nSendBufLength = 4 + 2 + 4 + *nBufSize; // Key(4) + cmd(2) + length(2) + payloadsize
	pSendBuf = (uint8_t*)malloc( sizeof(uint8_t) * nSendBufLength );
	nSendSize = TMS_MakePacket( TMS_KEY, iCmd, *pBuf, *nBufSize, pSendBuf, nSendBufLength );
	if( 0 == nSendSize )
	{
		NxErrMsg( "Error: TMS_MakePacket().\n" );
		goto ERR;
	}

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

	if(SEC_KEY != iRecKey)
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

	// printf("==================================================\n");
	// printf("key = 0x%08x\n", iRecKey);
	// printf("cmd = 0x%08x\n", iRecCmd);
	// printf("payloadSize = %d\n", nReceiveBufLength);
	// printf("==================================================\n");

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
uint32_t CNX_TMSClient::WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
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
uint32_t CNX_TMSClient::ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
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
				NxErrMsg("CNX_TMSClient::ReadData read err\n");
				return nTotalSize;
			}

			nSize      -= (uint32_t)iReadSize;
			tempBuf    += iReadSize;
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
int32_t NX_TMSSendCommand( const char *pIpAddr, uint32_t iCmd, uint8_t** pBuf, uint32_t* nBufSize )
{
	CNX_TMSClient *hTms = CNX_TMSClient::GetInstance();
	return hTms->SendCommand( pIpAddr, iCmd, pBuf, nBufSize );
}
