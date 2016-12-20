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
	int32_t TCON_Status( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedShortNum( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_LedShortPos( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t index, uint8_t funcIndex, uint8_t patternIndex );
	int32_t TCON_MasteringRead( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg );
	int32_t TCON_MasteringWrite( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg, uint8_t msb, uint8_t lsb );
	int32_t TCON_Quality( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg, uint8_t msb, uint8_t lsb );
	int32_t	TCON_TargetGamma( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size );
	int32_t	TCON_DeviceGamma( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size );
	int32_t TCON_DotCorrection( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size );
	int32_t TCON_InputSource( int32_t fd, uint32_t cmd, uint32_t index, uint8_t resolIndex, uint8_t srcIndex );
	int32_t TCON_ElapsedTime( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_Version( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_Multi( int32_t fd, uint32_t cmd, uint8_t index, uint8_t data );

	//	PFPGA Commands
	int32_t PFPGA_Status( int32_t fd );
	int32_t PFPGA_Source( int32_t fd, uint8_t *index );
	int32_t PFPGA_Version( int32_t fd) ;

	//	Battery Commands
	int32_t BAT_Status( int32_t fd );

	//	IMB Commands
	int32_t IMB_Status( int32_t fd );
	int32_t IMB_Version( int32_t fd );

	//	IPC Commands
	int32_t IPC_Version( int32_t fd );

	int32_t ProcessCommand( int32_t clientSocket, uint32_t cmd, void *pPayload, int32_t payloadSize );

private:
	//	Private Member Variables
	enum { MAX_LOD_MODULE = 12 };

	bool		m_IsRunning;
	bool		m_ExitLoop;
	pthread_t	m_hThread;

	int32_t		m_hSocket;
	uint8_t		m_PayloadBuf[MAX_PAYLOAD_SIZE];
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE+12];

	int32_t		(*m_pTestPatternFunc[5])( CNX_I2C*, uint8_t, uint8_t );

private:
	//	For Singletone
	static CNX_IPCServer	*m_psInstance;
};

CNX_IPCServer* CNX_IPCServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
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
	m_pTestPatternFunc[0] = &TestPatternColorBar;
	m_pTestPatternFunc[1] = &TestPatternFullScreenColor;
	m_pTestPatternFunc[2] = &TestPatternGrayScale;
	m_pTestPatternFunc[3] = &TestPatternDot;
	m_pTestPatternFunc[4] = &TestPatternDiagonal;
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
int32_t CNX_IPCServer::TCON_Status( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 1 );
#else
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
int32_t CNX_IPCServer::TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 2 );
#else
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

	if( 0 > i2c.Write( slave, TCON_REG_CHECK_DOOR_READ, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_DOOR_READ, 0x0000 );
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
int32_t CNX_IPCServer::TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
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

	result = 0;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, snedSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
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

	result = 0;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;
	int32_t iReadData = 0;
	int32_t iErrorNum = 0;

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
int32_t CNX_IPCServer::TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;
	int32_t iCoordinateX = 0, iCoordinateY = 0;
	int32_t iErrorOutReady;

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
int32_t CNX_IPCServer::TCON_LedShortNum( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

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
	printf("index(%d)\n", slave);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedShortPos( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

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
	printf("index(%d)\n", slave);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t index, uint8_t funcIndex, uint8_t patternIndex )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( TCON_CMD_PATTERN_RUN == cmd )
	{
		if( 0 > i2c.Write( slave, TCON_REG_XYZ_TO_RGB, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_XYZ_TO_RGB, 0x0001 );
			goto ERROR_TCON;
		}

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
		if( 0 > i2c.Write( slave, TCON_REG_XYZ_TO_RGB, 0x0000 ) )
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

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_MasteringRead( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg )
{
	uint8_t result[2] = { 0xFF, 0xFF };
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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
int32_t CNX_IPCServer::TCON_MasteringWrite( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg, uint8_t msb, uint8_t lsb )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Quality( int32_t fd, uint32_t cmd, uint8_t index, uint8_t reg, uint8_t msb, uint8_t lsb )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TargetGamma( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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
			((int16_t)(data[i * 3 + 1] << 5) & 0xE000) |
			((int16_t)(data[i * 3 + 2] << 3) & 0x1FE0) |
			((int16_t)(data[i * 3 + 3] >> 5) & 0x0007);

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0007);
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_TGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_TGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DeviceGamma( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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
			((int16_t)(data[i * 3 + 1] << 5) & 0xE000) |
			((int16_t)(data[i * 3 + 2] << 3) & 0x1FE0) |
			((int16_t)(data[i * 3 + 3] >> 5) & 0x0007);

		pLsbData[i] = ((int16_t)(data[i * 3 + 3]) & 0x0007);
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pLsbData + iDataSize / 4 * i, iDataSize / 4) )
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_DGAM_WR_SEL, wrSel | 0x0001) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DGAM_WR_SEL, wrSel );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, burstSel) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, burstSel );
		goto ERROR_TCON;
	}

