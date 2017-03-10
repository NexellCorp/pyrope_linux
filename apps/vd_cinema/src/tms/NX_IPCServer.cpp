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
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_I2C.h>
#include <NX_I2CRegister.h>
#include <SockUtils.h>
#include <tms_protocol.h>

#include <NX_IPCCommand.h>
#include <NX_IPCServer.h>
#include <CNX_BaseClass.h>

#define NX_DTAG	"[IPC Server]"
#include <NX_DbgMsg.h>

#define FAKE_DATA			0

#define I2C_DEBUG			0
#define I2C_SEPARATE_BURST	1

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

protected:
	//	Implementation CNX_Thread pure virtual function
	virtual void ThreadProc();

private:
	//	Local Socket
	int32_t WaitClient();
	int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size);

	//	TCON Commands
	int32_t TCON_Init( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LvdsStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_BootingStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedShortNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedShortPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_MasteringRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_MasteringWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_Quality( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t	TCON_TargetGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t	TCON_DeviceGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_DotCorrection( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_WriteConfig( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_InputSource( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_OptionalData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_Multi( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	PFPGA Commands
	int32_t PFPGA_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_Mute( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_UniformityEnableRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_UniformityEnableWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_UniformityData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_WriteConfig( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_Source( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	Battery Commands
	int32_t BAT_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	IMB Commands
	int32_t IMB_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t IMB_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	IPC Commands
	int32_t IPC_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	int32_t ProcessCommand( int32_t clientSocket, uint32_t cmd, void *pPayload, int32_t payloadSize );

private:
	//	Private Member Variables
	enum { MAX_LOD_MODULE = 12 };

	bool		m_IsRunning;
	bool		m_ExitLoop;
	pthread_t	m_hThread;

	int32_t		m_hSocket;
	//	Key(4bytes) + Length(2bytes) + Command(2bytes) + Payload(MAX 65533bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 8];
	uint8_t		m_ReceiveBuf[MAX_PAYLOAD_SIZE + 8];

	int32_t		(*m_pTestPatternFunc[6])( CNX_I2C*, uint8_t, uint8_t );

private:
	//	For Singletone
	static CNX_IPCServer	*m_psInstance;
};

CNX_IPCServer* CNX_IPCServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
static int32_t TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );

//------------------------------------------------------------------------------
CNX_IPCServer::CNX_IPCServer()
	: m_IsRunning (false)
	, m_ExitLoop (true)
	, m_hSocket (-1)
{
	m_pTestPatternFunc[0] = &TestPatternDci;
	m_pTestPatternFunc[1] = &TestPatternColorBar;
	m_pTestPatternFunc[2] = &TestPatternFullScreenColor;
	m_pTestPatternFunc[3] = &TestPatternGrayScale;
	m_pTestPatternFunc[4] = &TestPatternDot;
	m_pTestPatternFunc[5] = &TestPatternDiagonal;
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

	int32_t svrSocket = LS_Open( IPC_SERVER_FILE );
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
void CNX_IPCServer::ThreadProc()
{
	int32_t readSize;
	uint32_t key=0, cmd=0, len;
	uint8_t *pPayload;
	int32_t payloadSize;

	while( !m_ExitLoop )
	{
		uint8_t *pBuf = m_ReceiveBuf;
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
		if( len+8 > sizeof(m_ReceiveBuf) )
		{
			NxErrMsg("Error : Data size\n");
			goto ERROR;
		}

		//	Read all data
		readSize = ReadData ( clientSocket, pBuf+2, len );
		if( readSize <= 0 )
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

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::ReadData(int32_t fd, uint8_t *pBuf, int32_t size)
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
	} while(size > 0);
	return totalSize;
}


//------------------------------------------------------------------------------
//
//	TCON Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Init( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA1, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA1, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_SCAN_DATA2, 0x0025 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SCAN_DATA2, 0x0025 );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iWriteData, iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	iWriteData = (int16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( slave, TCON_REG_CHECK_STATUS, (uint16_t*)&iWriteData, 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_STATUS, iWriteData );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_STATUS );
		goto ERROR_TCON;
	}

	result = ((iWriteData == iReadData) ? 1 : 0);
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 2 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_DOOR_READ )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_DOOR_READ );
		goto ERROR_TCON;
	}

	result = (uint8_t)iReadData;

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 1 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_CLOSE_DOOR, 0 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CLOSE_DOOR, 0 );
		goto ERROR_TCON;
	}
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LvdsStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_LVDS_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LVDS_STATUS );
		goto ERROR_TCON;
	}

	result = (iReadData == 1080 || iReadData == 2160) ? 1 : 0;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_BootingStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 1 );
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_BOOTING_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_BOOTING_STATUS );
		goto ERROR_TCON;
	}

	result = (iReadData == 0x07E1 ) ? 1 : 0;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LOD_REMOVAL_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LOD_REMOVAL_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_PATTERN, 0x0000 );
		goto ERROR_TCON;
	}

	result = 0x01;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, snedSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	const uint16_t pattern[] = {
		10,		0,		1024,	0,		2160,	4095,	4095,	4095
	};

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(pattern) / sizeof(pattern[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_PATTERN + i, pattern[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_PATTERN + i, pattern[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LOD_REMOVAL_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );

		goto ERROR_TCON;
	}

	usleep( 2500000 );	// delay during 2.034sec over ( LOD Scan Time )

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	result = 0x01;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;
	int32_t iReadData = 0;
	int32_t iErrorNum = 0;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_LOD_100_CHK_DONE ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_LOD_100_CHK_DONE );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_ERROR_NUM_OVR ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_NUM_OVR );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( slave, TCON_REG_CURRENT_PERCENT) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CURRENT_PERCENT );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		iReadData = i2c.Read( slave, TCON_REG_ERROR_NUM_M1 + i );
		if( 0 > iReadData )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_ERROR_NUM_M1 + i );
			goto ERROR_TCON;
		}

		iErrorNum += iReadData;
	}

	result[0] = (uint8_t)(iErrorNum >> 24) & 0xFF;
	result[1] = (uint8_t)(iErrorNum >> 16) & 0xFF;
	result[2] = (uint8_t)(iErrorNum >>  8) & 0xFF;
	result[3] = (uint8_t)(iErrorNum >>  0) & 0xFF;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;
	int32_t iCoordinateX = 0, iCoordinateY = 0;
	int32_t iErrorOutReady;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	usleep( 50 );

	iErrorOutReady = i2c.Read( slave, TCON_REG_ERROR_OUT_RDY );
	if( iErrorOutReady )
	{
		iCoordinateX = i2c.Read( slave, TCON_REG_X_COORDINATE );
		iCoordinateY = i2c.Read( slave, TCON_REG_Y_COORDINATE );

		result[0] = (uint8_t)((iCoordinateX >> 8 ) & 0xFF);
		result[1] = (uint8_t)((iCoordinateX >> 0 ) & 0xFF);
		result[2] = (uint8_t)((iCoordinateY >> 8 ) & 0xFF);
		result[3] = (uint8_t)((iCoordinateY >> 0 ) & 0xFF);
	}

	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0001 );
	usleep( 50000 );
	i2c.Write( slave, TCON_REG_ERROR_OUT_CLK, 0x0000 );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedShortNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("port( %d ), slave( %d )\n", port, slave);

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedShortPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("port( %d ), slave( %d )\n", port, slave);

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : function index, pBuf[2] : pattern index
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t funcIndex = pBuf[1];
	uint8_t patternIndex = pBuf[2];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( TCON_CMD_PATTERN_RUN == cmd )
	{
		if( funcIndex != sizeof(m_pTestPatternFunc)/sizeof(m_pTestPatternFunc[0]) )
		{
			m_pTestPatternFunc[funcIndex](&i2c, index, patternIndex);
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0001 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0001) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0001, iReadData );
			}
