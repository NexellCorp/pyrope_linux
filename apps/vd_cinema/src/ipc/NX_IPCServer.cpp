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
	int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size);

	//	TCON Commands
	int32_t TCON_RegWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_RegRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_RegBurstWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	int32_t TCON_Init( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LvdsStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_BootingStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t	TCON_TargetGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t	TCON_DeviceGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_DotCorrection( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_DotCorrectionExtract( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_WhiteSeamRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_WhiteSeamWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_OptionalData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_SwReset( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t TCON_EEPRomRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );;
	int32_t TCON_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	PFPGA Commands
	int32_t PFPGA_RegWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_RegRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_RegBurstWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	int32_t PFPGA_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_Mute( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_UniformityData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_WriteConfig( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PFPGA_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	Platform Commands
	int32_t PLAT_NapVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PLAT_SapVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );
	int32_t PLAT_IpcVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	//	IMB Commands
	// int32_t IMB_ChangeContents( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize );

	int32_t ProcessCommand( int32_t clientSocket, uint32_t cmd, void *pPayload, int32_t payloadSize );

private:
	//	Private Member Variables
	enum { MAX_LOD_MODULE = 12, MAX_STR_SIZE = 1024 };

	bool		m_IsRunning;
	bool		m_ExitLoop;
	pthread_t	m_hThread;

	int32_t		m_hSocket;
	//	Key(4bytes) + Length(2bytes) + Command(2bytes) + Payload(MAX 65533bytes)
	uint8_t		m_SendBuf[MAX_PAYLOAD_SIZE + 8];
	uint8_t		m_ReceiveBuf[MAX_PAYLOAD_SIZE + 8];

	int32_t		(*m_pTestPatternFunc[6])( CNX_I2C*, uint8_t, uint8_t );

	uint8_t		m_NapVersion[MAX_STR_SIZE];
	uint8_t		m_SapVersion[MAX_STR_SIZE];
	uint8_t		m_IpcVersion[MAX_STR_SIZE];

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

static int32_t SendRemote( const char *pSockName, const char *pMsg );
static void    MakeDirectory( const char *pDir );

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
			if( key == NXP_KEY_VALUE )
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
		len = IPC_GET_LENGTH(pBuf[0], pBuf[1]);

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
		cmd = IPC_GET_COMMAND(pBuf[2], pBuf[3]);

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
int32_t CNX_IPCServer::TCON_RegWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index, pBuf[1] : msb reg, pBuf[2] : lsb reg, pBuf[3] : msb data, pBuf[4] : lsb data
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
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

	// printf( "Write Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n", port, slave, inReg, inData );

	result[0] = 0x00;
	result[1] = 0x00;
	result[2] = 0x00;
	result[3] = 0x00;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_RegRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index, pBuf[1] : msb reg, pBuf[2] : lsb reg
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_TCON;
	}

	NxDbgMsg( NX_DBG_INFO, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, iReadData );

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_RegBurstWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( fd );
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	//	Not Implemeation.
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Init( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DoorStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LvdsStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_BootingStatus( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedModeNormal( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_LIVE_LOD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LIVE_LOD_EN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	if( 0 > i2c.Write( slave, TCON_REG_PATTERN, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_PATTERN, 0x0000 );
		goto ERROR_TCON;
	}

	usleep( 100000 );

	result = 0x01;
#endif

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, snedSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedModeLod( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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

	usleep( 100000 );

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

	usleep( 3000000 );	// delay during 3.00sec over ( LOD Scan Time )

	if( 0 > i2c.Write( slave, TCON_REG_ERROR_OUT_SEL, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_ERROR_OUT_SEL, 0x0001 );
		goto ERROR_TCON;
	}

	result = 0x01;
#endif

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedOpenNum( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_LedOpenPos( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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

	usleep( 50000 );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TestPattern( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_TargetGamma( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

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

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
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

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
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

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

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

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
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

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

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

	uint16_t iMsbCheckSum = 0;
	uint16_t iLsbCheckSum = 0;

	int32_t iRetryCount = 0, bFail;

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

		iMsbCheckSum = iMsbCheckSum ^ pMsbData[i];
		iLsbCheckSum = iLsbCheckSum ^ pLsbData[i];
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

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
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

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iMsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

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

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
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

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iLsbCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}

		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( bFail ) {
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pMsbData );
	free( pLsbData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DotCorrection( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	uint64_t iCurTime = NX_GetTickCount();

	//
	//	pBuf[0] : index, pBuf[1] : module, pBuf[2] .. : data ( num of data : iSize - 1 )
	//
	uint8_t result	= 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t module = pBuf[1];
	uint8_t *ptr	= pBuf + 2;
	int32_t size	= iSize - 2;

	int32_t iDataSize = size / 2;
	uint16_t *pData = (uint16_t*)malloc( sizeof(uint16_t) * iDataSize );

	uint16_t ccData[9] = { 0x0000, };
	uint16_t opData[16] = { 0x0000, };

	uint16_t iCheckSum = 0;
	int32_t iRetryCount = 0, bFail;

	for( int32_t i = 0; i < iDataSize; i++ )
	{
		pData[i] = ((int16_t)(ptr[2 * i + 0] << 8) & 0xFF00) | ((int16_t)(ptr[2 * i + 1] << 0) & 0x00FF);
		iCheckSum = iCheckSum ^ pData[i];
	}

#if 0	// For Debugging.
	uint16_t *pOriData = (uint16_t*)malloc( sizeof(uint16_t) * 64 * 60 * 9 );
	int offset = 0, oriOffset = 0;

	for( int32_t i = 0; i < 64 * 60; i++ )
	{
        pOriData[oriOffset    ] = ((pData[offset + 7]      ) & 0x3FFF);
        pOriData[oriOffset + 1] = ((pData[offset + 6] >>  2) & 0x3FFF);
        pOriData[oriOffset + 2] = ((pData[offset + 6] << 12) & 0x3000) | ((pData[offset + 5] >>  4) & 0x0FFF);
        pOriData[oriOffset + 3] = ((pData[offset + 5] << 10) & 0x3C00) | ((pData[offset + 4] >>  6) & 0x03FF);
        pOriData[oriOffset + 4] = ((pData[offset + 4] <<  8) & 0x3F00) | ((pData[offset + 3] >>  8) & 0x00FF);
        pOriData[oriOffset + 5] = ((pData[offset + 3] <<  6) & 0x3FC0) | ((pData[offset + 2] >> 10) & 0x003F);
        pOriData[oriOffset + 6] = ((pData[offset + 2] <<  4) & 0x3FF0) | ((pData[offset + 1] >> 12) & 0x000F);
        pOriData[oriOffset + 7] = ((pData[offset + 1] <<  2) & 0x3FFC) | ((pData[offset    ] >> 14) & 0x0003);
        pOriData[oriOffset + 8] = ((pData[offset    ]      ) & 0x3FFF);

		printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			pOriData[oriOffset + 0], pOriData[oriOffset + 1], pOriData[oriOffset + 2], pOriData[oriOffset + 3],
			pOriData[oriOffset + 4], pOriData[oriOffset + 5], pOriData[oriOffset + 6], pOriData[oriOffset + 7],
			pOriData[oriOffset + 8] );

        oriOffset += 9;
        offset += 8;
	}

	if( pOriData ) free( pOriData );
#endif

	printf("port(%d), slave(0x%02x), module(%d)\n", port, slave, module);

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	// 0. Driver Clock Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(50000);

	//
	// 1. Read CC data.
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

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module >> 1 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module >> 1 );
		goto ERROR_TCON;
	}
	usleep(100000);

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

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}

	//
	//	2. Read Optional Data
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
	usleep(50000);

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

	//
	//	3. Flash Protection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
		goto ERROR_TCON;
	}

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	4. Erase Flash.
	//
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
	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0040) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0040 );
		goto ERROR_TCON;
	}

	iRetryCount = TCON_BURST_RETRY_COUNT;
	while( iRetryCount )
	{
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
		usleep(100000);

		if( (iDataSize == i2c.Read( slave, TCON_REG_BURST_DATA_CNT )) &&
			(iCheckSum == i2c.Read( slave, TCON_REG_BURST_DATA_XOR )) )
		{
			bFail = false;
			break;
		}
		// NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
		// 	port, slave, TCON_REG_BURST_DATA_CNT );

		iRetryCount--;
		bFail = true;
	}

	if( 0 > i2c.Write( slave, TCON_REG_LUT_BURST_SEL, 0x0000) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_LUT_BURST_SEL, 0x0000 );
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
	usleep(500000);

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
	usleep(500000);

	//
	//	8. Flash Protection On.
	//
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
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);

	//
	// 9. Driver Clock On.
	//
	if( 0 > i2c.Write( slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_DRVIVER_CLK_OFF, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(50000);

	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	if( pData ) free( pData );

	printf(">>>>> %s() Done : %lld ms\n", __FUNCTION__, NX_GetTickCount() - iCurTime );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_DotCorrectionExtract( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	uint64_t iStartTime = NX_GetTickCount();

	//
	//	pBuf[0] : index, pBuf[1] : module index
	//
	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pBuf[1];
	uint16_t nBaseX = module % 4 * 64;
	uint16_t nBaseY = module / 4 * 60;

	int32_t iResultLen = 64 * 60 * 8 * 2;
	uint8_t *pResult = (uint8_t*)malloc( iResultLen * sizeof(uint8_t) );
	memset( pResult, 0x00, iResultLen * sizeof(uint8_t) );

	uint8_t *pPtr = pResult;

	int32_t sendSize;
	int32_t cnt = 0;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	printf("port(%d), slave(%d), modlue(%d)\n", port, slave, module);

	for( int32_t i = 0; i < 60; i++ )
	{
		for( int32_t j = 0; j < 64; j++ )
		{
			int16_t ccData14[9] = { 0x0000, };
			int16_t ccData16[8] = { 0x0000, };

#if 1
			uint16_t pos_x = j + nBaseX;
			uint16_t pos_y = i + nBaseY;

			// printf("pos_x(%d), pos_y(%d)\n", pos_x, pos_y);

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_X, pos_x ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_X, pos_x );
				goto ERROR_TCON;
			}

			if( 0 > i2c.Write( slave, TCON_REG_ADDR_CC_POS_Y, pos_y ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_ADDR_CC_POS_Y, pos_y );
				goto ERROR_TCON;
			}

			usleep(50000);

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				if( 0 > (ccData14[k] = i2c.Read( slave, TCON_REG_ADDR_CC_IN_CC00 + k )) )
				{
					NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
						port, slave, TCON_REG_ADDR_CC_IN_CC00 + k );
					goto ERROR_TCON;
				}
				ccData14[k] &= 0x3FFF;
			}
#else
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData14) / sizeof(ccData14[0])); k++ )
			{
				ccData14[k] = cnt;		// 0101 0101 0101 0101
				ccData14[k] &= 0x3FFF;	// 0001 0101 0101 0101

				cnt = (cnt + 1) % 0x3FFF;
			}
#endif

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData14[0], ccData14[1], ccData14[2], ccData14[3], ccData14[4],
			// 	ccData14[5], ccData14[6], ccData14[7], ccData14[8] );

			//
			//	Convert Data ( 14bit x 9EA --> 16bit x 8EA )
			//
			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				ccData16[0] = ((ccData14[7] & 0x0003) << 14) | ((ccData14[8] & 0x3FFF) >>  0);
				ccData16[1] = ((ccData14[6] & 0x000F) << 12) | ((ccData14[7] & 0x3FFC) >>  2);
				ccData16[2] = ((ccData14[5] & 0x003F) << 10) | ((ccData14[6] & 0x3FF0) >>  4);
				ccData16[3] = ((ccData14[4] & 0x00FF) <<  8) | ((ccData14[5] & 0x3FC0) >>  6);
				ccData16[4] = ((ccData14[3] & 0x03FF) <<  6) | ((ccData14[4] & 0x3F00) >>  8);
				ccData16[5] = ((ccData14[2] & 0x0FFF) <<  4) | ((ccData14[3] & 0x3C00) >> 10);
				ccData16[6] = ((ccData14[1] & 0x3FFF) <<  2) | ((ccData14[2] & 0x3000) >> 12);
				ccData16[7] =                                  ((ccData14[0] & 0x3FFF) >>  0);
			}

			// printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			// 	ccData16[0], ccData16[1], ccData16[2], ccData16[3],
			// 	ccData16[4], ccData16[5], ccData16[6], ccData16[7] );

			for( int32_t k = 0; k < (int32_t)(sizeof(ccData16) / sizeof(ccData16[0])); k++ )
			{
				pPtr[k * 2    ] = (uint8_t)((ccData16[k] >> 8) & 0xFF);
				pPtr[k * 2 + 1] = (uint8_t)((ccData16[k] >> 0) & 0xFF);
			}
			pPtr += 16;

			//	Print Progress Debugging
			fprintf( stdout, "> %4d / %4d ( %3d %% )\r",
				i * 64 + j + 1, 60 * 64,
				(int)((float)((i * 64) + j + 1) / (float)(64 * 60) * (float)100) );
			fflush( stdout );
		}
	}

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, pResult, iResultLen, m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	if( pResult ) free( pResult );

	printf( "\n>> DotCorrection Extract Done. ( %lld mSec )\n", NX_GetTickCount() - iStartTime );
	return 0;
}

