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
#include <gdc_protocol.h>

#include <NX_CinemaCommand.h>
#include <NX_TMSServer.h>
#include <CNX_Base.h>
#include <CNX_CinemaManager.h>

#include <NX_Utils.h>
#include <NX_Version.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			10000
#define MAX_PAYLOAD_SIZE	65535

#ifndef UNUSED
#define UNUSED(x)			(void)(x)
#endif

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
	void	RegisterProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj );

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	int32_t BusyResource( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize );
	int32_t QueCommand( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize );
	int32_t DeleteConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize );
	int32_t UploadConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize );
	int32_t DownloadConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize );

	int32_t WaitClient();

	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t SendRemote( const char *pSockName, uint8_t *pSendBuf, int32_t iSendSize, uint8_t *pRecvBuf = NULL, int32_t iRecvSize = 0 );

private:
	enum { TMS_CMD_DELETE_CONFIG_ALL = 30, TMS_CMD_DELETE_CONFIG_10 = 31, TMS_CMD_DELETE_CONFIG_29 = 50 };
	enum { TMS_CMD_UPLOAD_CONFIG = 200, TMS_CMD_DOWNLOAD_CONFIG = 201 };

	enum { MAX_CMD_STEP = 3 };

	//	Private Member Variables
	bool		m_bThreadRun;
	int32_t		m_hSocket;

	//	Key(4bytes) + Length(2bytes) + Payload(MAX 65535bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 6];
	uint8_t		m_RecvBuf[MAX_PAYLOAD_SIZE + 6];

	void 		(*m_cbProcessCallback)( uint32_t iCmd, uint8_t *pInBuf, int32_t iInSize, uint8_t *pOutBuf, int32_t *iOutSize, void *pObj );
	void		*m_pObj;

	CNX_CinemaManager* m_pCinema;

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
CNX_TMSServer::CNX_TMSServer()
	: m_bThreadRun( false )
	, m_hSocket( -1 )
	, m_cbProcessCallback( NULL )
	, m_pObj( NULL )
	, m_pCinema( CNX_CinemaManager::GetInstance() )
{
	char szVersion[1024] = { 0x00, };
	snprintf( (char*)szVersion, sizeof(szVersion), "%s ( %08lld-%06lld )", NX_VERSION_TMS_SERVER, NX_DATE(), NX_TIME() );

	m_pCinema->SetTmsVersion( (uint8_t*)szVersion, strlen(szVersion) );
}

//------------------------------------------------------------------------------
CNX_TMSServer::~CNX_TMSServer()
{
	StopServer();

	CNX_CinemaManager::ReleaseInstance();
	m_pCinema = NULL;
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
		NxDbgMsg( NX_DBG_ERR, "Fail, TCP_Open().\n" );
		return -1;
	}

	if( 0 > listen(m_hSocket, 5) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail : listen (err = %d)\n", errno );
		goto ERROR_TMS;
	}

	m_bThreadRun = true;

	if( 0 != Start() )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Start().\n" );
		goto ERROR_TMS;
	}

	return 0;

ERROR_TMS:
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
void CNX_TMSServer::RegisterProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	m_cbProcessCallback = cbCallback;
	m_pObj = pObj;
}

