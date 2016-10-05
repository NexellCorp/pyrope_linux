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
#include <unistd.h>	//	write, close

#include <tms_protocol.h>
#include <SockUtils.h>

#include <NX_TMSClient.h>
#include <NX_TMSCommand.h>

#define NX_DTAG	"[TMS Client]"
#include <NX_DbgMsg.h>

#define	TMS_SERVER_FILE "/data/local/tmp/tms_server"

//
//	TMS Client API
//
class NX_TMSClient
{
public:
	NX_TMSClient(){};
	virtual ~NX_TMSClient(){};

	static NX_TMSClient* GetInstance( );
	static void ReleaseInstance( );

	//
	//	PFPGA Private Member Functions
	//
private:
	int32_t PFPGACmdStatus( uint8_t *status, int *size );
	int32_t PFPGACmdSetSource( uint8_t index );
	int32_t PFPGACmdVersion( uint8_t *pVersion );
public:
	int32_t PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size );

	//
	//	TCON Private Member Functions
	//
private:
	int32_t TCONCmdStatus( int32_t id, uint32_t cmd, uint8_t *status, int32_t *size );
	int32_t TCONCmdLedPosition( int32_t id, uint32_t cmd, uint8_t *pInfo, int32_t *size );
	int32_t TCONCmdLedOn( int32_t id, uint32_t cmd, uint8_t onOff );
	int32_t TCONCmdBrightnessControl( uint8_t mode, uint8_t evtMode );
	int32_t TCONCmdElapsedTime( uint8_t *pBuf );
	int32_t TCONCmdVersion( uint8_t *pVersion );

public:
	int32_t TConCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size );

	//
	//	Battery Member Functions
	//
private:
	int32_t BATCmdState( uint8_t *pBuf, int32_t *size );
public:
	int32_t BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );


	//
	//	Rx/Tx Buffer
	//
private:
	uint8_t m_TConSendBuf[64*1024];
	uint8_t m_TConReceiveBuf[64*1024];
	uint8_t m_PFPGASendBuf[64*1024];
	uint8_t m_PFPGAReceiveBuf[64*1024];

	//
	//	for Singleton
	//
private:
	static NX_TMSClient	*m_psInstance;
};

NX_TMSClient* NX_TMSClient::m_psInstance = NULL;

//////////////////////////////////////////////////////////////////////////////
//																			//
//							PFPGA Control APIs								//
//																			//
//////////////////////////////////////////////////////////////////////////////
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
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_STATE, NULL, 0, m_PFPGASendBuf, sizeof(m_PFPGASendBuf) );
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

int32_t NX_TMSClient::PFPGACmdSetSource( uint8_t index )
{
	int32_t clntSock, sendSize, ret=0;

	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, PFPGA_CMD_SOURCE, &index, sizeof(index), m_PFPGASendBuf, sizeof(m_PFPGASendBuf) );

	//	Write Command
	write( clntSock, m_PFPGASendBuf, sendSize );

	//	Read Response
	read( clntSock, m_PFPGAReceiveBuf, sizeof(m_PFPGAReceiveBuf) );

	close( clntSock);
	return ret;
}

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

int32_t NX_TMSClient::PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch ( cmd&0xFFFF )
	{
		case PFPGA_CMD_STATE :
			return PFPGACmdStatus( pBuf, size );
		case PFPGA_CMD_SOURCE :
			return PFPGACmdSetSource( pBuf[0] );
		case PFPGA_CMD_VERSION :
			return PFPGACmdVersion( pBuf );
		default :
			return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//							TCON Control APIs								//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t NX_TMSClient::TCONCmdStatus( int32_t id, uint32_t cmd, uint8_t *status, int32_t *size )
{
	int32_t clntSock, sendSize, recvSize, payloadSize, ret=0;
	uint32_t key;
	void *payload;
	(void)id;
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	status[0] = id;
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, status, *size, m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );
	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );

	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}

	memcpy( status, payload, payloadSize );

	close( clntSock);
	return ret;
}

int32_t NX_TMSClient::TCONCmdLedPosition( int32_t id, uint32_t cmd, uint8_t *pInfo, int32_t *size )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;
	(void)id;
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : socket (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, pInfo, *size, m_TConSendBuf, sizeof(m_TConSendBuf) );
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