#endif
		}
	}
	else
	{
		if( 0 > i2c.Write( slave, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_XYZ_TO_RGB, 0x0000 );
			goto ERROR_TCON;
		}

		if( funcIndex != sizeof(m_pTestPatternFunc)/sizeof(m_pTestPatternFunc[0]) )
		{
			if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_PATTERN, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_PATTERN )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_PATTERN );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_PATTERN, 0x0000, iReadData );
			}
#endif
		}
		else
		{
			if( 0 > i2c.Write( slave, TCON_REG_CABINET_ID, 0x0000 ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_CABINET_ID, 0x0000 );
				goto ERROR_TCON;
			}

#if I2C_DEBUG
			for( int32_t i = 0x16; i < 0x80; i++ )
			{
				int32_t ret = i2c.Read( i, TCON_REG_CHECK_STATUS );
				if( 0 > ret )
					continue;

				int32_t iReadData = 0x0000;
				if( 0 > (iReadData = i2c.Read( i, TCON_REG_CABINET_ID )) )
				{
					NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, i, TCON_REG_CABINET_ID );
				}

				NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );

				printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
					(iReadData == 0x0000) ? "Success" : "Fail",
					port, i, TCON_REG_CABINET_ID, 0x0000, iReadData );
			}
#endif
		}
	}

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_MasteringRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : register
	//

	uint8_t result[2] = { 0xFF, 0xFF };
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t reg		= pBuf[1];

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, reg )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, reg );
		goto ERROR_TCON;
	}

	result[0] = (uint8_t)((iReadData >> 8 ) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 0 ) & 0xFF);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_MasteringWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : register, pBuf[2] : msb data, pBuf[3] : lsb data
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t reg		= pBuf[1];
	uint8_t msb		= pBuf[2];
	uint8_t lsb		= pBuf[3];
	int16_t inValue = ((int16_t)(msb << 8) & 0xFF00) + (int16_t)lsb;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, reg, inValue ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, reg, inValue );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Quality( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : msg reg, pBuf[2] : lsb reg, pBuf[3] : msb data, pBuf[4] : lsb data
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];
	uint8_t msbData	= pBuf[3];
	uint8_t lsbData	= pBuf[4];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TargetGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	//
	//	pBuf[0] : index, pBuf[1] .. : data ( num of data : iSize - 1 )
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pBuf + 1;
	int32_t size	= iSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_TGAM_R_WDATA + (cmd - TCON_CMD_TGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0001 << (cmd - TCON_CMD_TGAM_R);

	int32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[%d] [ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	i, oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
	}
#else
	if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, dataReg, iDataSize );
		goto ERROR_TCON;
	}
