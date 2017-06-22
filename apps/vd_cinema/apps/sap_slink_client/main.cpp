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
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

#include <Board_Port.h>
#include <NX_Version.h>
#include <NX_UartProtocol.h>
#include <NX_Utils.h>
#include <NX_Queue.h>
#include <CNX_Uart.h>
#include <CNX_BaseClass.h>

#include <NX_GDCServer.h>

#define NX_DTAG "[S.AP Client]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
class SapGpio : public CNX_Thread
{
public:
	SapGpio()
		: m_bThreadRun( false )
		, m_pCbPrivate(NULL)
		, m_Callback(NULL)
	{
		m_hGpio[BOOT_OK_0] = new CNX_GpioControl();
		m_hGpio[BOOT_OK_0]->Init(GPIO_BOOT_OK_0);

		m_hGpio[BOOT_OK_1] = new CNX_GpioControl();
		m_hGpio[BOOT_OK_1]->Init(GPIO_BOOT_OK_1);

		for( int32_t i = 0 ;i < GPIO_MAX_VAL; i++ )
		{
			m_hGpio[i]->SetDirection( GPIO_DIRECTION_IN );
			m_GpioStatus[i] = 1;
		}
	}
	virtual ~SapGpio()
	{
		StopMonitor();

		for( int32_t i = 0; i < GPIO_MAX_VAL; i++ )
		{
			m_hGpio[i]->Deinit();
			delete m_hGpio[i];
		}
	}

	//	Implement Pure Virtual Function
	virtual void ThreadProc()
	{
		while(m_bThreadRun)
		{
			usleep(100000);
			for( int32_t i=0 ; i<GPIO_MAX_VAL ; i++ )
			{
				int32_t value = m_hGpio[i]->GetValue();
				if( 0 > value ) continue;

				if( m_GpioStatus[i] != value )
				{
					printf( "Detect Gpio %s.\n", value ? "High" : "Low" );

					if( m_Callback ) m_Callback( m_pCbPrivate, i, value );
					m_GpioStatus[i] = value;
				}
			}
		}
	}

	int32_t StartMonitor()
	{
		m_bThreadRun = true;
		Start();
		return 0;
	}

	void StopMonitor()
	{
		if( true == m_bThreadRun )
		{
			m_bThreadRun = false;
			Stop();
		}
	}

	void RegisterGpioCallback( void (*callback)( void *, uint32_t , uint32_t ), void *pCbPrivate )
	{
		m_pCbPrivate = pCbPrivate;
		m_Callback = callback;
	}

private:
	enum { BOOT_OK_0, BOOT_OK_1, GPIO_MAX_VAL };

	bool m_bThreadRun;
	void *m_pCbPrivate;
	void (*m_Callback)( void *, uint32_t , uint32_t );

	CNX_GpioControl *m_hGpio[GPIO_MAX_VAL];
	int32_t m_GpioStatus[GPIO_MAX_VAL];
};

//------------------------------------------------------------------------------
class SlinkClient : public CNX_Thread
{
public:
	SlinkClient()
		: m_FrameCounter(0)
	{
		m_hUart = new CNX_Uart();
		m_hUart->Init( 0 );
		NX_InitQueue( &m_CmdQ, 128 );
	}

	virtual ~SlinkClient()
	{
		delete m_hUart;
	}

public:
	int32_t StartService()
	{
		m_ExitLoop = false;
		Start();
		return 0;
	}

	void StopService()
	{
		m_ExitLoop = true;
		Stop();
	}

	int32_t AddCommand( int32_t cmd )
	{
		return NX_PushQueue( &m_CmdQ, (void*)cmd );
	}

	//	Implementation Pure Virtual Function
	virtual void ThreadProc();

private:
	CNX_Uart *m_hUart;
	bool m_ExitLoop;

	NX_QUEUE m_CmdQ;

	uint8_t m_SendBuf[4096];
	uint32_t m_FrameCounter;	// Frame counter
};

//------------------------------------------------------------------------------
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
				if( 0 >= written) {
					NxDbgMsg( NX_DBG_VBS, "Fail, write().\n" );
				}
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
				if( 0 >= written) {
					NxDbgMsg( NX_DBG_VBS, "Fail, write().\n" );
				}
				NxDbgMsg( NX_DBG_VBS, "CMD_ALIVE\n" );
				break;
			}

			case CMD_MARRIAGE:
			{
				NX_InitPacket( &pkt );
				pkt.command = CMD_MARRIAGE;
				pkt.payload = NULL;
				pkt.payloadSize = 0;
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				if( 0 >= written) {
					NxDbgMsg( NX_DBG_VBS, "Fail, write().\n" );
				}
				NxDbgMsg( NX_DBG_VBS, "CMD_MARRIAGE\n" );
				break;
			}

			case CMD_DIVORCE:
			{
				NX_InitPacket( &pkt );
				pkt.command = CMD_DIVORCE;
				pkt.payload = NULL;
				pkt.payloadSize = 0;
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				if( 0 >= written) {
					NxDbgMsg( NX_DBG_VBS, "Fail, write().\n" );
				}
				NxDbgMsg( NX_DBG_VBS, "CMD_DIVORCE\n" );
			}

			case CMD_VERSION:
			{
				char version[64] = { 0x00, };
				sprintf( version, "%s", NX_VERSION_SAP );

				NX_InitPacket( &pkt );
				pkt.command = CMD_VERSION;
				pkt.payload = (void*)version;
				pkt.payloadSize = strlen(version);
				sendBufSize = NX_GetSendBufferSize( &pkt );
				pkt.frameNumber = m_FrameCounter++;
				sendSize = NX_MakeUartPacket(&pkt, m_SendBuf, sendBufSize );
				written = m_hUart->Write(m_SendBuf, sendSize);
				if( 0 >= written) {
					NxDbgMsg( NX_DBG_VBS, "Fail, write().\n" );
				}
				NxDbgMsg( NX_DBG_VBS, "CMD_VERSION\n" );
			}

			default:
				break;
		}
	}
}


