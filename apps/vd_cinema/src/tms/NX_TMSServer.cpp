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
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>	//	errno

#include <SockUtils.h>
#include <tms_protocol.h>

#include <NX_TMSCommand.h>
#include <NX_TMSServer.h>
#include <CNX_BaseClass.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

class CNX_TMSServer : protected CNX_Thread
{
public:
	CNX_TMSServer();
	~CNX_TMSServer();

	static CNX_TMSServer* GetInstance( );
	static void ReleaseInstance( );

	int32_t StartServer();
	void StopServer();

	//	Implementation CNX_Thread pure virtual function
protected:
	virtual void ThreadProc();


	//	Private Functions
private:
	//	Local Socket
	int32_t WaitClient();
	int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size);


	//	PFPGA Commands
	int32_t PFPGA_GetState(int32_t fd);
	int32_t PFPGA_SetSource( int32_t fd, uint8_t *index );
	int32_t PFPGA_GetVersion(int32_t fd);

	//	TCON Commands
	int32_t TCON_Status( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedPosition( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedOn( int32_t fd, uint8_t onOff );
	int32_t TCON_BrightnessMode( int32_t fd, uint8_t mode, uint8_t evtMode );
	int32_t TCON_ElapsedTime( int32_t fd );
	int32_t TCON_Version( int32_t fd );

	//	Battery Commands
	int32_t BAT_GetState(int32_t fd);

	int32_t ProcessCommand( int32_t clientSocket, uint32_t cmd, void *pPayload, int32_t payloadSize );

	//	Private Member Variables
private:
	bool		m_IsRunning;
	bool		m_ExitLoop;
	pthread_t	m_hThread;

	int32_t		m_hSocket;
	uint8_t		m_PayloadBuf[MAX_PAYLOAD_SIZE];
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE+12];


	//	For Singletone
private:
	static CNX_TMSServer	*m_psInstance;
};

CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

CNX_TMSServer::CNX_TMSServer()
	: m_IsRunning (false)
	, m_ExitLoop (true)
	, m_hSocket (-1)
{
}

CNX_TMSServer::~CNX_TMSServer()
{
	StopServer();
}


int32_t CNX_TMSServer::StartServer()
{
	if( m_IsRunning )
		return 1;

	int32_t svrSocket = LS_Open( TMS_SERVER_FILE );
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

void CNX_TMSServer::StopServer()
{
	if( m_IsRunning )
	{
		m_ExitLoop = true;
		Stop();
	}
}


//
//		Wait Semaphore for Command Processing
//
void CNX_TMSServer::ThreadProc()
{
	int32_t readSize;
	uint32_t key=0, cmd=0, len;
	uint8_t *pPayload;
	int32_t payloadSize;

	while( !m_ExitLoop )
	{
		uint8_t *pBuf = m_PayloadBuf;
		int32_t clientSocket = WaitClient();
		if( 0 > clientSocket )	//	Error
		{
			break;
		}

		//
		//	Find Key Code
		//
		do{
			uint8_t tmp;
			readSize = ReadData( clientSocket, &tmp, 1 );
			if( readSize != 1 )
			{
				NxErrMsg("Read Error!!!\n");
				goto ERROR;
			}
			key = (key<<8) | tmp;
			if( key == TMS_KEY_VALUE )
			{
				break;
			}
		}while(1);


		//	Read Length
		readSize = ReadData(clientSocket, pBuf, 2);
		if( readSize <= 0 ){
			NxErrMsg("Error : Read Length\n");
			goto ERROR;
		}
		len = TMS_GET_LENGTH(pBuf[0], pBuf[1]);

		//	Size Check
		if( len+6 > MAX_PAYLOAD_SIZE )
		{
			NxErrMsg("Error : Data size\n");
			goto ERROR;
		}

		//	Read all data
		readSize = ReadData ( clientSocket, pBuf+2, len );
		if( readSize <=0 )
		{
			NxErrMsg("Error : Read Data\n");
			goto ERROR;
		}
		cmd = TMS_GET_COMMAND(pBuf[2], pBuf[3]);

		pPayload = pBuf + 4;
		payloadSize = len - 2;

		NxDbgMsg( NX_DBG_VBS, "\n================================================\n");
		NxDbgMsg( NX_DBG_VBS, "cmd = 0x%08x, readSize = %d\n", cmd, readSize );
		ProcessCommand( clientSocket, cmd, pPayload, payloadSize );

ERROR:
		if( clientSocket > 0 )
			close( clientSocket );
	}
}

int32_t CNX_TMSServer::WaitClient()
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
	return clientSocket;
}


int32_t CNX_TMSServer::ReadData(int32_t fd, uint8_t *pBuf, int32_t size)
{
	int32_t readSize, totalSize=0;
	do
	{
		readSize = read( fd, pBuf, size );
		if( readSize < 0 )
			return -1;

		size -= readSize;
		pBuf += readSize;
		totalSize += readSize;
	}while(size > 0);
	return totalSize;
}