#endif
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize / 4 );
			goto ERROR_TCON;
		}
	}
#else
	if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, dataReg, iDataSize );
		goto ERROR_TCON;
	}
#endif
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DeviceGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	//
	//	pBuf[0] : index, pBuf[1] .. : data ( num of data : iSize - 1 )
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pBuf + 1;
	int32_t size	= iSize - 1;

	uint16_t dataReg;
	uint16_t wrSel, burstSel;

	dataReg = TCON_REG_DGAM_R_WDATA + (cmd - TCON_CMD_DGAM_R) * 2;
	wrSel	= data[0] * 2;
	burstSel= 0x0008 << (cmd - TCON_CMD_DGAM_R);

	int32_t iDataSize = (size - 1) / 3;
	uint16_t* pMsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );
	uint16_t* pLsbData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		// pMsbData[i] = (int16_t)data[i * 3 + 1];
		// pLsbData[i] = ((int16_t)(data[i * 3 + 2] << 8) & 0xFF00) + (int16_t)data[i * 3 + 3];

		pMsbData[i]	=
			(((int16_t)data[i * 3 + 1] << 14) & 0xE000) |	// 1100 0000 0000 0000
			(((int16_t)data[i * 3 + 2] <<  6) & 0x3FC0) |	// 0011 1111 1100 0000
			(((int16_t)data[i * 3 + 3] >>  2) & 0x003F);	// 0000 0000 0011 1111

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0003);// 0000 0000 0000 0011

		// int32_t oriData =
		// 	(((int32_t)data[i * 3 + 1] << 16) & 0x00FF0000) |
		// 	(((int32_t)data[i * 3 + 2] <<  8) & 0x0000FF00) |
		// 	(((int32_t)data[i * 3 + 3] <<  0) & 0x000000FF);

		// printf("[ %d ( 0x%06x ) : %02x %02x %02x ] msb - 0x%04x, lsb - 0x%04x\n",
		// 	oriData, oriData, data[i * 3 + 1], data[i * 3 + 2], data[i * 3 + 3],
		// 	pMsbData[i], pLsbData[i] );
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize );
			goto ERROR_TCON;
		}
	}
#else
	if( 0 > i2c.Write( slave, dataReg, pLsbData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, dataReg, iDataSize );
		goto ERROR_TCON;
	}
