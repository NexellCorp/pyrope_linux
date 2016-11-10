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
#include <errno.h>

#include <CNX_I2C.h>
#include <NX_I2CRegister.h>
#include <SockUtils.h>
#include <tms_protocol.h>

#include <NX_TMSCommand.h>
#include <NX_TMSServer.h>
#include <CNX_BaseClass.h>

#define NX_DTAG	"[TMS Server]"
#include <NX_DbgMsg.h>

#define FAKE_DATA		0

//------------------------------------------------------------------------------
//
//	TMS Server APIs
//
class CNX_TMSServer : protected CNX_Thread
{
public:
	CNX_TMSServer();
	~CNX_TMSServer();

	static CNX_TMSServer* GetInstance();
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
	int32_t TCON_Mastering( int32_t fd, uint32_t cmd, uint8_t index, uint8_t item, uint8_t msb, uint8_t lsb );
	int32_t TCON_ElapsedTime( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t index );
	int32_t TCON_Version( int32_t fd, uint32_t cmd, uint8_t index );

	//	PFPGA Commands
	int32_t PFPGA_Status( int32_t fd );
	int32_t PFPGA_Source( int32_t fd, uint8_t *index );
	int32_t PFPGA_Version( int32_t fd) ;

	//	Battery Commands
	int32_t BAT_Status( int32_t fd );

	//	IMB Commands
	int32_t IMB_Status( int32_t fd );
	int32_t IMB_Version( int32_t fd );

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

	int32_t		(*m_pTestPatternFunc[16])( CNX_I2C*, uint8_t, uint8_t );

private:
	//	For Singletone
	static CNX_TMSServer	*m_psInstance;
};

CNX_TMSServer* CNX_TMSServer::m_psInstance = NULL;

//------------------------------------------------------------------------------
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );
static int32_t TestPatternCabinetNum( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex );

//------------------------------------------------------------------------------
CNX_TMSServer::CNX_TMSServer()
	: m_IsRunning (false)
	, m_ExitLoop (true)
	, m_hSocket (-1)
{
	m_pTestPatternFunc[0] = &TestPatternFullScreenColor;
	m_pTestPatternFunc[1] = &TestPatternGrayScale;
	m_pTestPatternFunc[2] = &TestPatternDot;
	m_pTestPatternFunc[3] = &TestPatternDiagonal;
	m_pTestPatternFunc[4] = &TestPatternCabinetNum;
}

//------------------------------------------------------------------------------
CNX_TMSServer::~CNX_TMSServer()
{
	StopServer();
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void CNX_TMSServer::StopServer()
{
	if( m_IsRunning )
	{
		m_ExitLoop = true;
		Stop();
	}
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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
	} while(size > 0);
	return totalSize;
}


//------------------------------------------------------------------------------
//
//	TCON Commands
//

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_Status( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 1 );
#else
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);
	int32_t iWriteData, iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	iWriteData = (int16_t)NX_GetRandomValue( 0x0000, 0x7FFF );
	if( 0 > i2c.Write( index, TCON_REG_CHECK_STATUS, (uint16_t*)&iWriteData, 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_CHECK_STATUS, iWriteData );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( index, TCON_REG_CHECK_STATUS )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_CHECK_STATUS );
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
int32_t CNX_TMSServer::TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = NX_GetRandomValue( 0, 2 );
#else
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( index, TCON_REG_CHECK_DOOR_READ )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_CHECK_DOOR_READ );
		goto ERROR_TCON;
	}

	result = (uint8_t)iReadData;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );
	
	return 0;	
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( index, TCON_REG_LOD_REMOVAL_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LOD_REMOVAL_EN, 0x0000 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( index, TCON_REG_LIVE_LOD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LIVE_LOD_EN, 0x0001 );
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
int32_t CNX_TMSServer::TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

#if FAKE_DATA
	result = 0;
#else
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( index, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LIVE_LOD_EN, 0x0000 );
		goto ERROR_TCON;			
	}

	if( 0 > i2c.Write( index, TCON_REG_LOD_REMOVAL_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LOD_REMOVAL_EN, 0x0001 );
		goto ERROR_TCON;
	}

	usleep( 2500000 );	// delay during 2.034sec over ( LOD Scan Time )
	result = 0;
