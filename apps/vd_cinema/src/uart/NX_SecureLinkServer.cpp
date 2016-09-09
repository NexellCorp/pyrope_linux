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


#include <pthread.h>

#define NX_DTAG "[Secure Link]"
#include <NX_DbgMsg.h>
#include <CNX_Uart.h>
#include <NX_UartProtocol.h>
#include <NX_Utils.h>
#include <CNX_BaseClass.h>

class CNX_SLinkServer : protected CNX_Thread
{
public:
	CNX_SLinkServer();
	virtual ~CNX_SLinkServer();

	static CNX_SLinkServer* GetInstance( );
	static void ReleaseInstance( );

	//
	//	Implementation NX_Thread's Pure virtual function
	//
	virtual void ThreadProc();


	int32_t StartService();
	void StopService();
	int32_t GotoSleep();		//	Sleep Command
	void RegisterEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam );
	void RegisterAliveCallback( int32_t (*callback)( void * ), void *pParam );

private:
	int32_t ReadData(uint8_t *pBuf, int32_t size);


private:
	enum{
		MAX_BUFFER_SIZE = 4096,
	};
	bool	m_bExitLoop;
	CNX_Uart *m_hUart;

	int32_t (*m_CbEventCallback)( void *arg, int32_t eventCode, void *data, int32_t size );
	int32_t (*m_CbAliveCallback)( void *arg );
	void *m_hAliveParam;
	void *m_hEvtParam;

	uint8_t m_RxBuf[MAX_BUFFER_SIZE];
	uint8_t m_TxBuf[MAX_BUFFER_SIZE];
	//
	//	for Singleton
	//
private:
	static CNX_SLinkServer	*m_psInstance;
};

CNX_SLinkServer	*CNX_SLinkServer::m_psInstance = NULL;

CNX_SLinkServer::CNX_SLinkServer()
	: m_hAliveParam (NULL)
	, m_hEvtParam (NULL)
{
	m_hUart = new CNX_Uart();
}

CNX_SLinkServer::~CNX_SLinkServer()
{
	delete m_hUart;
	Stop();
}

int32_t CNX_SLinkServer::StartService()
{
	Start();
	return 0;
}

void CNX_SLinkServer::StopService()
{
	m_bExitLoop	= true;
	Stop();
}

int32_t CNX_SLinkServer::ReadData(uint8_t *pBuf, int32_t size)
{
	int32_t readSize, totalSize=0;
	do
	{
		readSize = m_hUart->Read( pBuf, size );
		if( readSize < 0 )
			return -1;

		size -= readSize;
		pBuf += readSize;
		totalSize += readSize;
	}while(size > 0);
	return totalSize;
}

void CNX_SLinkServer::ThreadProc()
{
	uint8_t tmp;
	uint8_t *buf = m_RxBuf;
	NX_UART_PACKET rcvPacket;

	m_hUart->Init( 0 );

	while( !m_bExitLoop )
	{
		uint32_t startCode = 0;
		int32_t readSize, length;
		// memset( buf, 0, sizeof(m_RxBuf));
		//
		//	Find Start Code
		//
		do{
			readSize = ReadData( &tmp, 1 );
			if( readSize != 1 )
			{
				NxErrMsg("Read Error!!!\n");
				continue;
			}
			startCode = (startCode<<8) | tmp;
			if( startCode == UART_START_CODE )
			{
				break;
			}
		}while(1);

		//	Read Length
		readSize = ReadData( buf, 4 );
		if( readSize != 4 )
		{
			NxErrMsg("Read Length Error!!!\n");
			break;
		}
		length = buf[0]<<24 | buf[1]<<8 | buf[2]<<8 | buf[3];

		if( length < 0 || (length > (MAX_BUFFER_SIZE-4)) )
		{
			continue;
		}

		//	Read Remained Data
		readSize = ReadData( buf+4, length );
		if( readSize != length )
		{
			NxErrMsg("Read Error!!!\n");
			break;
		}

		//	Check Data Sanity
		if( 0 != NX_CheckUartPacket( buf+4, length ) )
		{
			NxErrMsg("~~~~~~~~~~~~~~ Packet Error !!!!!\n");
			HexDump( buf, length+4 );
		}

		//	Parsing Uart Command
		if( 0 != NX_ParseUartPacket( buf, length +4, &rcvPacket ) )
		{
			NxErrMsg("Packet Parsing Error");
		}
	}
}

int32_t CNX_SLinkServer::GotoSleep()
{
	return 0;
}


void CNX_SLinkServer::RegisterEventCallback( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	if( callback )
	{
		m_CbEventCallback = callback;
		m_hEvtParam = pParam;
	}
}


void CNX_SLinkServer::RegisterAliveCallback( int32_t (*callback)( void * ), void *pParam )
{
	if( callback )
	{
		m_CbAliveCallback = callback;
		m_hAliveParam = pParam;
	}
}


//
//	For Singleton
//
CNX_SLinkServer* CNX_SLinkServer::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_SLinkServer();
	}
	return (CNX_SLinkServer*)m_psInstance;
}

void CNX_SLinkServer::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}



//////////////////////////////////////////////////////////////////////////////
//																			//
//					External APIs for SecureLink							//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t NX_SLinkServerStart()
{
	CNX_SLinkServer *hSlink = CNX_SLinkServer::GetInstance();
	return hSlink->StartService();
}

void NX_SLinkServerStop()
{
	CNX_SLinkServer *hSlink = CNX_SLinkServer::GetInstance();
	hSlink->StopService();
	CNX_SLinkServer::ReleaseInstance();
}

int32_t NX_SLinkServerGotoSleep()
{
	CNX_SLinkServer *hSlink = CNX_SLinkServer::GetInstance();
	return hSlink->GotoSleep();
}

void NX_SLinkServerRegEventCB( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam )
{
	CNX_SLinkServer *hSlink = CNX_SLinkServer::GetInstance();
	hSlink->RegisterEventCallback( callback, pParam );
}

void NX_SLinkServerRegAliveCB( int32_t (*callback)( void * ), void *pParam )
{
	CNX_SLinkServer *hSlink = CNX_SLinkServer::GetInstance();
	hSlink->RegisterAliveCallback( callback, pParam );
}