int32_t NX_TMSClient::TCONCmdLedOn( int32_t id, uint32_t cmd, uint8_t onOff )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;
	(void)id;
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, cmd, &onOff, 1, m_TConSendBuf, sizeof(m_TConSendBuf) );
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

int32_t NX_TMSClient::TCONCmdBrightnessControl( uint8_t mode, uint8_t evtMode )
{
	int32_t ret = 0;
	int32_t clntSock;
	int32_t sendSize, recvSize;
	uint8_t modeBuf[2] = {mode, evtMode};
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock)
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, TCON_CMD_BR_CTRL, &modeBuf, 2, m_TConSendBuf, sizeof(m_TConSendBuf) );
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


int32_t NX_TMSClient::TCONCmdElapsedTime( uint8_t *pBuf )
{
	int32_t clntSock, sendSize, recvSize, ret = 0;
	clntSock = LS_Connect(TMS_SERVER_FILE);
	if( -1 == clntSock )
	{
		NxErrMsg( "Error : LS_Connect (%s)\n", TMS_SERVER_FILE);
		return -1;
	}
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, TCON_CMD_ELAPSED_TIME, NULL, 0, m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );
	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );

	uint32_t key, cmd;
	void *payload;
	int32_t payloadSize;
	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy( pBuf, payload, payloadSize );
	
	close( clntSock);
	return ret;
}


int32_t NX_TMSClient::TCONCmdVersion( uint8_t *pVersion )
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
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, TCON_CMD_VERSION, NULL, 0, m_TConSendBuf, sizeof(m_TConSendBuf) );
	write( clntSock, m_TConSendBuf, sendSize );
	recvSize = read( clntSock, m_TConReceiveBuf, sizeof(m_TConReceiveBuf) );

	if( 0 != TMS_ParsePacket( m_TConReceiveBuf, recvSize, &key, &cmd, &payload, &payloadSize ) )
	{
		NxErrMsg( "Error : TMS_ParsePacket\n" );
		ret = -1;
	}
	memcpy(pVersion, payload, 4);

	close( clntSock);
	return ret;
}

int32_t NX_TMSClient::TConCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	// bool bAll = ( id == -1 );
	switch ( cmd&0xFFFF )
	{
		case TCON_CMD_STATE:
		case TCON_CMD_OPEN:
		case TCON_CMD_SHORT:
		case TCON_CMD_DOOR:
			return TCONCmdStatus( id, cmd&0xFF, pBuf, size );
		case TCON_CMD_OPEN_POS:
		case TCON_CMD_SHORT_POS:
			return TCONCmdLedPosition( id, cmd&0xFF, pBuf, size );
		case TCON_CMD_ON:
			return TCONCmdLedOn( id, cmd, pBuf[0] );
		case TCON_CMD_BR_CTRL:
			return TCONCmdBrightnessControl( pBuf[0], pBuf[1] );
		case TCON_CMD_ELAPSED_TIME:
			return TCONCmdElapsedTime( pBuf );
		case TCON_CMD_VERSION:
			return TCONCmdVersion( pBuf );
		default:
			return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//							Battery Control APIs							//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t NX_TMSClient::BATCmdState( uint8_t *pBuf, int32_t * /*size*/ )
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
	sendSize = TMS_MakePacket( TMS_KEY_VALUE, BAT_CMD_STATE, NULL, 0, m_TConSendBuf, sizeof(m_TConSendBuf) );
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

int32_t NX_TMSClient::BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	switch( cmd )
	{
	case BAT_CMD_STATE:
		return BATCmdState( pBuf, size );
	default:
		return -1;
	}
	return 0;
}


//
//	For Singleton
//
NX_TMSClient* NX_TMSClient::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new NX_TMSClient();
	}
	return (NX_TMSClient*)m_psInstance;
}

void NX_TMSClient::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}



//////////////////////////////////////////////////////////////////////////////
//																			//
//							External Control APIs							//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t NX_TConCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->TConCommand( id, cmd, pBuf, size );
}

int32_t NX_PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->PFPGACommand( cmd, pBuf, size );
}

int32_t NX_BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size )
{
	NX_TMSClient *hTms = NX_TMSClient::GetInstance();
	return hTms->BATCommand( cmd, pBuf, size );
}
