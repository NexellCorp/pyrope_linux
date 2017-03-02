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

#include <NX_IPCClient.h>
#include <NX_IPCCommand.h>

#define NX_DTAG	"[IPC Client]"
#include <NX_DbgMsg.h>

#define	IPC_SERVER_FILE "/data/local/tmp/ipc_server"

//------------------------------------------------------------------------------
//
//	IPC Client APIs
//
class CNX_IPCClient
{
public:
	CNX_IPCClient(){};
	virtual ~CNX_IPCClient(){};

	static CNX_IPCClient* GetInstance( );
	static void ReleaseInstance( );

public:
	int32_t TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t IPCCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );

private:
	//	TCON Commands
	int32_t TCONCmdRead( int32_t id, uint32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t TCONCmdRead( int32_t id, uint32_t cmd, uint8_t reg, uint8_t *pBuf, int32_t *size );
	int32_t TCONCmdWrite( int32_t id, uint32_t cmd );
	int32_t TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf );
	int32_t TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf1, uint8_t buf2 );
	int32_t TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf1, uint8_t buf2, uint8_t buf3 );
	int32_t TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t *pBuf, int32_t *size );

	//	PFPGA Commands
	int32_t PFPGACmdRead( uint32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t PFPGACmdWrite( uint32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t PFPGACmdWrite( uint32_t cmd, uint8_t buf1, uint8_t buf2, uint8_t buf3 );
	int32_t PFPGACmdSource( uint8_t index );
	int32_t PFPGACmdVersion( uint8_t *pVersion );

	//	Battery Commands
	int32_t BATCmdStatus( uint8_t *pBuf, int32_t *size );

	//	IMB Commands
	int32_t IMBCmdStatus( uint8_t *pBuf, int32_t *size );

	//	IPC Commands
	int32_t IPCCmdServerVersion( uint32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t IPCCmdClientVerison( uint32_t cmd, uint8_t *pBuf, int32_t *size );

private:
	//	Rx/Tx Buffer
	uint8_t m_SendBuf[64*1024];
	uint8_t m_ReceiveBuf[64*1024];

private:
	//	For Singleton
	static CNX_IPCClient	*m_psInstance;
};

CNX_IPCClient* CNX_IPCClient::m_psInstance = NULL;

//------------------------------------------------------------------------------
//
//	TCON Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdRead( int32_t id, uint32_t cmd, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	uint8_t data[1] = { (uint8_t)id };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdRead( int32_t id, uint32_t cmd, uint8_t reg, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	uint8_t data[2] = { (uint8_t)id, reg };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdWrite( int32_t id, uint32_t cmd )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;

	uint8_t data[1] = { (uint8_t)id };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock );
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;

	uint8_t data[2] = { (uint8_t)id, buf };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock );
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf1, uint8_t buf2 )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;

	uint8_t data[3] = { (uint8_t)id, buf1, buf2 };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock );
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t buf1, uint8_t buf2, uint8_t buf3 )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;

	uint8_t data[4] = { (uint8_t)id, buf1, buf2, buf3 };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCmdWrite( int32_t id, uint32_t cmd, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	int32_t iDataSize = *size + 1;
	uint8_t *pData = (uint8_t*)malloc( sizeof(uint8_t) * iDataSize );

	pData[0] = id;
	memcpy( pData + 1, pBuf, *size );

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, pData, iDataSize, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock );
	free( pData );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch ( cmd & 0xFFFF )
	{
	case TCON_CMD_INIT:
	case TCON_CMD_STATUS:
	case TCON_CMD_DOOR_STATUS:
	case TCON_CMD_LVDS_STATUS:
	case TCON_CMD_BOOTING_STATUS:
	case TCON_CMD_OPEN_NUM:
	case TCON_CMD_OPEN_POS:
	case TCON_CMD_SHORT_NUM:
	case TCON_CMD_SHORT_POS:
		return TCONCmdRead( id, cmd, pBuf, size );

	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:
	case TCON_CMD_VERSION:
	case TCON_CMD_MASTERING_RD:
		return TCONCmdRead( id, cmd, pBuf[0], pBuf, size );

	case TCON_CMD_MODE_NORMAL:
	case TCON_CMD_MODE_LOD:
		return TCONCmdWrite( id, cmd );

	case TCON_CMD_PATTERN_RUN:
	case TCON_CMD_PATTERN_STOP:
	case TCON_CMD_INPUT_SOURCE:
		return TCONCmdWrite( id, cmd, pBuf[0], pBuf[1] );

	case TCON_CMD_MASTERING_WR:
	case TCON_CMD_QUALITY:
	case TCON_CMD_WRITE_CONFIG:
		return TCONCmdWrite( id, cmd, pBuf[0], pBuf[1], pBuf[2] );

	case TCON_CMD_TGAM_R:
	case TCON_CMD_TGAM_G:
	case TCON_CMD_TGAM_B:
	case TCON_CMD_DGAM_R:
	case TCON_CMD_DGAM_G:
	case TCON_CMD_DGAM_B:
	case TCON_CMD_DOT_CORRECTION:
		return TCONCmdWrite( id, cmd, pBuf, size );

	case TCON_CMD_MULTI:
		return TCONCmdWrite( id, cmd, pBuf[0], pBuf[1] );

	default:
		return -1;
	}

	return 0;
}


