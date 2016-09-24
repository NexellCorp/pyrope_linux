//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
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
#include <unistd.h>	//	usleep
#include <stdlib.h>	//	atoi
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <CNX_Uart.h>
#include <NX_UartProtocol.h>
#include <NX_Utils.h>
#include <NX_Queue.h>
#include <Board_Port.h>
#include <CNX_BaseClass.h>

#include <NX_GDCServer.h>

#define NX_DTAG "[S.AP Client]"
#include <NX_DbgMsg.h>


class SapGpio : public CNX_Thread
{
public:
	SapGpio();
	virtual ~SapGpio();

	//	Implement Pure Virtual Function
	virtual void ThreadProc();

	int32_t StartMonitor();
	void StopMonitor();

	void RegisterGpioCallback( void (*callback)( void *, uint32_t , uint32_t ), void * );

public:
	CNX_GpioControl m_Request;
	CNX_GpioControl m_BootOk0;
	CNX_GpioControl m_BootOk1;

	CNX_GpioControl m_DoorTemper;	//	Temper

	enum { REQUEST, BOOT0, BOOT1, DOOR, GPIO_MAX_VAL };

private:
	void *m_pCbPrivate;
	void (*m_Callback)( void *, uint32_t , uint32_t );
	int32_t m_GpioStatus[GPIO_MAX_VAL];
	CNX_GpioControl *m_hGpio[GPIO_MAX_VAL];
};


SapGpio::SapGpio()
	: m_pCbPrivate(NULL)
	, m_Callback(NULL)
{
	m_Request.Init(UART_REQUEST);
	m_BootOk0.Init(BOOT_OK_0);
	m_BootOk1.Init(BOOT_OK_1);
	m_DoorTemper.Init(DOOR_TEMPER);
	m_hGpio[0] = &m_Request;
	m_hGpio[1] = &m_BootOk0;
	m_hGpio[2] = &m_BootOk1;
	m_hGpio[3] = &m_DoorTemper;

	for( int32_t i=0 ; i<GPIO_MAX_VAL ; i++ )
	{
		m_hGpio[i]->SetDirection(GPIO_DIRECTION_IN);
		m_GpioStatus[i] = 1;	//	 Init All 1
	}
}

SapGpio::~SapGpio()
{
}

int32_t SapGpio::StartMonitor()
{
	Start();
	return 0;
}

void SapGpio::StopMonitor()
{
	Stop();
}

void SapGpio::ThreadProc()
{
	int32_t value;
	// Check Every 100 msec
	while(1)
	{
		usleep(100000);
		for( int32_t i=0 ; i<GPIO_MAX_VAL ; i++ )
		{
			value = m_hGpio[i]->GetValue();
			if( m_GpioStatus[i] != value )
			{
				if( value )
				{
					//	Button Up
					printf("Button Up\n");
				}
				else
				{
					//	Button Down
					printf("Button Down\n");
				}
				if( m_Callback )
				{
					m_Callback( m_pCbPrivate, i, value );
				}

				m_GpioStatus[i] = value;
			}
		}
	}
}


void SapGpio::RegisterGpioCallback( void (*callback)( void *, uint32_t , uint32_t ), void *pCbPrivate )
{
	m_pCbPrivate = pCbPrivate;
	m_Callback = callback;
}


class SlinkClient : public CNX_Thread
{
public:
	SlinkClient();
	virtual ~SlinkClient();

	int32_t StartService();
	void StopService();
	int32_t  AddCommand( int32_t cmd );


	//
	//	Implementation Pure Virtual Function
	//
	virtual void ThreadProc();

private:
	CNX_Uart *m_hUart;
	bool m_bUartInit;
	bool m_ExitLoop;

	//
	NX_QUEUE m_CmdQ;

	//
	uint8_t m_SendBuf[4096];

	//	Frame Counter
	uint32_t m_FrameCounter;
};


SlinkClient::SlinkClient()
	: m_hUart(NULL)
	, m_bUartInit(false)
	, m_FrameCounter(0)
{
	m_hUart = new CNX_Uart();
	m_bUartInit = m_hUart->Init( 0 );
	NX_InitQueue( &m_CmdQ, 128 );
}


SlinkClient::~SlinkClient()
{
	delete m_hUart;
}