#endif
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, dataReg, iDataSize / 4 );
			goto ERROR_TCON;
		}
	}
#else
	if( 0 > i2c.Write( slave, dataReg, pMsbData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, dataReg, iDataSize );
		goto ERROR_TCON;
	}
#endif
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DotCorrection( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	//
	//	pBuf[0] : index, pBuf[1] .. : data ( num of data : iSize - 1 )
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t *data	= pBuf + 1;
	int32_t size	= iSize - 1;

	uint16_t flashSel = (uint16_t)data[0];
	int32_t iDataSize = (size-1) / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t ccData[9] = { 0x0000, };
	uint16_t opData[16] = { 0x0000, };

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(data[2 * i + 1] << 8) & 0xFF00) | ((int16_t)(data[2 * i + 2] << 0) & 0x00FF);
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	// 1. Read Screen Correction Data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel >> 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel >> 1 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_CC00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_CC00_READ + i );
			goto ERROR_TCON;
		}

		ccData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	2. Read Optional Data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		opData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	3. Flash Protection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	4. Erase Flash.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_CHIP_ERASE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(4000000);

	//
	//	5. Write Dot Correction Data.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0040) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0040 );
		goto ERROR_TCON;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 30; i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData + iDataSize / 30 * i, iDataSize / 30) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_WDATA, iDataSize / 30 );
			goto ERROR_TCON;
		}
	}
#else
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_WDATA, pData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_WDATA, iDataSize );
		goto ERROR_TCON;
	}
#endif

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	6. Write Screen Correction Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(ccData) / sizeof(ccData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_CC00 + i, ccData[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_CC00 + i, ccData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_CC_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_CC_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	7. Write Optional Data.
	//
	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[i] ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00 + i, opData[i] );
			goto ERROR_TCON;
		}
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	//
	//	8. Flash Protection On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pData );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_WriteConfig( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : msg reg, pBuf[2] : lsb reg, pBuf[3] : msb data, pBuf[4] : lsb data
	//

	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];
	uint8_t msbData	= pBuf[3];
	uint8_t lsbData	= pBuf[4];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_InputSource( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : reoslution index, pBuf[2] : source index
	//

	uint8_t	result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t resolIndex	= pBuf[1];
	uint8_t srcIndex	= pBuf[2];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Not Implementaion. ( i2c-%d, %d, %d, %d )\n", port, slave, resolIndex, srcIndex );
	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_OptionalData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : module
	//

	uint8_t result[32] = { 0xFF, };
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pBuf[1];

	uint16_t iOptionalData[16] = { 0x0000, };

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < (int32_t)(sizeof(iOptionalData) / sizeof(iOptionalData[0])); i++ )
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ + i );
			goto ERROR_TCON;
		}

		iOptionalData[i] = (uint16_t)(iReadData & 0x0000FFFF);
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}

	for(int32_t i = 0; i < (int32_t)(sizeof(iOptionalData) / sizeof(iOptionalData[0])); i++ )
	{
		result[i * 2 + 0] = (uint8_t)((iOptionalData[i] >> 8 ) & 0xFF);
		result[i * 2 + 1] = (uint8_t)((iOptionalData[i] >> 0 ) & 0xFF);
	}

	//
	//	Print Debug Message
	//
	// printf(">> index %d, module %d\n", index, module);
	// printf("code(id)              : 0x%04x%04x%04x%04x\n", iOptionalData[3], iOptionalData[2], iOptionalData[1], iOptionalData[0] );
	// printf("module - date         : %04d.%02d.%02d\n", (iOptionalData[4] >> 9) & 0x7F, (iOptionalData[4] >> 5) & 0x0F, (iOptionalData[4] >> 0) & 0x1F );
	// printf("module - location     : %c%c%c%c\n", (iOptionalData[6] >> 8) & 0xFF, (iOptionalData[6] >> 0) & 0xFF, (iOptionalData[5] >> 8) & 0xFF, (iOptionalData[5] >> 0) & 0xFF );
	// printf("led - manufacturer    : %c%c%c\n", (iOptionalData[8] >> 0) & 0xFF, (iOptionalData[7] >> 8) & 0xFF, (iOptionalData[7] >> 0) & 0xFF );
	// printf("correction - date     : %04d.%02d.%02d\n", (iOptionalData[9] >> 1) & 0x7F, ((iOptionalData[9] << 3) & 0x80) | ((iOptionalData[8] << 13) & 0x07), (iOptionalData[8] >> 8) & 0x1F );
	// printf("correction - location : %c%c%c%c\n", (iOptionalData[11] >> 0) & 0xFF, (iOptionalData[10] >> 8) & 0xFF, (iOptionalData[10] >> 0) & 0xFF, (iOptionalData[9] >> 8) & 0xFF );
	// printf("hour of use           : %lld\n", (((uint64_t)(iOptionalData[15] << 9)) & 0x00FFFFFF) | ((uint64_t)(iOptionalData[14] >> 7) & 0x000001FF) );
	// printf("parity                : %d\n", (iOptionalData[15] >> 15) & 0x01 );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Multi( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : index, pBuf[1] : data
	//

	uint8_t	result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t data	= pBuf[1];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_MULTI, data ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MULTI, data );
		goto ERROR_TCON;
	}

