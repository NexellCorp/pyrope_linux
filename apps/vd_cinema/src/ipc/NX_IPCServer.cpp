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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_I2C.h>
#include <CNX_EEPRom.h>
#include <CNX_EEPRomDataParser.h>
#include <NX_I2CRegister.h>
#include <SockUtils.h>
#include <ipc_protocol.h>

#include <NX_IPCCommand.h>
#include <NX_IPCServer.h>
#include <CNX_BaseClass.h>

#include <NX_Version.h>

#define NX_DTAG	"[IPC Server]"
#include <NX_DbgMsg.h>

#include "NX_I2CController.h"

#define FAKE_DATA			0

#define I2C_DEBUG			0
#define I2C_SEPARATE_BURST	1

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

#define TCON_BURST_RETRY_COUNT			3

#define TCON_EEPROM_DATA_SIZE			128 * 1024	// 128 KBytes
#define TCON_EEPROM_VERSION_SIZE		512
#define TCON_EEPROM_PAGE_SIZE			256
#define TCON_EEPROM_MAX_VERSION_SIZE	257

#define TCON_EEPROM_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_EEPROM"
#define TCON_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/TCON_USB"
#define PFPGA_USB_RESULT_PATH			"/storage/sdcard0/SAMSUNG/PFPGA"

#define TCON_EEPROM_BINARY_FILE			"/storage/sdcard0/SAMSUNG/TCON_EEPROM/T_REG.bin"


//------------------------------------------------------------------------------
//
//	IPC Server APIs
//
class CNX_IPCServer : protected CNX_Thread
{
public:
	CNX_IPCServer();
	~CNX_IPCServer();

	static CNX_IPCServer* GetInstance();
	static void ReleaseInstance();

public:
	int32_t StartServer();
	void StopServer();

	void SetNapVersion( uint8_t *pVersion, int32_t iSize );
	void SetSapVersion( uint8_t *pVersion, int32_t iSize );

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	//	Local Socket
	int32_t WaitClient();
	uint32_t ReadData(int32_t fd, uint8_t *pBuf, uint32_t size);

private:
	//	Private Member Variables
	enum { MAX_LOD_MODULE = 12, MAX_STR_SIZE = 1024 };

	bool		m_IsRunning;
	bool		m_ExitLoop;
	pthread_t	m_hThread;

	int32_t		m_hSocket;

	int32_t		(*m_pTestPatternFunc[6])( CNX_I2C*, uint8_t, uint8_t );

	uint8_t		m_NapVersion[MAX_STR_SIZE];
	uint8_t		m_SapVersion[MAX_STR_SIZE];
	uint8_t		m_IpcVersion[MAX_STR_SIZE];

private:
	//	For Singletone
	static CNX_IPCServer	*m_psInstance;
};

CNX_IPCServer* CNX_IPCServer::m_psInstance = NULL;

static void    MakeDirectory( const char *pDir );

//------------------------------------------------------------------------------
CNX_IPCServer::CNX_IPCServer()
	: m_IsRunning (false)
	, m_ExitLoop (true)
	, m_hSocket (-1)
{
	memset( m_NapVersion, 0x00, sizeof(m_NapVersion) );
	memset( m_SapVersion, 0x00, sizeof(m_SapVersion) );
	memset( m_IpcVersion, 0x00, sizeof(m_IpcVersion) );

	snprintf( (char*)m_IpcVersion, sizeof(m_IpcVersion), "%s ( %s, %s )", NX_VERSION_IPC_SERVER, __TIME__, __DATE__ );

	MakeDirectory( TCON_EEPROM_RESULT_PATH );
	MakeDirectory( TCON_USB_RESULT_PATH );
	MakeDirectory( PFPGA_USB_RESULT_PATH );
}