//------------------------------------------------------------------------------
void CNX_TMSServer::ThreadProc()
{
	int32_t iReadSize;

	uint8_t iCmd = 0x00;
	int32_t iCmdStep = 0;
	int32_t iCmdPend = 0;

	while( m_bThreadRun )
	{
		int32_t iClientSocket = WaitClient();
		if( !iClientSocket )
		{
			continue;
		}
		else if( 0 > iClientSocket )
		{
			goto ERROR_TMS;
		}

		while( m_bThreadRun )
		{
			uint8_t *pRecvBuf = m_RecvBuf;
			uint32_t iKey = 0;
			uint8_t *pPayload;
			uint16_t iPayloadSize;

			do {
				uint8_t temp;
				iReadSize = ReadData( iClientSocket, &temp, 1 );
				if( iReadSize != 1 )
				{
					NxErrMsg( "Fail, ReadData().\n" );
					break;
				}

				iKey = (iKey << 8) | temp;
				if( iKey == KEY_GDC(temp) )
				{
					break;
				}
			} while( m_bThreadRun );

			if( 1 > iReadSize )
			{
				goto ERROR_TMS;
			}

			//	Key
			*pRecvBuf++ = (( iKey >> 24) & 0xFF);
			*pRecvBuf++ = (( iKey >> 16) & 0xFF);
			*pRecvBuf++ = (( iKey >>  8) & 0xFF);
			*pRecvBuf++ = (( iKey      ) & 0xFF);

			//	Read Length
			iReadSize = ReadData( iClientSocket, pRecvBuf, 2 );
			if( 2 != iReadSize )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read Length. ( read: %d, expected: 2 )\n", iReadSize );
				goto ERROR_TMS;
			}

			iPayloadSize = MAKE_LENGTH( pRecvBuf[0], pRecvBuf[1] );
			pRecvBuf += 2;

			//	Size Check
			if( iPayloadSize+8 > (int32_t)sizeof(m_RecvBuf) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Check Payload Size.\n" );
				goto ERROR_TMS;
			}

			//	Read all data
			if( iPayloadSize != 0 )
			{
				iReadSize = ReadData( iClientSocket, pRecvBuf, iPayloadSize );
				if( iReadSize != iPayloadSize )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read Payload. ( read: %d, expected: %d )\n", iReadSize, iPayloadSize );
					goto ERROR_TMS;
				}
			}

			pPayload = pRecvBuf;

			//	Check Valid Command
			if( KEY_GDC( CMD_IMB(IMB_CMD_QUE) ) != iKey )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Invalid Command. ( 0x%08X )\n", iKey );
				break;
			}

			NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
			NxDbgMsg( NX_DBG_VBS, "iKey = 0x%08x, iPayloadSize = %d\n", iKey, iPayloadSize );

			NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
			NX_HexDump( m_RecvBuf, iPayloadSize + 6, NX_DTAG, "Recv" );

			if( m_pCinema->IsBusy() )
			{
				BusyResource( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize );
				break;
			}
			else
			{
				if( ( TMS_CMD_UPLOAD_CONFIG == pPayload[0] ) ||			//	Upload User Mode
					( TMS_CMD_UPLOAD_CONFIG == iCmd        ) )
				{
					if( TMS_CMD_UPLOAD_CONFIG == pPayload[0] )
					{
						iCmd = pPayload[0];
					}

					if( 0 != UploadConfig( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize ) )
					{
						break;
					}
				}
				else if( ( TMS_CMD_DOWNLOAD_CONFIG == pPayload[0] ) ||		// Download User Mode
						 ( TMS_CMD_DOWNLOAD_CONFIG == iCmd        ) )
				{
					if( TMS_CMD_DOWNLOAD_CONFIG == pPayload[0] )
					{
						iCmd = pPayload[0];
					}

					if( 0 != DownloadConfig( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize ) )
					{
						break;
					}
				}
				else if( (  TMS_CMD_DELETE_CONFIG_ALL == pPayload[0] ) ||	// Delete Config All
						 (  TMS_CMD_DELETE_CONFIG_ALL == iCmd        ) )
				{
					if( TMS_CMD_DELETE_CONFIG_ALL == pPayload[0] )
					{
						iCmd = pPayload[0];
					}

					if( 0 != DeleteConfig( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize ) )
					{
						break;
					}
				}
				else if( (  TMS_CMD_DELETE_CONFIG_10 <= pPayload[0] &&  TMS_CMD_DELETE_CONFIG_29 >= pPayload[0]) ||		//	Delete user defined mode
						 (  TMS_CMD_DELETE_CONFIG_10 <= iCmd        &&  TMS_CMD_DELETE_CONFIG_29 >= iCmd       ) )
				{
					if( TMS_CMD_DELETE_CONFIG_10 <= pPayload[0] || TMS_CMD_DELETE_CONFIG_29 >= pPayload[0] )
					{
						iCmd = pPayload[0];
					}

					if( 0 != DeleteConfig( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize ) )
					{
						break;
					}
				}
				else														//	Change Mode & System Setting Command
				{
					if( 0 != QueCommand( iClientSocket, iCmd, iCmdStep++, iKey, (uint8_t*)pPayload, iPayloadSize ) )
					{
						break;
					}
				}
				// else
				// {
				// 	uint8_t result = 0xFF;
				// 	int32_t iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );
				// 	WriteData( iClientSocket, m_SendBuf, iSendSize );

				// 	NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
				// 	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
				// 	break;
				// }
			}

			if( MAX_CMD_STEP < iCmdPend++ )
			{
				break;
			}
		}

