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
#include <unistd.h>

#include <tms_protocol.h>
#include <SockUtils.h>

#include <NX_TMSClient.h>
#include <NX_TMSCommand.h>

#define NX_DTAG	"[TMS Client]"
#include <NX_DbgMsg.h>

#define	TMS_SERVER_FILE "/data/local/tmp/tms_server"

//------------------------------------------------------------------------------
//
//	TMS Client APIs
//
class NX_TMSClient
{
public:
	NX_TMSClient(){};
	virtual ~NX_TMSClient(){};

	static NX_TMSClient* GetInstance( );
	static void ReleaseInstance( );

public:
	int32_t TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
	int32_t IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );

private:
	//	TCON Commands
	int32_t TCONCmdStatus( int32_t id, uint32_t cmd, uint8_t *status, int32_t *size );
	int32_t TCONCmdMode( int32_t id, uint32_t cmd );
	int32_t TCONCmdInfo( int32_t id, uint32_t cmd, uint8_t *pInfo, int32_t *size );
	int32_t TCONCmdControl( uint32_t id, uint32_t cmd, uint8_t value1, uint8_t value2 );
	int32_t TCONCmdControl( uint32_t id, uint32_t cmd, uint8_t value1, uint8_t value2, uint8_t value3 );

	//	PFPGA Commands
	int32_t PFPGACmdStatus( uint8_t *status, int *size );
	int32_t PFPGACmdSource( uint8_t index );
	int32_t PFPGACmdVersion( uint8_t *pVersion );

	//	Battery Commands
	int32_t BATCmdStatus( uint8_t *pBuf, int32_t *size );

	//	IMB Commands
	int32_t IMBCmdStatus( uint8_t *pBuf, int32_t *size );

private:
	//	Rx/Tx Buffer
	uint8_t m_TConSendBuf[64*1024];
	uint8_t m_TConReceiveBuf[64*1024];
	uint8_t m_PFPGASendBuf[64*1024];
	uint8_t m_PFPGAReceiveBuf[64*1024];

private:
	//	For Singleton
	static NX_TMSClient	*m_psInstance;
};

NX_TMSClient* NX_TMSClient::m_psInstance = NULL;

//------------------------------------------------------------------------------
//
//	TCON Control APIs
//

//------------------------------------------------------------------------------
int32_t NX_TMSClient::TCONCmdStatus( int32_t id, uint32_t cmd, uint8_t *status, int32_t *size )
{
	uint8_t index = id;
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", TMS_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &index, sizeof(index), m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
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
int32_t NX_TMSClient::TCONCmdMode( int32_t id, uint32_t cmd )
{
	uint8_t index = id;
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", TMS_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &index, sizeof(index), m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		close( clntSock );
		return -1;
	}

	close( clntSock);
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::TCONCmdInfo( int32_t id, uint32_t cmd, uint8_t *pInfo, int32_t *size )
{
	uint8_t index = id;
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key;
	void *payload;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", TMS_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &index, sizeof(index), m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
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

	memcpy( pInfo, payload, payloadSize );
	*size = payloadSize;

	close( clntSock);
	return 0;

}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::TCONCmdControl( uint32_t id, uint32_t cmd, uint8_t value1, uint8_t value2 )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;
	uint8_t data[3] = { (uint8_t)id, value1, value2 };

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::TCONCmdControl( uint32_t id, uint32_t cmd, uint8_t value1, uint8_t value2, uint8_t value3 )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;
	uint8_t data[4] = { (uint8_t)id, value1, value2, value3 };

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}

printf("0x%02x, 0x%02x, 0x%02x, 0x%02x\n", data[0], data[1], data[2], data[3]);

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &data, sizeof(data), m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 > recvSize )
	{
		NxErrMsg( "Fail, read().\n" );
		ret = -1;
	}



	close( clntSock);
	return ret;
}