//------------------------------------------------------------------------------
//
//	PFPGA Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACmdRead( uint32_t cmd, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACmdWrite( uint32_t cmd, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, pBuf, *size, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACmdWrite( uint32_t cmd, uint8_t buf1, uint8_t buf2, uint8_t buf3 )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;

	uint8_t data[3] = { buf1, buf2, buf3 };

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACmdSource( uint8_t source )
{
	int32_t clntSock, sendSize, ret=0;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", IPC_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_SOURCE, &source, sizeof(source), m_SendBuf, sizeof(m_SendBuf) );

	//	Write Command
	write( clntSock, m_SendBuf, sendSize );

	//	Read Response
	read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACmdVersion( uint8_t *pVersion )
{
	int32_t clntSock, sendSize, ret=0;
	int32_t recvSize;
	uint32_t key, cmd;
	void *payload;
	int32_t payloadSize;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", IPC_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_VERSION, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );

	//	Write Command
	write( clntSock, m_SendBuf, sendSize );

	//	Read Response
	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy(pVersion, payload, 4);

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch ( cmd & 0xFFFF )
	{
	case PFPGA_CMD_STATUS :
	case PFPGA_CMD_UNIFORMITY_RD :
		return PFPGACmdRead( cmd, pBuf, size );

	case PFPGA_CMD_UNIFORMITY_WR :
	case PFPGA_CMD_UNIFORMITY_DATA :
	case PFPGA_CMD_MUTE :
		return PFPGACmdWrite( cmd, pBuf, size );

	case PFPGA_CMD_WRITE_CONFIG:
		return PFPGACmdWrite( cmd, pBuf[0], pBuf[1], pBuf[2] );

	case PFPGA_CMD_SOURCE :
		return PFPGACmdSource( pBuf[0] );

	case PFPGA_CMD_VERSION :
		return PFPGACmdVersion( pBuf );

	default :
		return -1;
	}

	return 0;
}


//------------------------------------------------------------------------------
//
//	Battery Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::BATCmdStatus( uint8_t *pBuf, int32_t * /*size*/ )
{
	int32_t clntSock, sendSize, recvSize, payloadSize, ret = 0;
	uint32_t key, cmd;
	void *payload;
	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", IPC_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, BAT_CMD_STATUS, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );
	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );

	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy(pBuf, payload, 8);

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch( cmd )
	{
	case BAT_CMD_STATUS:
		return BATCmdStatus( pBuf, size );
	default:
		return -1;
	}
	return 0;
}


//------------------------------------------------------------------------------
//
//	IMB Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::IMBCmdStatus( uint8_t *status, int32_t *size )
{
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key, cmd;
	void *payload;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, IMB_CMD_STATUS, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( status, payload, payloadSize );
	*size = payloadSize;

	close( clntSock);
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch( cmd )
	{
	case IMB_CMD_STATUS:
	case IMB_CMD_VERSION:
		return IMBCmdStatus( pBuf, size );
	default:
		return -1;
	}
	return 0;
}


//------------------------------------------------------------------------------
//
//	IMB Control APIs
//

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::IPCCmdServerVersion( uint32_t cmd, uint8_t *pBuf, int32_t *size )
{
	int32_t clntSock;
	int32_t sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(IPC_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", IPC_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, NULL, 0, m_SendBuf, sizeof(m_SendBuf) );
	write( clntSock, m_SendBuf, sendSize );

	recvSize = read( clntSock, m_ReceiveBuf, sizeof(m_ReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_ReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket().\n" );
		close( clntSock );
		return -1;
	}

	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		close( clntSock );
		return -1;
	}

	memcpy( pBuf, payload, payloadSize );
	*size = payloadSize;

	close( clntSock);
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::IPCCmdClientVerison( uint32_t /*cmd*/, uint8_t *pBuf, int32_t *size )
{
	uint8_t version[1024];
	sprintf( (char*)version, "%s, %s", __TIME__, __DATE__ );

	int32_t payloadSize = (int32_t)strlen((const char*)version);
	if( *size < payloadSize )
	{
		NxErrMsg( "Error: Buffer size(%d) < Payload Size(%d).", *size, payloadSize );
		return -1;
	}

	memcpy( pBuf, version, payloadSize );
	*size = payloadSize;

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IPCClient::IPCCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch( cmd )
	{
	case IPC_CMD_VERSION_SERVER:
		return IPCCmdServerVersion( cmd, pBuf, size );
	case IPC_CMD_VERSION_CLIENT:
		return IPCCmdClientVerison( cmd, pBuf, size );
	default:
		return -1;
	}
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
int32_t NX_TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->TCONCommand( id, cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->PFPGACommand( cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->BATCommand( cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->IMBCommand( cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_IPCCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	CNX_IPCClient *hIpc = CNX_IPCClient::GetInstance();
	return hIpc->IPCCommand( cmd, pBuf, size );
}