//------------------------------------------------------------------------------
#define IS_TCON_MODULE_TOP(module)		(((module / 4) == 0) ? true : false)
#define IS_TCON_MODULE_BOTTOM(module)	(((module / 4) == 5) ? true : false)
#define IS_TCON_MODULE_LEFT(module)		(((module % 4) == 0) ? true : false)
#define IS_TCON_MODULE_RIGHT(module)	(((module % 4) == 3) ? true : false)

#define TCON_MODULE_TOP			0
#define TCON_MODULE_BOTTOM		1
#define TCON_MODULE_LEFT		2
#define TCON_MODULE_RIGHT		3

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_WhiteSeamRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t iSeamValue[4] = { 16384, 16384, 16384, 16384 };

	uint16_t iTempValue[4];
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	2. Read top seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_TOP] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_TOP] = iTempValue[TCON_MODULE_TOP];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_TOP, iSeamValue[TCON_MODULE_TOP] );
		goto ERROR_TCON;
	}

	//
	//	3. Read bottom seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_BOTTOM(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA01_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA01_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_BOTTOM] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_BOTTOM] = iTempValue[TCON_MODULE_BOTTOM];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_BOTTOM, iSeamValue[TCON_MODULE_BOTTOM] );
		goto ERROR_TCON;
	}

	//
	//	4. Read left seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_LEFT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA02_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_LEFT] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_LEFT] = iTempValue[TCON_MODULE_LEFT];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_LEFT, iSeamValue[TCON_MODULE_LEFT] );
		goto ERROR_TCON;
	}

	//
	//	5. Read right seam value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_RIGHT(module) )
			continue;

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA03_READ )) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_DATA00_READ );
			goto ERROR_TCON;
		}

		iTempValue[TCON_MODULE_RIGHT] = iReadData & 0x7FFF;
		iSeamValue[TCON_MODULE_RIGHT] = iTempValue[TCON_MODULE_RIGHT];
	}

	if( 0 > i2c.Write( slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SEAM_RIGHT, iSeamValue[TCON_MODULE_RIGHT] );
		goto ERROR_TCON;
	}

	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_WhiteSeamWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index, pBuf[1:2] : top, pBuf[3:4] : bottom, pBuf[5:6] : left, pBuf[7:8] : right
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint16_t opData[24][16]	= { 0x0000, };
	uint16_t iSeamValue[4];

	for( int32_t i = 0; i < 4; i++ )
	{
		iSeamValue[i] = ((int16_t)(pBuf[i*2+1] << 8) & 0xFF00) + (int16_t)pBuf[i*2+2];
	}

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Read Flash Data. ( FLASH --> SRAM in FPGA ) :: All optional data(24EA) is read.
	//
	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0001 );
		goto ERROR_TCON;
	}
	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_F_LED_RD_EN, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_F_LED_RD_EN, 0x0000 );
		goto ERROR_TCON;
	}
	usleep(75000);

	//
	//	2. Read Optional Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	2-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	2-2. Read Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			int32_t iReadData = 0x0000;
			if( 0 > (iReadData = i2c.Read( slave, TCON_REG_F_LED_DATA00_READ + i )) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00_READ + i );
				goto ERROR_TCON;
			}

			opData[module][i] = (uint16_t)(iReadData & 0x0000FFFF);
		}
	}

	//
	//	3. Modify Seam Value.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		if( !IS_TCON_MODULE_TOP(module) && !IS_TCON_MODULE_BOTTOM(module) &&
			!IS_TCON_MODULE_LEFT(module) && !IS_TCON_MODULE_RIGHT(module) )
		{
			continue;
		}

		if( IS_TCON_MODULE_TOP(module) )
		{
			opData[module][TCON_MODULE_TOP]    = iSeamValue[TCON_MODULE_TOP];

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
		}

		if( IS_TCON_MODULE_BOTTOM(module) )
		{
			opData[module][TCON_MODULE_BOTTOM]	= iSeamValue[TCON_MODULE_BOTTOM];

			if( IS_TCON_MODULE_LEFT(module) )	opData[module][TCON_MODULE_LEFT] = iSeamValue[TCON_MODULE_LEFT];
			if( IS_TCON_MODULE_RIGHT(module) )	opData[module][TCON_MODULE_RIGHT] = iSeamValue[TCON_MODULE_RIGHT];
		}

		if( IS_TCON_MODULE_LEFT(module) )
		{
			opData[module][TCON_MODULE_LEFT]	= iSeamValue[TCON_MODULE_LEFT];

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
		}

		if( IS_TCON_MODULE_RIGHT(module) )
		{
			opData[module][TCON_MODULE_RIGHT]	= iSeamValue[TCON_MODULE_RIGHT];

			if( IS_TCON_MODULE_TOP(module) )	opData[module][TCON_MODULE_TOP] = iSeamValue[TCON_MODULE_TOP];
			if( IS_TCON_MODULE_BOTTOM(module) )	opData[module][TCON_MODULE_BOTTOM] = iSeamValue[TCON_MODULE_BOTTOM];
		}
	}

	//
	//	4. Write Flash Data.
	//
	for( int32_t module = 0; module < 24; module++ )
	{
		//
		//	4-0. Process Skip if NOT (IS_TCON_MODULE_TOP & IS_TCON_MODULE_BOTTOM & IS_TCON_MODULE_LEFT & IS_TCON_MODULE_RIGHT)
		//
		if( !IS_TCON_MODULE_TOP(module) && !IS_TCON_MODULE_BOTTOM(module) &&
			!IS_TCON_MODULE_LEFT(module) && !IS_TCON_MODULE_RIGHT(module) )
		{
			continue;
		}

		//
		//	4-1. Flash Selection.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_SEL, module );
			goto ERROR_TCON;
		}
		usleep(100000);

		//
		//	4-2. Flash Protection Off.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0002 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		//
		//	4-3. Write Optional Data.
		//
		for( int32_t i = 0; i < 16; i++ )
		{
			if( 0 > i2c.Write( slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] ) )
			{
				NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
					port, slave, TCON_REG_F_LED_DATA00 + i, opData[module][i] );
				goto ERROR_TCON;
			}
		}

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0001 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0001 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_F_LED_WR_EN, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_F_LED_WR_EN, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(500000);

		//
		//	4-4. Flash Protection On.
		//
		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0003 );
			goto ERROR_TCON;
		}
		usleep(75000);

		if( 0 > i2c.Write( slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 ) )
		{
			NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
				port, slave, TCON_REG_FLASH_STATUS_WRITE, 0x0000 );
			goto ERROR_TCON;
		}
		usleep(75000);
	}

	//
	//	5. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_AccumulateTime( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( fd );
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	//	pBuf[0] : index, pBuf[1] : module,
	//

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_OptionalData( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index, pBuf[1] : module
	//

	uint8_t result[32] = { 0xFF, };
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t module	= pBuf[1];

	uint16_t opData[16] = { 0x0000, };

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	//
	//	1. Flash Selection.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, module ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, module );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	2. Pluse Generation for Flash Read.
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
	usleep(50000);

	//
	//	3. Read Optional Data.
	//
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
		usleep(50000);
	}

	//
	//	4. Flash Selection Off.
	//
	if( 0 > i2c.Write( slave, TCON_REG_FLASH_SEL, 0x001F ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_FLASH_SEL, 0x001F );
		goto ERROR_TCON;
	}
	usleep(100000);

	//
	//	Print Debug Message
	//
	printf(">> Optional Data Information : index(%d), module(%d)\n", index, module);

	for( int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		printf(" -. REG_F_LED_DATA%02d_READ : 0x%04x\n", i, opData[i]);
	}

	//
	//
	//
	for(int32_t i = 0; i < (int32_t)(sizeof(opData) / sizeof(opData[0])); i++ )
	{
		result[i * 2 + 0] = (uint8_t)((opData[i] >> 8) & 0xFF);
		result[i * 2 + 1] = (uint8_t)((opData[i] >> 0) & 0xFF);
	}

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_SwReset( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(75000);

	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	if( 0 > i2c.Write( slave, TCON_REG_SW_RESET, 0x0001 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_SW_RESET, 0x0001 );
		goto ERROR_TCON;
	}

	usleep(150000);

	if( 0 > i2c.Write( slave, TCON_REG_MUTE, 0x0000 ) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Write(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, TCON_REG_MUTE, 0x0000 );
		goto ERROR_TCON;
	}

	usleep(75000);


	result = 0x01;

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_EEPRomRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

	uint8_t result	= 0xFF;
	int32_t sendSize;

	int32_t iAddr = 0, iReadSize;
	int32_t iRemainSize;

	uint8_t *pInBuf = (uint8_t*)malloc( TCON_EEPROM_DATA_SIZE );
	uint8_t *pPtr;

	int32_t iRet;
	CNX_EEPRom eeprom;
	CNX_EEPRomDataParser parser;
	TCON_EEPROM_INFO *pInfo = NULL;
	uint8_t version[TCON_EEPROM_MAX_VERSION_SIZE];

	FILE *pFile = NULL;
	int32_t bUpdate = true;

	//
	//	0. Check Version
	//
	if( !access( TCON_EEPROM_BINARY_FILE, F_OK) )
	{
		iAddr       = 0;
		pPtr        = pInBuf;
		iRemainSize = TCON_EEPROM_VERSION_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n" );
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_VERSION_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_VERSION_SIZE );

		//
		//	0-1. EEPROM Version Parsing.
		//
		iRet = parser.Init( pInBuf, TCON_EEPROM_VERSION_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		memcpy( version, pInfo->version, TCON_EEPROM_MAX_VERSION_SIZE );
		parser.Deinit();

		//
		//	0-2. Binary Version Parsing.
		//
		parser.Init( TCON_EEPROM_BINARY_FILE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.ParseVersion( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser ParseVersion(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		// NX_HexDump( version, sizeof(version) );
		// NX_HexDump( pInfo->version, sizeof(version) );

		if( !strcmp( (char*)pInfo->version, (char*)version ) )
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version match.\n");
			bUpdate = false;
		}
		else
		{
			NxDbgMsg( NX_DBG_DEBUG, "Version missmatch.\n");
		}

		parser.Deinit();
	}

	if( bUpdate )
	{
		//
		//	1. Read EEPRom Data.
		//
		iAddr       = 0;
		pPtr        = pInBuf;
		iRemainSize = TCON_EEPROM_DATA_SIZE;

		while( 0 < iRemainSize )
		{
			if( iRemainSize > TCON_EEPROM_PAGE_SIZE ) iReadSize = TCON_EEPROM_PAGE_SIZE;
			else iReadSize = iRemainSize;

			if( 0 > eeprom.Read( iAddr, pPtr, iReadSize) )
			{
				NxDbgMsg( NX_DBG_ERR, "Fail, EEPRom Read().\n");
				goto ERROR_TCON;
			}

			iAddr		+= iReadSize;
			pPtr		+= iReadSize;
			iRemainSize -= iReadSize;

			fprintf(stdout, "EEPRom Read. ( 0x%08X / 0x%08x )\r", iAddr, TCON_EEPROM_DATA_SIZE);
			fflush(stdout);

			NxDbgMsg( NX_DBG_DEBUG, "EEPRom Read. ( 0x%08X / 0x%08x )\n", iAddr, TCON_EEPROM_DATA_SIZE );
		}
		NxDbgMsg( NX_DBG_DEBUG, "\nEEPRom Read Done. ( %d bytes )\n", TCON_EEPROM_DATA_SIZE );

		//
		//	2. Make Binary File.
		//
		pFile = fopen( TCON_EEPROM_BINARY_FILE, "wb" );
		if( NULL == pFile )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Make Binary File. ( %s )\n", TCON_EEPROM_BINARY_FILE );
			goto ERROR_TCON;
		}

		fwrite( pInBuf, 1, TCON_EEPROM_DATA_SIZE, pFile );
		fclose( pFile );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s )\n", TCON_EEPROM_BINARY_FILE );

		//
		//	3. Make T_REG_EEPROM.txt
		//
		iRet = parser.Init( pInBuf, TCON_EEPROM_DATA_SIZE );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Init(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		iRet = parser.Parse( &pInfo );
		if( 0 > iRet )
		{
			NxDbgMsg( NX_DBG_ERR, "Fail, Parser Parse(). ( iRet = %d )\n", iRet );
			goto ERROR_TCON;
		}

		parser.WriteTconInfo( pInfo, TCON_EEPROM_RESULT_PATH );
		NxDbgMsg( NX_DBG_DEBUG, "Make done. ( %s/T_REG_EEPROM.txt )\n", TCON_EEPROM_RESULT_PATH );

		sync();

		result = 0x01;		// Update Successful.
	}
	else
	{
		result = 0x00;		// Update is not needed.
	}

	NxDbgMsg( NX_DBG_ERR, "%s(). ( result = 0x%02X )\n", __FUNCTION__, result );

ERROR_TCON:
	parser.Deinit();
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::TCON_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : index
	//

	uint8_t result[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;

	uint8_t index	= pBuf[0];
	int32_t port	= (index & 0x80) >> 7;
	uint8_t slave	= (index & 0x7F);

	uint8_t msbReg	= pBuf[1];
	uint8_t lsbReg	= pBuf[2];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iModeName, iTime;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_TCON;
	}

	if( 0 > (iModeName = i2c.Read( slave, TCON_REG_FPGA_MODE_NAME )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_MODE_NAME );
		goto ERROR_TCON;
	}

	if( 0 > (iTime = i2c.Read( slave, TCON_REG_FPGA_TIME1 )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, TCON_REG_FPGA_TIME1 );
		goto ERROR_TCON;
	}

	result[0] = (uint8_t)((iModeName >> 24) & 0xFF);
	result[1] = (uint8_t)((iModeName >> 16) & 0xFF);
	result[2] = (uint8_t)((iModeName >>  8) & 0xFF);
	result[3] = (uint8_t)((iModeName >>  0) & 0xFF);

	result[4] = (uint8_t)((iTime >> 24) & 0xFF);
	result[5] = (uint8_t)((iTime >> 16) & 0xFF);
	result[6] = (uint8_t)((iTime >>  8) & 0xFF);
	result[7] = (uint8_t)((iTime >>  0) & 0xFF);

ERROR_TCON:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	PFPGA Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_RegWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : msb reg, pBuf[1] : lsb reg, pBuf[2] : msb data, pBuf[3] : lsb data
	//

	uint8_t	result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
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

	result[0] = 0x00;
	result[1] = 0x00;
	result[2] = 0x00;
	result[3] = 0x00;

ERROR_PFPGA:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_RegRead( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

	//
	//	pBuf[0] : msb reg, pBuf[1] : lsb reg
	//

	uint8_t	result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	uint8_t msbReg	= pBuf[0];
	uint8_t lsbReg	= pBuf[1];

	uint16_t inReg	= ((int16_t)(msbReg << 8) & 0xFF00) + (int16_t)lsbReg;
	int32_t iReadData;

	CNX_I2C i2c( port );

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, inReg )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, inReg );
		goto ERROR_PFPGA;
	}

	NxDbgMsg( NX_DBG_INFO, "Read Data. ( i2c-%d, slave: 0x%02x, reg: 0x%04x, data: 0x%04x )\n",
			port, slave, inReg, iReadData );

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_PFPGA:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_RegBurstWrite( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( fd );
	UNUSED( cmd );
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	//	Not Implemetation
	//

	uint8_t result = 0xFF;
	int32_t sendSize;

ERROR_PFPGA:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Status( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	free( pData );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Mute( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( iSize );

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
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PFPGA_Version( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	//	pBuf : not used
	//

	uint8_t result[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	int32_t sendSize;

	int32_t port	= PFPGA_I2C_PORT;
	uint8_t slave	= PFPGA_I2C_SLAVE;

	CNX_I2C i2c( port );
	int32_t iReadData = 0x0000;

	if( 0 > i2c.Open() )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Open(). ( i2c-%d )\n", port );
		goto ERROR_PFPGA;
	}

	if( 0 > (iReadData = i2c.Read( slave, PFPGA_REG_PF_VERSION )) )
	{
		NxDbgMsg( NX_DBG_VBS, "Fail, Read(). ( i2c-%d, slave: 0x%02x, reg: 0x%04x )\n",
			port, slave, PFPGA_REG_PF_VERSION );
		goto ERROR_PFPGA;
	}

	result[0] = (uint8_t)((iReadData >> 24) & 0xFF);
	result[1] = (uint8_t)((iReadData >> 16) & 0xFF);
	result[2] = (uint8_t)((iReadData >>  8) & 0xFF);
	result[3] = (uint8_t)((iReadData >>  0) & 0xFF);

ERROR_PFPGA:
	sendSize = IPC_MakePacket( SEC_KEY_VALUE, cmd, result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sendSize );

	return 0;
}


//------------------------------------------------------------------------------
//
//	Platform Commands
//

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PLAT_NapVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	// pBuf : not used
	//

	int32_t sendSize = IPC_MakePacket ( SEC_KEY_VALUE, cmd, m_NapVersion, strlen((const char*)m_NapVersion), m_SendBuf, sizeof(m_SendBuf) );
	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sent );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PLAT_SapVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	// pBuf : not used
	//

	int32_t sendSize = IPC_MakePacket ( SEC_KEY_VALUE, cmd, m_SapVersion, strlen((const char*)m_SapVersion), m_SendBuf, sizeof(m_SendBuf) );
	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sent );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::PLAT_IpcVersion( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
{
	UNUSED( pBuf );
	UNUSED( iSize );

	//
	// pBuf : not used
	//

	int32_t sendSize = IPC_MakePacket ( SEC_KEY_VALUE, cmd, m_IpcVersion, strlen((const char*)m_IpcVersion), m_SendBuf, sizeof(m_SendBuf) );
	write( fd, m_SendBuf, sendSize );
	// NX_HexDump( m_SendBuf, sent );
	return 0;
}

//------------------------------------------------------------------------------
// int32_t CNX_IPCServer::IMB_ChangeContents( int32_t fd, uint32_t cmd, uint8_t *pBuf, int32_t iSize )
// {
// 	UNUSED( iSize );
// #if 0
// 	//
// 	//	pBuf[0] : color format,	pBuf[1] : Bit number,	pBuf[2] : Resolution, 	pBuf[3:4] : Frame rate,
// 	//	pBuf[5] : Gamma curve,	pBuf[6:7] : Brightness,	pBuf[8] : Enhancement setting
// 	//

// 	uint8_t result	= 0xFF;
// 	int32_t sendSize;

// 	uint16_t iColorFormat	= (uint16_t)pBuf[0];
// 	uint16_t iBitNumber		= (uint16_t)pBuf[1];
// 	uint16_t iResolution	= (uint16_t)pBuf[2];
// 	uint16_t iFrameRate		= ((int16_t)(pBuf[3] << 8) & 0xFF00) + (int16_t)pBuf[4];
// 	uint16_t iGammaCurve	= (uint16_t)pBuf[5];
// 	uint16_t iBrightness	= ((int16_t)(pBuf[6] << 8) & 0xFF00) + (int16_t)pBuf[7];
// 	uint16_t iEnhancement	= (uint16_t)pBuf[8];

// 	printf(">> iColorFormat	= %5d ( 0x%04x )\n", iColorFormat, iColorFormat );
// 	printf(">> iBitNumber	= %5d ( 0x%04x )\n", iBitNumber, iBitNumber );
// 	printf(">> iResolution	= %5d ( 0x%04x )\n", iResolution, iResolution );
// 	printf(">> iFrameRate	= %5d ( 0x%04x )\n", iFrameRate, iFrameRate );
// 	printf(">> iGammaCurve	= %5d ( 0x%04x )\n", iGammaCurve, iGammaCurve );
// 	printf(">> iBrightness	= %5d ( 0x%04x )\n", iBrightness, iBrightness );
// 	printf(">> iEnhancement	= %5d ( 0x%04x )\n", iEnhancement, iEnhancement );

// 	result = 0x01;
// #else
// 	uint8_t result = 0xFF;
// 	int32_t sendSize;

// 	printf(">> receive data : 0x%02x\n", pBuf[0]);

// 	if( pBuf[0] == 0x00 || pBuf[0] == 0x01 )
// 	{
// 		if( !SendRemote( "cinema.change.contents", (pBuf[0] == 0x00) ? "0" : "1" ) )
// 		{
// 			result = 0x01;
// 		}
// 	}
// #endif
// ERROR_RESERVED:
// 	sendSize = IPC_MakePacket ( SEC_KEY_VALUE, cmd, &result, sizeof(result), m_SendBuf, sizeof(m_SendBuf) );

// 	write( fd, m_SendBuf, sendSize );
// 	// NX_HexDump( m_SendBuf, sent );

// 	return 0;
// }

//------------------------------------------------------------------------------
int32_t CNX_IPCServer::ProcessCommand( int32_t fd, uint32_t cmd, void *pPayload, int32_t payloadSize )
{
	// printf("cmd( 0x%04X ), payload( %p ), size( %d )\n", cmd, pPayload, payloadSize);

	switch( cmd )
	{
	//
	//	TCON Commands
	//
	case TCON_CMD_REG_WRITE:
		return TCON_RegWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_REG_READ:
		return TCON_RegRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_REG_BURST_WRITE:
		return TCON_RegBurstWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

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

	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:
		return TCON_TestPattern( fd, cmd, (uint8_t*)pPayload, payloadSize );

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

	case TCON_CMD_DOT_CORRECTION_EXTRACT:
		return TCON_DotCorrectionExtract( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_WHITE_SEAM_READ:
		return TCON_WhiteSeamRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_WHITE_SEAM_WRITE:
		return TCON_WhiteSeamWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:
		return TCON_OptionalData( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_SW_RESET:
		return TCON_SwReset( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_EEPROM_READ:
		return TCON_EEPRomRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case TCON_CMD_VERSION:
		return TCON_Version( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	PFPGA Commands
	//
	case PFPGA_CMD_REG_WRITE:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_REG_WRITE ):
		return PFPGA_RegWrite( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_REG_READ:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_REG_READ ):
		return PFPGA_RegRead( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_STATUS:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_STATUS ):
		return PFPGA_Status( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_MUTE:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_MUTE ):
		return PFPGA_Mute( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_UNIFORMITY_DATA:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_UNIFORMITY_DATA ):
		return PFPGA_UniformityData( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PFPGA_CMD_VERSION:
	case GDC_COMMAND( CMD_TYPE_PFPGA, PFPGA_CMD_VERSION ):
		return PFPGA_Version( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	Platform Commands
	//
	case PLATFORM_CMD_NAP_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_NAP_VERSION ):
		return PLAT_NapVersion( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PLATFORM_CMD_SAP_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_SAP_VERSION ):
		return PLAT_SapVersion( fd, cmd, (uint8_t*)pPayload, payloadSize );

	case PLATFORM_CMD_IPC_SERVER_VERSION:
	case GDC_COMMAND( CMD_TYPE_PLATFORM, PLATFORM_CMD_IPC_SERVER_VERSION ):
		return PLAT_IpcVersion( fd, cmd, (uint8_t*)pPayload, payloadSize );

	//
	//	IMB Commands
	//
	// case IMB_CMD_CHANGE_CONTENTS:
	// case GDC_COMMAND( CMD_TYPE_IMB, IMB_CMD_CHANGE_CONTENTS ):
	// 	return IMB_ChangeContents( fd, cmd, (uint8_t*)pPayload, payloadSize );

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

//------------------------------------------------------------------------------
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
// static int32_t SendRemote( const char *pSockName, const char *pMsg )
// {
// 	int32_t sock, len;
// 	struct sockaddr_un addr;

// 	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
// 	{
// 		printf("Fail, socket().\n");
// 		return -1;
// 	}

// 	addr.sun_family  = AF_UNIX;
// 	addr.sun_path[0] = '\0';	// for abstract namespace
// 	strcpy( addr.sun_path + 1, pSockName );

// 	len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

// 	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
// 	{
// 		printf("Fail, connect(). ( node: %s )\n", pSockName);
// 		close( sock );
// 		return -1;
// 	}

// 	if( 0 > write(sock, pMsg, strlen(pMsg)) )
// 	{
// 		printf("Fail, write().\n");
// 		close( sock );
// 		return -1;
// 	}

// 	close( sock );
// 	return 0;
// }

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
				printf("Make Directory. ( %s )\n", buf);
				mkdir( buf, 0777 );
			}
			*pBuf = '/';
		}
		pBuf++;
	}

	if( 0 != access( buf, F_OK) )
	{
		printf("Make Directory. ( %s )\n", buf);
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