//------------------------------------------------------------------------------
int32_t NX_TMSClient::TCONCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch ( cmd & 0xFFFF )
	{
	case TCON_CMD_STATUS:
	case TCON_CMD_DOOR_STATUS:
		return TCONCmdStatus( id, cmd & 0xFF, pBuf, size );

	case TCON_CMD_MODE_NORMAL:
	case TCON_CMD_MODE_LOD:
		return TCONCmdMode( id, cmd & 0xFF );

	case TCON_CMD_OPEN_NUM:
	case TCON_CMD_OPEN_POS:
	case TCON_CMD_SHORT_NUM:
	case TCON_CMD_SHORT_POS:
	case TCON_CMD_ELAPSED_TIME:
	case TCON_CMD_ACCUMULATE_TIME:
	case TCON_CMD_VERSION:
		return TCONCmdInfo( id, cmd & 0xFF, pBuf, size );

	case TCON_CMD_PATTERN:
		return TCONCmdControl( id, cmd & 0xFF, pBuf[0], pBuf[1] );

	case TCON_CMD_MASTERING:
		return TCONCmdControl( id, cmd & 0xFF, pBuf[0], pBuf[1], pBuf[2] );

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
int32_t NX_TMSClient::PFPGACmdStatus( uint8_t *status, int32_t * /*size*/ )
{
	int32_t clntSock, sendSize, ret = 0;
	int32_t recvSize;
	uint32_t key, cmd;
	void *payload;
	int32_t payloadSize;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_STATUS, NULL, 0, m_PFPGASendBuf, sizeof(m_PFPGASendBuf) );
	//	Write Command
	write( clntSock, m_PFPGASendBuf, sendSize );

	//	Read Response
	recvSize = read( clntSock, m_PFPGAReceiveBuf, sizeof(m_PFPGAReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_PFPGAReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	status[0] = ((uint8_t*)payload)[0];
	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::PFPGACmdSource( uint8_t source )
{
	int32_t clntSock, sendSize, ret=0;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_SOURCE, &source, sizeof(source), m_PFPGASendBuf, sizeof(m_PFPGASendBuf) );

	//	Write Command
	write( clntSock, m_PFPGASendBuf, sendSize );

	//	Read Response
	read( clntSock, m_PFPGAReceiveBuf, sizeof(m_PFPGAReceiveBuf) );

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::PFPGACmdVersion( uint8_t *pVersion )
{
	int32_t clntSock, sendSize, ret=0;
	int32_t recvSize;
	uint32_t key, cmd;
	void *payload;
	int32_t payloadSize;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_VERSION, NULL, 0, m_PFPGASendBuf, sizeof(m_PFPGASendBuf) );

	//	Write Command
	write( clntSock, m_PFPGASendBuf, sendSize );

	//	Read Response
	recvSize = read( clntSock, m_PFPGAReceiveBuf, sizeof(m_PFPGAReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_PFPGAReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy(pVersion, payload, 4);

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch ( cmd & 0xFFFF )
	{
	case PFPGA_CMD_STATUS :
		return PFPGACmdStatus( pBuf, size );

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
int32_t NX_TMSClient::BATCmdStatus( uint8_t *pBuf, int32_t * /*size*/ )
{
	int32_t clntSock, sendSize, recvSize, payloadSize, ret = 0;
	uint32_t key, cmd;
	void *payload;
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, BAT_CMD_STATUS, NULL, 0, m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );
	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );

	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy(pBuf, payload, 8);

	close( clntSock);
	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_TMSClient::BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
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
int32_t NX_TMSClient::IMBCmdStatus( uint8_t *status, int32_t *size )
{
	int32_t clntSock, sendSize, recvSize, payloadSize;
	uint32_t key, cmd;
	void *payload;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect(). ( %s )\n", TMS_SERVER_FILE);
		return -1;
	}

	sendSize = TMS_MakePacket( TMS_KEY_VALUE, IMB_CMD_STATUS, NULL, 0, m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );

	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );
	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
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
int32_t NX_TMSClient::IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
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
//	For Singleton
//

//------------------------------------------------------------------------------
NX_TMSClient* NX_TMSClient::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new NX_TMSClient();
	}
	return (NX_TMSClient*)m_psInstance;
}

//------------------------------------------------------------------------------
void NX_TMSClient::ReleaseInstance()
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
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->TCONCommand( id, cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->PFPGACommand( cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->BATCommand( cmd, pBuf, size );
}

//------------------------------------------------------------------------------
int32_t NX_IMBCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->IMBCommand( cmd, pBuf, size );
}