//
//
//
int32_t CNX_TMSServer::PFPGA_GetState(int32_t fd)
{
	uint8_t state[1] = {1};
	int32_t sendSize, sent;
	FUNC_IN();

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_STATE, state, 1, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::PFPGA_SetSource( int32_t fd, uint8_t * /*index*/ )
{
	int32_t sendSize, sent;
	FUNC_IN();

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_SOURCE, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::PFPGA_GetVersion(int32_t fd)
{
	FUNC_IN();
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_VERSION, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}


int32_t CNX_TMSServer::TCON_Status( int32_t fd, uint32_t cmd, uint8_t index )
{
	FUNC_IN();
	uint8_t state[128];
	int32_t sendSize = 1, sent;

	//
	//	TODO : need real access routine
	//

	//	fake routine
	for( int32_t i=0 ; i<128 ; i++ )
	{
		state[i] = i % 2;
	}

	if( index == 0xff )
	{
		sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, state, 128, m_SendBuf, sizeof(m_SendBuf) );
	}
	else
	{
		sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &state[index], 1, m_SendBuf, sizeof(m_SendBuf) );
	}

	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::TCON_LedPosition( int32_t fd, uint32_t cmd, uint8_t index )
{
	FUNC_IN();

	uint8_t state[64][3];
	int32_t sendSize, sent;
	(void)index;

	//
	//	TODO : need real access routine
	//

	//	fake routine
	for( int32_t i=0 ; i<64 ; i++ )
	{
		state[i][0] = (i*2)%32;
		state[i][1] = (i*3)%32;
		state[i][2] = (i*4)%32;
	}
	int32_t failNumber = 3;

	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, state, failNumber * 3, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::TCON_LedOn( int32_t fd, uint8_t onOff )
{
	FUNC_IN();
	int32_t sendSize, sent;
	(void)onOff;

	//
	//	TODO : need real access routine
	//

	sendSize = TMS_MakePacket( SEC_KEY_VALUE, TCON_CMD_ON, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::TCON_BrightnessMode( int32_t fd, uint8_t mode, uint8_t evtMode )
{
	FUNC_IN();
	int32_t sendSize, sent;
	(void)mode;
	(void)evtMode;

	//
	//	TODO : need real access routine
	//

	sendSize = TMS_MakePacket( SEC_KEY_VALUE, TCON_CMD_BR_CTRL, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::TCON_ElapsedTime( int32_t fd )
{
	FUNC_IN();
	int32_t sendSize, sent;
	uint32_t time = 1000;
	//
	//	TODO : need real access routine
	//

	sendSize = TMS_MakePacket( SEC_KEY_VALUE, TCON_CMD_BR_CTRL, &time, sizeof(time), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::TCON_Version( int32_t fd )
{
	FUNC_IN();
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, TCON_CMD_VERSION, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}

int32_t CNX_TMSServer::BAT_GetState(int32_t fd)
{
	FUNC_IN();
	uint32_t voltage = 1158;
	uint32_t currnet = 1500;
	uint32_t data[2];
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//
	data[0] = voltage;
	data[1] = currnet;

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, BAT_CMD_STATE, data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	HexDump( m_SendBuf, sent );
	FUNC_OUT();
	return 0;
}


int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t cmd, void *pPayload, int32_t payloadSize )
{
	(void)payloadSize;
	switch( cmd )
	{
	//	PFPGA Commands
	case PFPGA_CMD_STATE:
		PFPGA_GetState( fd );
		return 0;
	case PFPGA_CMD_SOURCE:
		PFPGA_SetSource( fd, (uint8_t*)pPayload );
		return 0;
	case PFPGA_CMD_VERSION:
		PFPGA_GetVersion( fd );
		return 0;

	//	TCON Commands
	case TCON_CMD_STATE:
	case TCON_CMD_OPEN:
	case TCON_CMD_SHORT:
	case TCON_CMD_DOOR:
	{
		uint8_t *index = (uint8_t *)pPayload;
		return TCON_Status( fd, cmd, index[0] );
	}
	case TCON_CMD_OPEN_POS:
	case TCON_CMD_SHORT_POS:
	{
		uint8_t *index = (uint8_t *)pPayload;
		return TCON_LedPosition( fd, cmd, index[0] );
	}
	case TCON_CMD_ON:
		return TCON_LedOn( fd, ((uint8_t*)pPayload)[0] );

	case TCON_CMD_BR_CTRL:
	{
		uint8_t *mode = (uint8_t *)pPayload;
		return TCON_BrightnessMode( fd, mode[0], mode[1] );
	}
	case TCON_CMD_ELAPSED_TIME:
	{
		return TCON_ElapsedTime( fd );
	}
	case TCON_CMD_VERSION:
		return TCON_Version( fd );

	//	BAT Commands
	case BAT_CMD_STATE:
		return BAT_GetState( fd );

	default:
		return -1;
	}
	return 0;
}


//
//	For Singleton
//
CNX_TMSServer* CNX_TMSServer::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_TMSServer();
	}
	return (CNX_TMSServer*)m_psInstance;
}

void CNX_TMSServer::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}



//
//	External APIs
//
int32_t NX_TMSServerStart()
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	return hTms->StartServer();
}

void NX_TMSServerStop()
{
	CNX_TMSServer *hTms = CNX_TMSServer::GetInstance();
	hTms->StopServer();
}
