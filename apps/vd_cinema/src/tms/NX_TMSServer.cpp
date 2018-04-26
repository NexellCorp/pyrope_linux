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
#include <tms_protocol.h>

#include <NX_IPCCommand.h>
#include <NX_TMSServer.h>
#include <CNX_BaseClass.h>
#include <NX_Utils.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#include <stdlib.h>
#include "NX_I2CController.h"
#define MAX_TIMEOUT			10000

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

static uint32_t ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize );
static uint32_t WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize );
// static int32_t SendRemote( const char *pSockName, const char *pMsg );

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

private:
	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

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
	uint32_t nReadSize;	//for ReadData
	uint32_t nWriteSize;

	int32_t iI2CResult; //for temp.. NX_SendData

	while( m_bThreadRun )
	{
		uint32_t iKey = 0;
		uint16_t iCommand = 0;
		uint32_t nLength = 0;

		uint8_t commandBuf[2];
		uint8_t lengthBuf[4];
		uint8_t* pValue = NULL;

		uint32_t nSendSize;	//result of make packet ( total data length )

		uint8_t* pI2CResult = NULL;		//result of I2C
		uint32_t nI2CResultSize = 0;	//result size of I2C

		uint8_t* pReplyBuf = NULL;	// data for client
		uint32_t nReplyBufSize = 0;	// Key(4bytes) + Command(2bytes) + Length(4bytes) + pValueSize

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
			nReadSize = ReadData( clientSocket, &tempData, 1 );
			if( 1 > nReadSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				break;
			}

			iKey = (iKey << 8) | tempData;
			if( iKey == TMS_KEY )
			{
				break;
			}
		} while( m_bThreadRun );

		if( 1 > nReadSize )
		{
			goto ERROR;
		}

		//command 2bytes
		nReadSize = ReadData( clientSocket, commandBuf, 2 );
		if( 2 > nReadSize )
		{
			NxErrMsg( "Fail, ReadData().\n" );
			goto ERROR;
		}
		iCommand = MAKE_LENGTH_2BYTE( commandBuf[0], commandBuf[1] );


		//length 4bytes
		nReadSize = ReadData( clientSocket, lengthBuf, 4 );
		if( 4 > nReadSize )
		{
			NxErrMsg( "Fail, ReadData().\n" );
			goto ERROR;
		}

		nLength = MAKE_LENGTH_4BYTE( lengthBuf[0], lengthBuf[1], lengthBuf[2], lengthBuf[3] );

		if( 0 != nLength )
		{
			pValue = (uint8_t*)malloc( sizeof(uint8_t) * nLength );
			if( NULL == pValue )
			{
				NxErrMsg( "Fail, malloc() for value , check Length.\n" );
				goto ERROR;
			}

			nReadSize = ReadData( clientSocket, pValue , nLength );
			if(nReadSize != nLength)
			{
				NxErrMsg( "Fail, ReadData().\n" );
				goto ERROR;
			}
		}


		// pI2CResult will malloc in NX_SendData
		// nI2CResultSize will be determined in NX_SendData
		iI2CResult = NX_SendData( this , iCommand , pValue , nLength, &pI2CResult , &nI2CResultSize );
		if(0 > iI2CResult)
		{
			NxErrMsg( "Fail, I2CController.NX_SendData\n" );
		}

		// make I2C result to form of protocol
		nReplyBufSize = 4 + 2 + 4 + nI2CResultSize;
		pReplyBuf = (uint8_t*) malloc( sizeof(uint8_t) * nReplyBufSize );
		if( NULL == pReplyBuf )
		{
			NxErrMsg( "Fail, malloc() for reply (pReplyBuf).\n" );
			goto ERROR;
		}

		nSendSize = TMS_MakePacket( SEC_KEY , iCommand , pI2CResult , nI2CResultSize , pReplyBuf, nReplyBufSize );
		if(0 == nSendSize)
		{
			NxErrMsg( "Fail, TMS_MakePacket(). ret : %d\n" ,nSendSize );
			goto ERROR;
		}

		// send response...
		nWriteSize = WriteData( clientSocket, pReplyBuf, nSendSize );
		if(nWriteSize != nSendSize)
		{
			NxErrMsg( "Fail, WriteData(). send : %u , wrriten : %u \n" , nSendSize , nWriteSize );
		}
		//NX_HexDump( pReplyBuf, nSendSize );

ERROR:
		if( 0 < clientSocket )
		{
			close( clientSocket );
			clientSocket = -1;
		}

		if(pValue) free(pValue);
		if(pReplyBuf) free(pReplyBuf);
		if(pI2CResult) free(pI2CResult);
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
				NxErrMsg( "Fail, accept().\n" );
				return -1;
			}

			// printf( "Connect Success. ( %d )\n", clnSocket );
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
static uint32_t ReadData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
{
	uint32_t nTotalSize = 0;
	int64_t iReadSize = 0;
	do {
		int32_t ret;
		struct pollfd hPoll;

		hPoll.fd		= fd;
		hPoll.events	= POLLIN | POLLERR;
		hPoll.revents	= 0;
		ret = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
		if( 0 < ret )
		{
			iReadSize = read( fd, pBuf, nSize );
			if( 0 >= iReadSize )
			{
				NxErrMsg("NX_TMSServer::ReadData read err\n");
				return nTotalSize;
			}

			nSize     -= (uint32_t)iReadSize;
			pBuf      += iReadSize;
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
static uint32_t WriteData( int32_t fd, uint8_t *pBuf, uint32_t nSize )
{
	uint32_t nTotalSize = 0;
	int32_t iWriteSize = 0;
	do {
		iWriteSize = write( fd, pBuf, nSize );
		if( 0 >= iWriteSize )
		{
			NxErrMsg( "Fail, write().\n" );
			return nTotalSize;
		}

		nSize     -= iWriteSize;
		pBuf      += iWriteSize;
		nTotalSize += iWriteSize;
	} while(nSize > 0);

	return nTotalSize;
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