void SlinkClient::ThreadProc()
{
	uint32_t cmd;
	NX_UART_PACKET pkt;
	int32_t sendBufSize, written, sendSize;

	while(!m_ExitLoop)
	{
		if( NX_GetQueueCnt(&m_CmdQ) <= 0 )
		{
			usleep( 1000 );	//	Wait 1 msec
			continue;
		}

		NX_PopQueue( &m_CmdQ, (void**)&cmd );

		//	Check Command Queue
		switch( cmd )
		{
			case CMD_BOOT_DONE:
			{
				NX_InitPacket( &pkt );
				pkt.command = CMD_BOOT_DONE;
				pkt.payload = NULL;
				pkt.payloadSize = 0;
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				NxDbgMsg( NX_DBG_VBS, "CMD_BOOT_DONE\n" );
				break;
			}
			case CMD_ALIVE:
			{
				NX_InitPacket( &pkt );
				pkt.command = CMD_ALIVE;
				pkt.payload = NULL;
				pkt.payloadSize = 0;
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				NxDbgMsg( NX_DBG_VBS, "CMD_ALIVE\n" );
				break;
			}
			case CMD_MARRIAGE_STATE:
			{
				uint32_t state = m_FrameCounter;
				NX_InitPacket( &pkt );
				pkt.command = CMD_MARRIAGE_STATE;
				pkt.payload = &state;
				pkt.payloadSize = sizeof(state);
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				NxDbgMsg( NX_DBG_VBS, "CMD_MARRIAGE_STATE\n" );
				break;
			}
			default:
				break;
		}
	}
}


int32_t SlinkClient::StartService()
{
	m_ExitLoop = false;
	Start();
	return 0;
}


void SlinkClient::StopService()
{
	m_ExitLoop = true;
	Stop();
}

int32_t  SlinkClient::AddCommand( int32_t cmd )
{
	//	 Push Command
	return NX_PushQueue( &m_CmdQ, (void*)cmd );
}




//
//	Main Application Part
//

#define SERVER_PRIV_FILE	"/mnt/mmc/bin/cert/leaf.key"
#define SERVER_CERT_FILE	"/mnt/mmc/bin/cert/leaf.signed.pem"

#define TCP_PORT			43684

SlinkClient *gstSlinkClient = NULL;
SapGpio *gstSapGpio = NULL;

//------------------------------------------------------------------------------
static void signal_handler( int32_t signal )
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(signal), signal);

	switch( signal )
	{
		case SIGINT :
			printf("SIGINT..\n"); 	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	NX_MarriageServerStop();

	if( gstSlinkClient )
	{
		gstSlinkClient->StopService();
		delete gstSlinkClient;
	}

	if( gstSapGpio )
	{
		delete gstSapGpio;	
	}

	exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

//------------------------------------------------------------------------------
static int32_t MarriageCallbackFunction( void *pObj, int32_t iEventCode, void *pData, int32_t iSize )
{
	switch( iEventCode )
	{
		case EVENT_RECEIVE_CERTIFICATE:
			printf("MSG1 Success.\n");
			break;
		case EVENT_ACK_CERTIFICATE:
			printf("RSP1 Success.\n");
			break;
		case EVENT_RECEIVE_PLANE_DATA:
			printf("MSG2 Success.\n");
			break;
		case EVENT_ACK_SIGN_PLANE_TEXT:
			printf("RSP2 Success.\n");
			break;
		case EVENT_RECEIVE_MARRIAGE_OK:
			printf("MSG3 Success.\n");
			break;
		case EVENT_ACK_MARRIAGE_OK:
			printf("RSP3 Sucess.\n");
			break;
		case ERROR_MAKE_PACKET:
		case ERROR_SIGN_PLANE_TEXT:
			printf("Error, Marriage.\n");
			break;
		default:
			break;
	}

	return 0;
}

//------------------------------------------------------------------------------
static void GpioCallbackFunction( void *pPrivate, uint32_t gpioPort, uint32_t value )
{
	NxDbgMsg( NX_DBG_DEBUG, "gpioPort = %d, value = %d\n", gpioPort, value);
	if( 1 == gpioPort )
	{
		if ( value == 0 )
		{
			system("sync");
			system("poweroff");
		}
	}
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	register_signal();

	gstSlinkClient = new SlinkClient();
	gstSapGpio = new SapGpio();

	gstSlinkClient->StartService();

	//	GPIO Start & Register Callback
	gstSapGpio->RegisterGpioCallback(GpioCallbackFunction, gstSapGpio);
	gstSapGpio->StartMonitor();

	gstSlinkClient->AddCommand( CMD_BOOT_DONE );

	// Start Marriage Server
	NX_MarraigeEventCallback( MarriageCallbackFunction, NULL );
	NX_MarriageServerStart( TCP_PORT, SERVER_CERT_FILE, SERVER_PRIV_FILE );

	while( 1 )
	{
		gstSlinkClient->AddCommand( CMD_MARRIAGE_STATE );
		gstSlinkClient->AddCommand( CMD_ALIVE );		//	Alive Command
		usleep( 1000000 );
	}

	gstSlinkClient->StopService();
	delete gstSlinkClient;
	delete gstSapGpio;

	return 0;
}