#if I2C_DEBUG
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_MULTI )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_MULTI );
		}

		NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
			(iReadData == data) ? "Success" : "Fail",
			port, slave, TCON_REG_MULTI, data, iReadData );

		printf( "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
			(iReadData == data) ? "Success" : "Fail",
			port, slave, TCON_REG_MULTI, data, iReadData );
	}
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	PFPGA Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Status( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	//
	//	pBuf : not used
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_CHECK_STATUS );
		goto ERROR_PFPGA;
	}

	result = ((PFPGA_VAL_CHECK_STATUS == iReadData) ? 1 : 0);
#endif

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Mute( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : data
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t data	= pBuf[0];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_MUTE, data ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_MUTE, data );
		goto ERROR_PFPGA;
	}

	result = 0x01;
#endif

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_UniformityEnableRead( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	//
	//	pBuf : not used
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_NUC_EN )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_NUC_EN );
		goto ERROR_PFPGA;
	}

	result = (uint8_t)iReadData;
#endif

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_UniformityEnableWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : data
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0 , 1 );
#else
	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t data	= pBuf[0];

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_NUC_EN, data ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_NUC_EN, data );
		goto ERROR_PFPGA;
	}

	result = 0x01;
#endif

#if I2C_DEBUG
	{
		int32_t iReadData = 0x0000;
		if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_NUC_EN )) )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, PFPGA_REG_NUC_EN );
		}

		NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
			(iReadData == data) ? "Success" : "Fail",
			port, slave, PFPGA_REG_NUC_EN, data, iReadData );

		printf( "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
			(iReadData == data) ? "Success" : "Fail",
			port, slave, PFPGA_REG_NUC_EN, data, iReadData );
	}
#endif

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_UniformityData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	//
	//	pBuf[0] - : data
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t *data	= pBuf;
	int32_t size	= iSize;

	int32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(data[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(data[2 * i + 1] << 0) & 0x00FF);
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0080) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0080 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData + iDataSize / 4 * i, iDataSize / 4) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
				port, slave, PFPGA_REG_NUC_WDATA, iDataSize / 4 );
			goto ERROR_PFPGA;
		}
	}
#else
	if( 0 > i2c.Write( slave, PFPGA_REG_NUC_WDATA, pData, iDataSize ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, size: 0x%04x )\n",
			port, slave, PFPGA_REG_NUC_WDATA, iDataSize );
		goto ERROR_PFPGA;
	}