ERROR_TMS:
		iCmd =
		iCmdStep =
		iCmdPend = 0;

		if( 0 < iClientSocket )
		{
			close( iClientSocket );
			iClientSocket = -1;
		}
	}
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::BusyResource( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize )
{
	UNUSED( iCmd );
	UNUSED( iCmdStep );
	UNUSED( pPayload );
	UNUSED( iPayloadSize );

	int32_t iSendSize;
	uint8_t result = 0xFE;

	NxDbgMsg( NX_DBG_INFO, "IsBusy().\n" );

	iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );
	WriteData( iSock, m_SendBuf, iSendSize );

	NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );

	return 1;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::QueCommand( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize )
{
	UNUSED( iCmd );
	UNUSED( iCmdStep );
	UNUSED( iPayloadSize );

	int32_t iSendSize;
	uint8_t result = 0xFF;

	NxDbgMsg( NX_DBG_INFO, "Send QueCommand.\n" );

	uint8_t recvData[1024] = { 0x00, };
	if( 0 > SendRemote( "cinema.tms", pPayload, iPayloadSize, recvData, sizeof(recvData) ) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
		printf("Fail, SendRemote. ( node: cinema.tms )\n");
	}
	else
	{
		result = !strncmp( (char*)recvData, "pass", sizeof(recvData) ) ? 0x01 : 0x00;
	}

	iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );
	WriteData( iSock, m_SendBuf, iSendSize );

	NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
	NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );

	return 1;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::DeleteConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize )
{
	UNUSED( iSock );
	UNUSED( iCmd );
	UNUSED( iKey );
	UNUSED( pPayload );
	UNUSED( iPayloadSize );

	uint8_t outBuf[MAX_PAYLOAD_SIZE];
	int32_t iOutSize;
	int32_t iSendSize;

	switch( iCmdStep )
	{
		case 0:
		{
			uint8_t result[MAX_PAYLOAD_SIZE];

			if( 0 > SendRemote( "cinema.tms", pPayload, iPayloadSize, result, sizeof(result) ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
				printf("Fail, SendRemote. ( node: cinema.tms )\n");
			}

			result[0] = 0x01;

			iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), &result, 1, m_SendBuf, sizeof(m_SendBuf) );
			WriteData( iSock, m_SendBuf, iSendSize );

			NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
			NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive QueCommand Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );

			return 1;
		}

		default:
			break;
	}

	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::UploadConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize )
{
	int32_t iSendSize;

	switch( iCmdStep )
	{
		case 0:
		{
			uint8_t result = iCmd;
			iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > WriteData( iSock, m_SendBuf, iSendSize ) )
			{
				return -1;
			}

			NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
			return 0;
		}

		case 1:
		{
			uint8_t result[MAX_PAYLOAD_SIZE];

			uint8_t *pInBuf = (uint8_t*)malloc( sizeof(uint8_t)*(iPayloadSize+1) );
			memcpy( pInBuf+1, pPayload, iPayloadSize );
			pInBuf[0] = iCmd;

			if( 0 > SendRemote( "cinema.tms", pInBuf, iPayloadSize+1, result, sizeof(result) ) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, SendRemote. ( node: cinema.tms )\n");
				printf("Fail, SendRemote. ( node: cinema.tms )\n");
			}
			if( pInBuf ) free( pInBuf );

			result[0] = (iPayloadSize >> 8) & 0xFF;
			result[1] = (iPayloadSize >> 0) & 0xFF;

			iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), result, 2, m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > WriteData( iSock, m_SendBuf, iSendSize ) )
			{
				return -1;
			}

			NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
			NxDbgMsg( NX_DBG_DEBUG, ">>>> Receive UploadConfig Done. ( curTime: %lld mSec )\n",  NX_GetTickCount() );
			return 1;
		}

		default:
			break;
	}

	return -1;
}

//------------------------------------------------------------------------------
#include <CNX_CinemaBehavior.h>