//
//	Main Application Part
//

#define SERVER_PRIV_FILE	"/mnt/mmc/bin/cert/leaf.key"
#define SERVER_CERT_FILE	"/mnt/mmc/bin/cert/leaf.signed.pem"

#define TCP_PORT			43684

static SlinkClient *gstSlinkClient = NULL;
static SapGpio *gstSapGpio = NULL;

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

	if( gstSapGpio )
	{
		gstSapGpio->StopMonitor();
		delete gstSapGpio;
	}

	if( gstSlinkClient )
	{
		gstSlinkClient->StopService();
		delete gstSlinkClient;
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
static int32_t MarriageSimpleCallbackFunction( void *pObj, int32_t iEventCode, void *pData, int32_t iSize )
{
	int32_t *bMarriaged = (int32_t*)pObj;

	*bMarriaged = false;

	switch( iEventCode )
	{
		case EVENT_ACK_MARRIAGE_OK:
			printf("Marriage Success.\n");
			*bMarriaged = true;
			break;

		case ERROR_MAKE_PACKET:
		case ERROR_SIGN_PLANE_TEXT:
			printf("Fail, Marriage.\n");
			break;

		default:
			break;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t MarriageVerboseCallbackFunction( void *pObj, int32_t iEventCode, void *pData, int32_t iSize )
{
	int32_t *bMarriaged = (int32_t*)pObj;

	*bMarriaged= false;

	switch( iEventCode )
	{
	case EVENT_RECEIVE_CERTIFICATE :
		printf("======================================================================\n");
		printf(">> Receive Ceriticate.\n");
		printf("======================================================================\n");
		printf("%s", (char*)pData);
		printf("======================================================================\n");
		break;

	case EVENT_RECEIVE_PLANE_DATA :
		printf("======================================================================\n");
		printf(">> Receive Plane Data.\n");
		printf("======================================================================\n");
		NX_HexDump( pData, iSize );
		printf("======================================================================\n");
		break;

	case EVENT_RECEIVE_MARRIAGE_OK :
		printf("======================================================================\n");
		printf(">> Receive Marriage OK.\n");
		printf("======================================================================\n");
		break;

	case EVENT_ACK_CERTIFICATE :
		printf("======================================================================\n");
		printf(">> Transfer Ceriticate.\n");
		printf("======================================================================\n");
		NX_HexDump( pData, iSize );
		printf("======================================================================\n");
		break;

	case EVENT_ACK_SIGN_PLANE_TEXT :
		printf("======================================================================\n");
		printf(">> Transfer Sign Data.\n");
		printf("======================================================================\n");
		NX_HexDump( pData, iSize );
		printf("======================================================================\n");
		break;

	case EVENT_ACK_MARRIAGE_OK :
		printf("======================================================================\n");
		printf(">> Transfer Marriage OK.\n");
		printf("======================================================================\n");
		NX_HexDump( pData, iSize );
		printf("======================================================================\n");
		*bMarriaged = true;
		break;

	case ERROR_MAKE_PACKET :
		printf("Error, Make Packet.\n");
		break;

	case ERROR_SIGN_PLANE_TEXT :
		printf("Error, Sign Data.\n");
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
		if( 0 == value )
		{
			sync();
			reboot( LINUX_REBOOT_CMD_POWER_OFF );
		}
	}
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;
	int32_t bVerbose = false;
	int32_t bMarriaged = false;

	while( -1 != (opt=getopt(argc, argv, "v")))
	{
		switch( opt )
		{
		case 'v' :
			bVerbose = true;
			break;
		default :
			break;
		}
	}

	register_signal();

	gstSlinkClient = new SlinkClient();
	gstSapGpio = new SapGpio();

	gstSlinkClient->StartService();
	gstSlinkClient->AddCommand( CMD_BOOT_DONE );

	//	GPIO Start & Register Callback
	gstSapGpio->RegisterGpioCallback(GpioCallbackFunction, gstSapGpio);
	gstSapGpio->StartMonitor();

	// Start Marriage Server
	if( !bVerbose )	NX_MarraigeEventCallback( MarriageSimpleCallbackFunction, &bMarriaged );
	else			NX_MarraigeEventCallback( MarriageVerboseCallbackFunction, &bMarriaged );

	NX_MarriageServerStart( TCP_PORT, SERVER_CERT_FILE, SERVER_PRIV_FILE );

	while( 1 )
	{
		if( !bMarriaged )	gstSlinkClient->AddCommand( CMD_DIVORCE );
		else				gstSlinkClient->AddCommand( CMD_MARRIAGE );

		gstSlinkClient->AddCommand( CMD_ALIVE );
		gstSlinkClient->AddCommand( CMD_VERSION );

		usleep( 1000000 );
	}

	NX_MarriageServerStop();

	gstSapGpio->StopMonitor();
	delete gstSlinkClient;

	gstSlinkClient->StopService();
	delete gstSapGpio;

	return 0;
}