#endif

	if( 0 > i2c.Write( slave, PFPGA_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, PFPGA_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_PFPGA;
	}
	usleep(100000);

	result = 0x01;

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_WriteConfig( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t /*iSize*/ )
{
	//
	//	pBuf[0] : msg reg, pBuf[1] : lsb reg, pBuf[2] : msb data, pBuf[3] : lsb data
	//
	uint8_t	result	= 0xFF;
	int32_t sendSize;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pBuf[0];
	uint8_t lsbReg	= pBuf[1];
	uint8_t msbData	= pBuf[2];
	uint8_t lsbData	= pBuf[3];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	uint16_t inData	= ((int16_t)(msbData << 8) & 0xFF00) + (int16_t)lsbData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > i2c.Write( slave, inReg, inData ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_INFO, "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, inData );

	result = 0x01;

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Source( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Version( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}


//------------------------------------------------------------------------------
//
//	Battery Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::BAT_Status( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	uint32_t voltage = 1158;
	uint32_t currnet = 1500;
	uint32_t data[2];
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//
	data[0] = voltage;
	data[1] = currnet;

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}


//------------------------------------------------------------------------------
//
//	IMB Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::IMB_Status( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	uint8_t state[1] = {1};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, state, 1, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::IMB_Version( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::IPC_Version( int32_t fd, uint32_t cmd, uint8_t */*pBuf*/, int32_t /*iSize*/ )
{
	uint8_t version[32];
	int32_t sendSize;

	sprintf( (char*)version, "%s, %s", __TIME__, __DATE__ );

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, version, strlen((const char*)version), m_SendBuf, sizeof(m_SendBuf) );
	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::ProcessCommand( int32_t fd, uint32_t cmd, void *pPayload, int32_t payloadSize )
{
	switch( cmd )
	{
	//
	//	TCON Commands
	//
	case TCON_CMD_INIT:
		return TCON_Init( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_STATUS:
		return TCON_Status( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_DOOR_STATUS:
		return TCON_DoorStatus( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_LVDS_STATUS:
		return TCON_LvdsStatus( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_BOOTING_STATUS:
		return TCON_BootingStatus( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_MODE_NORMAL:
		return TCON_LedModeNormal( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_MODE_LOD:
		return TCON_LedModeLod( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_OPEN_NUM:
		return TCON_LedOpenNum( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_OPEN_POS:
		return TCON_LedOpenPos( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_SHORT_NUM:
		return TCON_LedShortNum( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_SHORT_POS:
		return TCON_LedShortPos( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:
		return TCON_TestPattern( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_MASTERING_RD:
		return TCON_MasteringRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_MASTERING_WR:
		return TCON_MasteringWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_QUALITY:
		return TCON_Quality( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_TGAM_R:
	case TCON_CMD_TGAM_G:
	case TCON_CMD_TGAM_B:
		return TCON_TargetGamma( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_DGAM_R:
	case TCON_CMD_DGAM_G:
	case TCON_CMD_DGAM_B:
		return TCON_DeviceGamma( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_DOT_CORRECTION:
		return TCON_DotCorrection( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_WRITE_CONFIG:
		return TCON_WriteConfig( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:
	case TCON_CMD_VERSION:
		return TCON_OptionalData( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_MULTI:
		return TCON_Multi( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	PFPGA Commands
	//
	case PFPGA_CMD_STATUS:
		return PFPGA_Status( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_MUTE:
		return PFPGA_Mute( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_UNIFORMITY_RD:
		return PFPGA_UniformityEnableRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_UNIFORMITY_WR:
		return PFPGA_UniformityEnableWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_UNIFORMITY_DATA:
		return PFPGA_UniformityData( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_WRITE_CONFIG:
		return PFPGA_WriteConfig( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_SOURCE:
		return PFPGA_Source( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_VERSION:
		return PFPGA_Version( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	BAT Commands
	//
	case BAT_CMD_STATUS:
		return BAT_Status( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	IMB Commands
	//
	case IMB_CMD_STATUS:
		return IMB_Status( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	IPC Commands
	//
	case IPC_CMD_VERSION_SERVER:
		return IPC_Version( fd, cmd, (uint8_t*)pPayload, payloadSize );

	default:
		return -1;
	}

	return 0;
}


//------------------------------------------------------------------------------
//
//	Test Pattern Functions
//

//------------------------------------------------------------------------------
static int32_t TestPatternDci( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	2901,	2171,	100		},	//	Red-1
		{	10,		0,		1024,	0,		2160,	2417,	3493,	1222	},	//	Green-1
		{	10,		0,		1024,	0,		2160,	2014,	1416,	3816	},	//	Blue-1
		{	10,		0,		1024,	0,		2160,	2911,	3618,	3890	},	//	Cyan-1
		{	10,		0,		1024,	0,		2160,	3289,	2421,	3814	},	//	Magenta-1
		{	10,		0,		1024,	0,		2160,	3494,	3853,	1221	},	//	Yellow-1
		{	10,		0,		1024,	0,		2160,	2738,	2171,	1233	},	//	Red-2
		{	10,		0,		1024,	0,		2160,	2767,	3493,	2325	},	//	Green-2
		{	10,		0,		1024,	0,		2160,	1800,	1416,	3203	},	//	Blue-2
		{	10,		0,		1024,	0,		2160,	3085,	3590,	3756	},	//	Cyan-2
		{	10,		0,		1024,	0,		2160,	3062,	2421,	3497	},	//	Magenta-2
		{	10,		0,		1024,	0,		2160,	3461,	3777,	2065	},	//	Yellow-2
		{	10,		0,		1024,	0,		2160,	3883,	3960,	4092	},	//	White-1
		{	10,		0,		1024,	0,		2160,	3794,	3960,	3890	},	//	White-2
		{	10,		0,		1024,	0,		2160,	3893,	3960,	3838	},	//	White-3
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1024,	0,		2160,	4095,	4095,	4095	},	// 6 Color Bar
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	10,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	White	100%
		{	10,		0,		1024,	0,		2160,	3685,	3685,	3685	},	//	Gray	90%
		{	10,		0,		1024,	0,		2160,	3276,	2948,	2948	},	//	Gray	80%
		{	10,		0,		1024,	0,		2160,	2866,	2866,	2866	},	//	Gray	70%
		{	10,		0,		1024,	0,		2160,	2457,	2457,	2457	},	//	Gray	60%
		{	10,		0,		1024,	0,		2160,	2047,	2047,	2047	},	//	Gray	50%
		{	10,		0,		1024,	0,		2160,	1638,	1638,	1638	},	//	Gray	40%
		{	10,		0,		1024,	0,		2160,	1228,	1228,	1228	},	//	Gray	30%
		{	10,		0,		1024,	0,		2160,	819,	819,	819		},	//	Gray	20%
		{	10,		0,		1024,	0,		2160,	409,	409,	409		},	//	Gray	1c0%
		{	10,		0,		1024,	0,		2160,	0,		0,		0		},	//	Black	0%
		{	10,		0,		1024,	0,		2160,	4095,	0,		0		},	//	Red		100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	0		},	//	Green	100%
		{	10,		0,		1024,	0,		2160,	0,		0,		4095	},	//	Blue	100%
		{	10,		0,		1024,	0,		2160,	4095,	0,		4095	},	//	Magenta	100%
		{	10,		0,		1024,	0,		2160,	0,		4095,	4095	},	//	Cyan	100%
		{	10,		0,		1024,	0,		2160,	4095,	4095,	0		},	//	Yellow 	00%
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28
		{	2,		0,		1024,	0,		2160	},							//	Gray 16-Step
		{	3,		0,		1024,	0,		2160	},							//	Gray 32-Step
		{	4,		0,		1024,	0,		2160	},							//	Gray 64-Step
		{	5,		0,		1024,	0,		2160	},							//	Gray 128-Step
		{	6,		0,		1024,	0,		2160	},							//	Gray 256-Step
		{	7,		0,		1024,	0,		2160	},							//	Gray 512-Step
		{	9,		7,		1024,	0,		2160	},							//	Gray 2048-Step
		{	9,		1,		1024,	0,		2160	},							//	Red 2048-Step
		{	9,		2,		1024,	0,		2160	},							//	Green 2048-Step
		{	9,		4,		1024,	0,		2160	},							//	Blue 2048-Step
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	17,		0,		1024,	0,		2160,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	104,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1024,	0,		2160,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

	if( 0 > pI2c->Write( index & 0x7F, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
	{
		return -1;
	}

	for( int32_t i = 0; i < iDataNum; i++ )
	{
		if( 0 > pI2c->Write( index & 0x7F, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}
	}

#if I2C_DEBUG
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	for( int32_t i = 0x16; i < 0x80; i++ )
	{
		int32_t ret = pI2c->Read( i, TCON_REG_CHECK_STATUS );
		if( 0 > ret )
			continue;

		for( int32_t j = 0; j < iDataNum; j++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = pI2c->Read( i, TCON_REG_PATTERN + j )) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, i, TCON_REG_PATTERN + j );
			}

			NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );

			printf("%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
				(iReadData == pData[j]) ? "Success" : "Fail",
				port, i, TCON_REG_PATTERN + j, pData[j], iReadData );
		}
	}
#endif
	return 0;
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