#endif

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;
	int32_t iReadData = 0;
	int32_t iErrorNum = 0;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( index, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;		
	}

	if( 0 > i2c.Read( index, TCON_REG_LOD_100_CHK_DONE ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LOD_100_CHK_DONE );		
		goto ERROR_TCON;
	}
	
	if( 0 > i2c.Read( index, TCON_REG_ERROR_NUM_OVR ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_NUM_OVR );		
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( index, TCON_REG_CURRENT_PERCENT) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_CURRENT_PERCENT );		
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		iReadData = i2c.Read( index, TCON_REG_ERROR_NUM_M1 + i );
		if( 0 > iReadData )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
				TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_NUM_M1 + i );
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
int32_t CNX_TMSServer::TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t index )
{
	uint8_t *pResult = NULL;
	int32_t sendSize;
	int32_t iReadData = 0;
	int32_t iErrorNum = 0;
	int32_t iOffset = 0;
	int32_t iErrorNumModule[MAX_LOD_MODULE];
	
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( index, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;		
	}

	if( 0 > i2c.Read( index, TCON_REG_LOD_100_CHK_DONE ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_LOD_100_CHK_DONE );		
		goto ERROR_TCON;
	}
	
	if( 0 > i2c.Read( index, TCON_REG_ERROR_NUM_OVR ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_NUM_OVR );		
		goto ERROR_TCON;
	}

	if( 0 > i2c.Read( index, TCON_REG_CURRENT_PERCENT) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
			TCON_LEFT_I2C_PORT, index, TCON_REG_CURRENT_PERCENT );		
		goto ERROR_TCON;
	}

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		iReadData = i2c.Read( index, TCON_REG_ERROR_NUM_M1 + i );
		if( 0 > iReadData )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n", 
				TCON_LEFT_I2C_PORT, index, TCON_REG_ERROR_NUM_M1 + i );
			goto ERROR_TCON;
		}

		iErrorNum += iReadData;
		iErrorNumModule[i] = iReadData;
	}

	pResult = (uint8_t*)malloc( iErrorNum * 4 );
	memset( pResult, 0xFF, iErrorNum * 4 );

	for( int32_t i = 0; i < MAX_LOD_MODULE; i++ )
	{
		for( int32_t j = 0; j < (int32_t)iErrorNumModule[i]; j++ )
		{
			usleep( 50 );

			int32_t iErrorOutReady = i2c.Read( index, TCON_REG_ERROR_OUT_RDY );
			if( iErrorOutReady )
			{
				int32_t iCoordinateX = i2c.Read( index, TCON_REG_X_COORDINATE );
				int32_t iCoordinateY = i2c.Read( index, TCON_REG_Y_COORDINATE );

				printf("x( %d ), y( %d )\n", iCoordinateX, iCoordinateY );

				pResult[iOffset + 0] = (uint8_t)((iCoordinateX >> 8 ) & 0xFF);
				pResult[iOffset + 1] = (uint8_t)((iCoordinateX >> 0 ) & 0xFF);
				pResult[iOffset + 2] = (uint8_t)((iCoordinateY >> 8 ) & 0xFF);
				pResult[iOffset + 3] = (uint8_t)((iCoordinateY >> 0 ) & 0xFF);

				iOffset += 4;
			}

			i2c.Write( index, TCON_REG_ERROR_OUT_CLK, 0x0001 );
			usleep( 50000 );
			i2c.Write( index, TCON_REG_ERROR_OUT_CLK, 0x0000 );
		}
	}

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, pResult, iErrorNum * 4, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	free( pResult );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_LedShortNum( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("index(%d)\n", index);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_LedShortPos( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("index(%d)\n", index);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t index, uint8_t funcIndex, uint8_t patternIndex )
{
	int32_t sendSize;
	
	CNX_I2C i2c(TCON_LEFT_I2C_PORT);
	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	m_pTestPatternFunc[funcIndex](&i2c, index, patternIndex);

ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_Mastering( int32_t fd, uint32_t cmd, uint8_t index, uint8_t item, uint8_t msb, uint8_t lsb )
{
	uint8_t result = 0xFF;
	int32_t sendSize;

	int16_t inValue = ((int16_t)(msb << 8) & 0xFF00) + (int16_t)lsb;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}
	
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf( "index(%d), item(%d), value(%d)\n", index, item, inValue );

	// if( 0 > i2c.Write( index, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	// {
	// 	NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", 
	// 		TCON_LEFT_I2C_PORT, index, TCON_REG_LIVE_LOD_EN, 0x0000 );
	// 	goto ERROR_TCON;			
	// }

	result = 0;
	
ERROR_TCON:
	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_ElapsedTime( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("index(%d)\n", index);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("index(%d)\n", index);

ERROR_TCON:
	sendSize = TMS_MakePacket( SEC_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::TCON_Version( int32_t fd, uint32_t cmd, uint8_t index )
{
	int32_t sendSize;

	CNX_I2C i2c(TCON_LEFT_I2C_PORT);

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", TCON_LEFT_I2C_PORT);
		goto ERROR_TCON;
	}

	//
	//
	//
	NxDbgMsg( NX_DBG_INFO, "Not Implementaion.\n" );
	printf("index(%d)\n", index);

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
int32_t CNX_TMSServer::PFPGA_Status( int32_t fd )
{
	uint8_t state[1] = {1};
	int32_t sendSize, sent;

	//
	//	TODO : Need real access routine.
	//

	sendSize = TMS_MakePacket ( SEC_KEY_VALUE, PFPGA_CMD_STATUS, state, 1, m_SendBuf, sizeof(m_SendBuf) );
	sent = write( fd, m_SendBuf, sendSize );
	NxDbgMsg(NX_DBG_VBS, "Write Reply = %d/%d\n", sent, sendSize);
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::PFPGA_Source( int32_t fd, uint8_t * /*index*/ )
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
int32_t CNX_TMSServer::PFPGA_Version( int32_t fd )
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
int32_t CNX_TMSServer::BAT_Status( int32_t fd )
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
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}


//------------------------------------------------------------------------------
//
//	IMB Commands
//

//------------------------------------------------------------------------------
int32_t CNX_TMSServer::IMB_Status( int32_t fd )
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
int32_t CNX_TMSServer::IMB_Version( int32_t fd )
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
int32_t CNX_TMSServer::ProcessCommand( int32_t fd, uint32_t cmd, void *pPayload, int32_t /*payloadSize*/ )
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

	case TCON_CMD_PATTERN:
		return TCON_TestPattern( fd, cmd, pData[0], pData[1], pData[2] );

	case TCON_CMD_MASTERING:
		return TCON_Mastering( fd, cmd, pData[0], pData[1], pData[2], pData[3] );

	case TCON_CMD_ELAPSED_TIME:
		return TCON_ElapsedTime( fd, cmd, pData[0] );

	case TCON_CMD_ACCUMULATE_TIME:
		return TCON_AccumulateTime( fd, cmd, pData[0] );

	case TCON_CMD_VERSION:
		return TCON_Version( fd, cmd, pData[0] );

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
static int32_t TestPatternFullScreenColor( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternFullScreenColor[][8] = {
	//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		10,		0,		1024,	0,		1080,	4095,	4095,	4095,	//	White	100%
		10,		0,		1024,	0,		1080,	3685,	3685,	3685,	//	Gray	90%
		10,		0,		1024,	0,		1080,	3276,	2948,	2948,	//	Gray	80%
		10,		0,		1024,	0,		1080,	2866,	2866,	2866,	//	Gray	70%
		10,		0,		1024,	0,		1080,	2457,	2457,	2457,	//	Gray	60%
		10,		0,		1024,	0,		1080,	2047,	2047,	2047,	//	Gray	50%
		10,		0,		1024,	0,		1080,	1638,	1638,	1638,	//	Gray	40%
		10,		0,		1024,	0,		1080,	1228,	1228,	1228,	//	Gray	30%
		10,		0,		1024,	0,		1080,	819,	819,	819,	//	Gray	20%
		10,		0,		1024,	0,		1080,	409,	409,	409,	//	Gray	1c0%
		10,		0,		1024,	0,		1080,	0,		0,		0,		//	Black	0%
		10,		0,		1024,	0,		1080,	4095,	0,		0,		//	Red		100%
		10,		0,		1024,	0,		1080,	0,		4095,	0,		//	Green	100%
		10,		0,		1024,	0,		1080,	0,		0,		4095,	//	Blue	100%
		10,		0,		1024,	0,		1080,	4095,	0,		4095,	//	Magenta	100%
		10,		0,		1024,	0,		1080,	0,		4095,	4095,	//	Cyan	100%
		10,		0,		1024,	0,		1080,	4095,	4095,	0,		//	Yellow 	00%
	};

	const uint16_t *pData = patternFullScreenColor[patternIndex];

	for( int32_t i = 0; i < (int32_t)(sizeof(patternFullScreenColor[0]) / sizeof(patternFullScreenColor[0][0])); i++ )
	{
		if( 0 > pI2c->Write( index, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}

		printf("[wr] slave(0x%02x), addr(0x%04x), data(0x%04x)\n", 
			index, TCON_REG_PATTERN + i, pData[i] );
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternGrayScale( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternGrayScale[][8] = {
	//	0x24	0x25	0x26	0x27	0x28
		2,		0,		1024,	0,		1080,							//	Gray 16-Step
		3,		0,		1024,	0,		1080,							//	Gray 32-Step
		4,		0,		1024,	0,		1080,							//	Gray 64-Step
		5,		0,		1024,	0,		1080,							//	Gray 128-Step
		6,		0,		1024,	0,		1080,							//	Gray 256-Step
		7,		0,		1024,	0,		1080,							//	Gray 512-Step
		9,		7,		1024,	0,		1080,							//	Gray 2048-Step
		9,		1,		1024,	0,		1080,							//	Red 2048-Step
		9,		2,		1024,	0,		1080,							//	Green 2048-Step
		9,		4,		1024,	0,		1080,							//	Blue 2048-Step
	};

	const uint16_t *pData = patternGrayScale[patternIndex];

	for( int32_t i = 0; i < (int32_t)(sizeof(patternGrayScale[0]) / sizeof(patternGrayScale[0][0])); i++ )
	{
		if( 0 > pI2c->Write( index, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}

		printf("[wr] slave(0x%02x), addr(0x%04x), data(0x%04x)\n", 
			index, TCON_REG_PATTERN + i, pData[i] );
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDot( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternDot[][8] = {
	//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		17,		0,		1024,	0,		1080,	4095,	4095,	4095,	//	Dot Pattern 1x1
	};

	const uint16_t *pData = patternDot[patternIndex];

	for( int32_t i = 0; i < (int32_t)(sizeof(patternDot[0]) / sizeof(patternDot[0][0])); i++ )
	{
		if( 0 > pI2c->Write( index, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}

		printf("[wr] slave(0x%02x), addr(0x%04x), data(0x%04x)\n", 
			index, TCON_REG_PATTERN + i, pData[i] );
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternDiagonal( CNX_I2C *pI2c, uint8_t index, uint8_t patternIndex )
{
	const uint16_t patternDiagonal[][8] = {
	//	0x24	0x25	0x26	0x27	0x28	0x29	0x2A	0x2B
		104,	0,		1024,	0,		1080,	65535,	65535,	65535,	//	Right-Down
		105,	0,		1024,	0,		1080,	65535,	65535,	65535,	//	Right-Up
	};

	const uint16_t *pData = patternDiagonal[patternIndex];

	for( int32_t i = 0; i < (int32_t)(sizeof(patternDiagonal[0]) / sizeof(patternDiagonal[0][0])); i++ )
	{
		if( 0 > pI2c->Write( index, TCON_REG_PATTERN + i, pData[i] ) )
		{
			return -1;
		}

		printf("[wr] slave(0x%02x), addr(0x%04x), data(0x%04x)\n", 
			index, TCON_REG_PATTERN + i, pData[i] );
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t TestPatternCabinetNum( CNX_I2C *pI2c, uint8_t index, uint8_t /*patternIndex*/ )
{
	if( 0 > pI2c->Write( index, TCON_REG_CABINET_ID, 0x0001 ) )
	{
		return -1;
	}
	
	printf("[wr] slave(0x%02x), addr(0x%04x), data(0x%04x)\n", 
		index, TCON_REG_CABINET_ID, 0x0001 );
	
	return 0;
}


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_TMSServer* CNX_TMSServer::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_TMSServer();
	}
	return (CNX_TMSServer*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_TMSServer::ReleaseInstance( )
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