#if I2C_SEPARATE_BURST
	for( int32_t i = 0; i < 4; i++ )
	{
		if( 0 > i2c.Write( slave, dataReg, pMsbData + iDataSize / 4 * i, iDataSize / 4) )
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

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DotCorrection( int32_t fd, uint32_t cmd, uint32_t index, uint8_t *data, int32_t size )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, flashSel >> 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, flashSel >> 1 );
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

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0001 ) )
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

#if I2C_SEPARATE_BURST
	// printf("0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
	// 	pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7] );

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
		if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00, opData[i] ) )
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

	result = 0x01;

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pData );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_InputSource( int32_t fd, uint32_t cmd, uint32_t index, uint8_t resolIndex, uint8_t srcIndex )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Not Implementaion. ( i2c-%d, %d, %d, %d )\n", port, slave, resolIndex, srcIndex );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_ElapsedTime( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

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
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion. ( i2c-%d, %d )\n", port, slave );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion. ( i2c-%d, %d )\n", port, slave );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Version( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

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
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion. ( i2c-%d, %d )\n", port, slave );

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Multi( int32_t fd, uint32_t cmd, uint8_t index, uint8_t data )
{
	int32_t sendSize;

	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

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
	int32_t iReadData = 0x0000;
	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_MULTI )) )
	{
		NxDbgMsg( NX_DBG_ERR, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_MULTI );
	}

	NxDbgMsg( NX_DBG_VBS, "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
		(iReadData == data) ? "Success" : "Fail",
		port, i, TCON_REG_MULTI, data, iReadData );

	printf( "%s.( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x, read: 0x%04x )\n",
		(iReadData == data) ? "Success" : "Fail",
		port, i, TCON_REG_MULTI, data, iReadData );
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	PFPGA Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Status( int32_t fd )
{
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

	if( 0 > (iReadData = i2c.Read( slave, TCON_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_CHECK_STATUS );
		goto ERROR_PFPGA;
	}

	result = ((PFPGA_VAL_CHECK_STATUS == iReadData) ? 1 : 0);
#endif

	printf("port(%d), slave(%02x), reg(%02x), data(%d)\n", port, slave, PFPGA_VAL_CHECK_STATUS, iReadData);

ERROR_PFPGA:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, PFPGA_REG_CHECK_STATUS, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Source( int32_t fd, uint8_t * /*index*/ )
{
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_SOURCE, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Version( int32_t fd )
{
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_VERSION, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
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
int32_t CNX_IPCServer::BAT_Status( int32_t fd )
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

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, BAT_CMD_STATUS, data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
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
int32_t CNX_IPCServer::IMB_Status( int32_t fd )
{
	uint8_t state[1] = {1};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, IMB_CMD_STATUS, state, 1, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::IMB_Version( int32_t fd )
{
	uint8_t version[4] = {0, 1, 10, 20};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, IMB_CMD_VERSION, version, sizeof(version), m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::IPC_Version( int32_t fd )
{
	uint8_t version[32];
	int32_t sendSize;

	sprintf( (char*)version, "%s, %s", __TIME__, __DATE__ );

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, IPC_CMD_VERSION_SERVER, version, strlen((const char*)version), m_SendBuf, sizeof(m_SendBuf) );
	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::ProcessCommand( int32_t fd, uint32_t cmd, void *pPayload, int32_t payloadSize )
{
	uint8_t *pData = (uint8_t*)pPayload;
	switch( cmd )
	{
	//	TCON Commands
	case TCON_CMD_STATUS:
		return TCON_Status( fd, cmd, pData[0] );

	case TCON_CMD_DOOR_STATUS:
		return TCON_DoorStatus( fd, cmd, pData[0] );

	case TCON_CMD_MODE_NORMAL:
		return TCON_LedModeNormal( fd, cmd, pData[0] );

	case TCON_CMD_MODE_LOD:
		return TCON_LedModeLod( fd, cmd, pData[0] );

	case TCON_CMD_OPEN_NUM:
		return TCON_LedOpenNum( fd, cmd, pData[0] );

	case TCON_CMD_OPEN_POS:
		return TCON_LedOpenPos( fd, cmd, pData[0] );

	case TCON_CMD_SHORT_NUM:
		return TCON_LedShortNum( fd, cmd, pData[0] );

	case TCON_CMD_SHORT_POS:
		return TCON_LedShortPos( fd, cmd, pData[0] );

	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:
		return TCON_TestPattern( fd, cmd, pData[0], pData[1], pData[2] );

	case TCON_CMD_MASTERING_RD:
		return TCON_MasteringRead( fd, cmd, pData[0], pData[1] );

	case TCON_CMD_MASTERING_WR:
		return TCON_MasteringWrite( fd, cmd, pData[0], pData[1], pData[2], pData[3] );

	case TCON_CMD_QUALITY:
		return TCON_Quality( fd, cmd, pData[0], pData[1], pData[2], pData[3] );

	case TCON_CMD_TGAM_R:
	case TCON_CMD_TGAM_G:
	case TCON_CMD_TGAM_B:
		return TCON_TargetGamma( fd, cmd, pData[0], pData + 1, payloadSize );

	case TCON_CMD_DGAM_R:
	case TCON_CMD_DGAM_G:
	case TCON_CMD_DGAM_B:
		return TCON_DeviceGamma( fd, cmd, pData[0], pData + 1, payloadSize );

	case TCON_CMD_DOT_CORRECTION:
		return TCON_DotCorrection( fd, cmd, pData[0], pData + 1, payloadSize );

	case TCON_CMD_ELAPSED_TIME:
		return TCON_ElapsedTime( fd, cmd, pData[0] );

	case TCON_CMD_ACCUMULATE_TIME:
		return TCON_AccumulateTime( fd, cmd, pData[0] );

	case TCON_CMD_VERSION:
		return TCON_Version( fd, cmd, pData[0] );

	case TCON_CMD_MULTI:
		return TCON_Multi( fd, cmd, pData[0], pData[1] );

	//	PFPGA Commands
	case PFPGA_CMD_STATUS:
		return PFPGA_Status( fd );

	case PFPGA_CMD_SOURCE:
		return PFPGA_Source( fd, (uint8_t*)pPayload );

	case PFPGA_CMD_VERSION:
		return PFPGA_Version( fd );

	//	BAT Commands
	case BAT_CMD_STATUS:
		return BAT_Status( fd );

	//	IMB Commands
	case IMB_CMD_STATUS:
		return IMB_Status( fd );

	//	IPC Commands
	case IPC_CMD_VERSION_SERVER:
		return IPC_Version( fd );

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
static int32_t TestPatternColorBar( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternData[][8] = {
		//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		{	1,		0,		1024,	0,		1080,	4095,	4095,	4096	},	// 6 Color Bar
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

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
		{	10,		0,		1024,	0,		1080,	4095,	4095,	4095	},	//	White	100%
		{	10,		0,		1024,	0,		1080,	3685,	3685,	3685	},	//	Gray	90%
		{	10,		0,		1024,	0,		1080,	3276,	2948,	2948	},	//	Gray	80%
		{	10,		0,		1024,	0,		1080,	2866,	2866,	2866	},	//	Gray	70%
		{	10,		0,		1024,	0,		1080,	2457,	2457,	2457	},	//	Gray	60%
		{	10,		0,		1024,	0,		1080,	2047,	2047,	2047	},	//	Gray	50%
		{	10,		0,		1024,	0,		1080,	1638,	1638,	1638	},	//	Gray	40%
		{	10,		0,		1024,	0,		1080,	1228,	1228,	1228	},	//	Gray	30%
		{	10,		0,		1024,	0,		1080,	819,	819,	819		},	//	Gray	20%
		{	10,		0,		1024,	0,		1080,	409,	409,	409		},	//	Gray	1c0%
		{	10,		0,		1024,	0,		1080,	0,		0,		0		},	//	Black	0%
		{	10,		0,		1024,	0,		1080,	4095,	0,		0		},	//	Red		100%
		{	10,		0,		1024,	0,		1080,	0,		4095,	0		},	//	Green	100%
		{	10,		0,		1024,	0,		1080,	0,		0,		4095	},	//	Blue	100%
		{	10,		0,		1024,	0,		1080,	4095,	0,		4095	},	//	Magenta	100%
		{	10,		0,		1024,	0,		1080,	0,		4095,	4095	},	//	Cyan	100%
		{	10,		0,		1024,	0,		1080,	4095,	4095,	0		},	//	Yellow 	00%
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

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
		{	2,		0,		1024,	0,		1080	},							//	Gray 16-Step
		{	3,		0,		1024,	0,		1080	},							//	Gray 32-Step
		{	4,		0,		1024,	0,		1080	},							//	Gray 64-Step
		{	5,		0,		1024,	0,		1080	},							//	Gray 128-Step
		{	6,		0,		1024,	0,		1080	},							//	Gray 256-Step
		{	7,		0,		1024,	0,		1080	},							//	Gray 512-Step
		{	9,		7,		1024,	0,		1080	},							//	Gray 2048-Step
		{	9,		1,		1024,	0,		1080	},							//	Red 2048-Step
		{	9,		2,		1024,	0,		1080	},							//	Green 2048-Step
		{	9,		4,		1024,	0,		1080	},							//	Blue 2048-Step
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

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
		{	17,		0,		1024,	0,		1080,	4095,	4095,	4095	},	//	Dot Pattern 1x1
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

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
		{	104,	0,		1024,	0,		1080,	65535,	65535,	65535	},	//	Right-Down
		{	105,	0,		1024,	0,		1080,	65535,	65535,	65535	},	//	Right-Up
	};

	const uint16_t *pData = patternData[patternIndex];
	int32_t iDataNum = (int32_t)(sizeof(patternData[0]) / sizeof(patternData[0][0]));

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