//------------------------------------------------------------------------------
CNX_IPCServer::~CNX_IPCServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::StartServer()
{
	if( m_IsRunning )
		return 1;

	int32_t svrSocket = -1;
	svrSocket = LS_Open( IPC_SERVER_FILE );
	if( svrSocket < 0 )
	{
		NxErrMsg( "Error : server socket \n");
		return false;
	}

	m_hSocket = svrSocket;
	m_ExitLoop = false;

	if( 0 != Start() )
	{
		m_ExitLoop = true;
		close(m_hSocket);
		m_hSocket = -1;
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_IPCServer::StopServer()
{
	if( m_IsRunning )
	{
		m_ExitLoop = true;
		Stop();
	}
}

//------------------------------------------------------------------------------
void CNX_IPCServer::SetNapVersion( uint8_t *pVersion, int32_t iSize )
{
	memset( m_NapVersion, 0x00, sizeof(m_NapVersion) );
	memcpy( m_NapVersion, pVersion, iSize );
}

//------------------------------------------------------------------------------
void CNX_IPCServer::SetSapVersion( uint8_t *pVersion, int32_t iSize )
{
	memset( m_SapVersion, 0x00, sizeof(m_SapVersion) );
	memcpy( m_SapVersion, pVersion, iSize );
}

//------------------------------------------------------------------------------
void CNX_IPCServer::ThreadProc()
{
	uint32_t nReadSize; //for read

	int32_t iI2CResult; //for temp.. NX_SendData

	while( !m_ExitLoop )
	{
		uint32_t nKey = 0;
		uint16_t iCmd = 0;
		uint32_t nLength = 0;

		uint8_t cmdBuf[2];
		uint8_t lengthBuf[4];
		uint8_t* pValue = NULL;

		uint32_t nSendSize;	//result of make packet ( total data length )

		uint8_t* pI2CResult = NULL;		//result of I2C
		uint32_t nI2CResultSize = 0;	//result size of I2C

		uint8_t* pReplyBuf = NULL;	// data for client
		uint32_t nReplyBufSize = 0;	// Key(4bytes) + Command(2bytes) + Length(4bytes) + pValueSize

		int32_t clientSocket = WaitClient();
		if( 0 > clientSocket )	//	Error
		{
			break;
		}

		//
		//	Find Key Code 4 bytes
		//
		do{
			uint8_t tmp;
			nReadSize = ReadData( clientSocket, &tmp, 1 );
			if( 1 != nReadSize )
			{
				NxErrMsg("Read Error!!!\n");
				goto ERROR;
			}
			nKey = (nKey<<8) | tmp;
			if( nKey == NXP_KEY_VALUE )
			{
				break;
			}
		}while(1);

		//	Read Command 2 bytes
		nReadSize = ReadData(clientSocket , cmdBuf , 2);
		if( 0 >= nReadSize )
		{
			NxErrMsg("Error : Read Command\n");
			goto ERROR;
		}
		iCmd = IPC_GET_COMMAND(cmdBuf[0] , cmdBuf[1]);

		//	Read Length 4 bytes
		nReadSize = ReadData(clientSocket, lengthBuf, 4);
		if( 0 >= nReadSize )
		{
			NxErrMsg("Error : Read Length\n");
			goto ERROR;
		}
		nLength = IPC_GET_LENGTH(lengthBuf[0] , lengthBuf[1] , lengthBuf[2] , lengthBuf[3]);

		if( 0 != nLength )
		{
			pValue = (uint8_t*)malloc( sizeof(uint8_t) * nLength );
			if( NULL == pValue )
			{
				NxErrMsg( "Fail, malloc() for value , check Length.\n" );
				goto ERROR;
			}

			nReadSize = ReadData( clientSocket, pValue , nLength );
			if( nLength != nReadSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				goto ERROR;
			}
		}

		// NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
		// NxDbgMsg( NX_DBG_VBS, "iCmd = 0x%08x, nReadSize = %d\n", iCmd, nReadSize );

		iI2CResult = NX_SendData( this , iCmd , pValue , nLength , &pI2CResult , &nI2CResultSize );
		if(0 > iI2CResult)
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, I2CController.NX_SendData \n" );
		}

		// make I2C result to form of protocol
		nReplyBufSize = 4 + 2 + 4 + nI2CResultSize;
		pReplyBuf = (uint8_t*) malloc( sizeof(uint8_t) * nReplyBufSize );
		if( NULL == pReplyBuf )
		{
			NxErrMsg( "Fail, malloc() for reply (pReplyBuf).\n" );
			goto ERROR;
		}

		nSendSize = IPC_MakePacket( SEC_KEY_VALUE, iCmd, pI2CResult, nI2CResultSize, pReplyBuf, nReplyBufSize );
		if(0 == nSendSize)
		{
			NxErrMsg( "Fail, IPC_MakePacket(). ret : %d\n" ,nSendSize );
			goto ERROR;
		}
		write( clientSocket, pReplyBuf, nSendSize );

ERROR:
		if( clientSocket > 0 )
			close( clientSocket );

		if(pValue) free(pValue);
		if(pReplyBuf) free(pReplyBuf);
		if(pI2CResult) free(pI2CResult);
	}
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::WaitClient()
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
uint32_t CNX_IPCServer::ReadData(int32_t fd, uint8_t *pBuf, uint32_t size)
{
	int64_t readSize = 0;
	uint32_t totalSize = 0;
	uint8_t* tempBuf = pBuf;
	do
	{
		readSize = read( fd, tempBuf, size );
		if( readSize < 0 )
		{
			NxErrMsg("NX_IPCServer , ReadData read err\n");
			return -1;
		}

		size -= (uint32_t)readSize;
		tempBuf += readSize;
		totalSize += (uint32_t)readSize;
	} while(size > 0);
	return totalSize;
}


//------------------------------------------------------------------------------
static void MakeDirectory( const char *pDir )
{
	char buf[1024];
	char *pBuf = buf;

	memcpy( buf, pDir, sizeof(buf) -1 );
	buf[sizeof(buf)-1] = 0x00;

	while( *pBuf )
	{
		if( '/' == *pBuf )
		{
			*pBuf = 0x00;
			if( 0 != access( buf, F_OK ) && (pBuf != buf) )
			{
				NxDbgMsg( NX_DBG_VBS, "Make Directory. ( %s )\n", buf );
				mkdir( buf, 0777 );
			}
			*pBuf = '/';
		}
		pBuf++;
	}

	if( 0 != access( buf, F_OK) )
	{
		NxDbgMsg( NX_DBG_VBS, "Make Directory. ( %s )\n", buf );
		mkdir( buf, 0777 );
	}
}

//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_IPCServer* CNX_IPCServer::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_IPCServer();
	}
	return (CNX_IPCServer*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_IPCServer::ReleaseInstance( )
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
int32_t NX_IPCServerStart()
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	return hIpc->StartServer();
}

//------------------------------------------------------------------------------
void NX_IPCServerStop()
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	hIpc->StopServer();
}

//------------------------------------------------------------------------------
void NX_SetNapVersion( uint8_t *pVersion, int32_t iSize )
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	hIpc->SetNapVersion( pVersion, iSize );
}

//------------------------------------------------------------------------------
void NX_SetSapVersion( uint8_t *pVersion, int32_t iSize )
{
	CNX_IPCServer *hIpc = CNX_IPCServer::GetInstance();
	hIpc->SetSapVersion( pVersion, iSize );
}
