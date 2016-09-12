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

#include <CNX_Uart.h>
#include <NX_UartProtocol.h>
#include <NX_Utils.h>
#include <NX_Queue.h>
#include <CNX_BaseClass.h>


class SlinkClient : public CNX_Thread
{
public:
	SlinkClient();
	virtual ~SlinkClient();

	int32_t StartService();
	void StopService();
	int32_t  AddCommand( int32_t cmd );


	//
	//	Implementation Secure Link Client
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
				printf( "CMD_BOOT_DONE\n" );
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
				printf( "CMD_ALIVE\n" );
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
				printf( "CMD_MARRIAGE_STATE\n" );
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

int32_t main( int32_t argc, char *argv[] )
{
#if 0
	const char *TestString = "1234567890abcdef";
	CNX_Uart *hUart = new CNX_Uart();
	int32_t written;
	uint32_t frameNumber = 0;
	int32_t port = 0; 

	int32_t count = 1;

	if( argc>1 )
	{
		port = atoi(argv[1]);
	}

	if( !hUart->Init( port ) )
	{
		printf("Uart Init Error!!!\n");
		return -1;
	}

	NX_UART_PACKET pkt;
	NX_InitPacket( &pkt );
	pkt.command = 0x12344321;
	pkt.payload = (void *)TestString;
	pkt.payloadSize = strlen(TestString);

	int32_t sendBufSize = NX_GetSendBufferSize( &pkt );
	uint8_t *sendBuffer = (uint8_t *)malloc(4096);

	int32_t sendSize;

	if( count == -1 )
	{
		count = 0x7fffffff;
	}

	for( int32_t i=0 ; i < count ; i++ )
	{
		pkt.frameNumber = frameNumber;
		sendSize = NX_MakeUartPacket(&pkt, sendBuffer, sendBufSize );
		written = hUart->Write(sendBuffer, sendSize);
		frameNumber++;
	}

	free(sendBuffer);

	delete hUart;
#endif
	SlinkClient *hClient = new SlinkClient();
	hClient->StartService();

	while( 1 )
	{
		hClient->AddCommand( CMD_BOOT_DONE );
		hClient->AddCommand( CMD_MARRIAGE_STATE );
		hClient->AddCommand( CMD_ALIVE );		//	Alive Command
		usleep( 1000000 );
	}

	hClient->StopService();
	delete hClient;

}