int32_t CNX_TMSServer::DownloadConfig( int32_t iSock, uint8_t iCmd, int32_t iCmdStep, uint32_t iKey, uint8_t *pPayload, int32_t iPayloadSize )
{
	UNUSED( iCmd );
	UNUSED( iPayloadSize );

	int32_t iSendSize;
	static int32_t iMode = 0;

	switch( iCmdStep )
	{
		case 0:
		{
			iMode = pPayload[1];
			for( int32_t i = 0; i < 3; i++ )
			{
				char szFile[1024];
				snprintf( szFile, sizeof(szFile), "/storage/sdcard0/SAMSUNG/TCON_BEHAVIOR/BEHAVIOR_%s_MODE%02d.txt",
					(i == 0) ? "R" : ((i==1) ? "G" : "B"),
					iMode
				);

				if( 0 > access(szFile, F_OK) )
					continue;

				CNX_CinemaBehavior behavior;
				if( !behavior.Parse( szFile ) )
				{
					NX_BEHAVIOR_INFO *pInfo;
					behavior.GetBehaviorInfo( &pInfo );

					iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF),
						(pInfo->pDesc == NULL) ? NULL : pInfo->pDesc,
						(pInfo->pDesc == NULL) ? 0 : strlen(pInfo->pDesc),
						m_SendBuf, sizeof(m_SendBuf)
					);

					NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );

					if( 0 > WriteData( iSock, m_SendBuf, iSendSize ) )
					{
						return -1;
					}
					return 0;
				}
			}

			iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
			WriteData( iSock, m_SendBuf, iSendSize );

			NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
			return -1;
		}

		case 1:
		{
			uint8_t result[MAX_PAYLOAD_SIZE];
			uint8_t inBuf[2] = {
				(uint8_t)iMode,
				(uint8_t)((pPayload[1] == 'R') ? 0 : ((pPayload[1] == 'G') ? 1 : 2)) };
			int32_t iReadSize;

			m_pCinema->SendCommand(PLATFORM_CMD_CONFIG_DOWNLOAD, inBuf, sizeof(inBuf), result, &iReadSize );

			iSendSize = GDC_MakePacket( KEY_SEC(iKey & 0xFF), result, iReadSize, m_SendBuf, sizeof(m_SendBuf) );
			if( 0 > WriteData( iSock, m_SendBuf, iSendSize ) )
			{
				return -1;
			}

			NX_HexDump( m_SendBuf, iSendSize, NX_DTAG, "Send" );
			return 1;
		}

		default:
			break;
	}
	return -1;
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
				NxDbgMsg( NX_DBG_ERR, "Fail, accept().\n" );
				return -1;
			}

			// printf( "Connect Success. ( %d )\n", clnSocket );
			return clnSocket;
		}
	}
	else if( 0 > ret )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, poll().\n");
		return -1;
	}

	NxDbgMsg( NX_DBG_VBS, "Timeout. WaitClient().\n");
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize )
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
			NxDbgMsg( NX_DBG_VBS, "Timeout. ReadData().\n");
			return 0;
		}
	} while(iSize > 0);

	return totalSize;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::SendRemote( const char *pSockName, uint8_t *pSendBuf, int32_t iSendSize, uint8_t *pRecvBuf, int32_t iRecvSize )
{
	int32_t sock, len;
	struct sockaddr_un addr;
	int32_t iReadSize = 0;

	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, socket().\n");
		printf("Fail, socket().\n");
		return -1;
	}

	addr.sun_family  = AF_UNIX;
	addr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( addr.sun_path + 1, pSockName );

	len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, connect(). ( node: %s )\n", pSockName);
		printf("Fail, connect(). ( node: %s )\n", pSockName);
		close( sock );
		return -1;
	}

	//
	// by doriya
	//
	uint8_t *pBuf = (uint8_t*)malloc( sizeof(uint8_t) * (iSendSize + 2) );
	pBuf[0] = (iSendSize >> 8) & 0xFF;
	pBuf[1] = (iSendSize >> 0) & 0xFF;
	memcpy( pBuf + 2, pSendBuf, iSendSize );

	if( 0 > write(sock, (uint8_t*)pBuf, iSendSize+2) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, write().\n");
		printf("Fail, write().\n");
		close( sock );
		if( pBuf ) free( pBuf );
		return -1;
	}

	if( NULL != pRecvBuf && 0 < iRecvSize )
	{
		if( 0 > (iReadSize = read(sock, (uint8_t*)pRecvBuf, iRecvSize)) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, read().\n");
			printf("Fail, read().\n");
			close( sock );
			if( pBuf ) free( pBuf );
			return -1;
		}
	}

	close( sock );
	if( pBuf ) free( pBuf );
	return iReadSize;
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

//------------------------------------------------------------------------------
void NX_TMSServerProcessCallback( void (*cbCallback)(uint32_t, uint8_t*, int32_t, uint8_t*, int32_t*, void*), void *pObj )
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->RegisterProcessCallback( cbCallback, pObj );
}
