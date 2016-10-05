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
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <CNX_BaseClass.h>
#include <CNX_OpenSSL.h>
#include <gdc_protocol.h>
#include <SockUtils.h>

#define NX_DTAG	"[GDC Client]"
#include <NX_DbgMsg.h>

#define MAX_TIMEOUT			3000
#define MAX_PAYLOAD_SIZE	65535

class CNX_GDCClient
{
public:
	CNX_GDCClient(){}
	~CNX_GDCClient(){}

	static CNX_GDCClient* GetInstance();
	static void ReleaseInstance();

public:
	int32_t Verify( const char *pIpAddr, int32_t iPort, const char *pCertFile, const char *pPrivFile, uint8_t *pPlaneText );

private:
	int32_t ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize );
	int32_t WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize );

private:
	static CNX_GDCClient *m_psInstance;
};

//------------------------------------------------------------------------------
CNX_GDCClient* CNX_GDCClient::m_psInstance = NULL;


//------------------------------------------------------------------------------
//
//	For Singleton
//

//------------------------------------------------------------------------------
CNX_GDCClient* CNX_GDCClient::GetInstance( )
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_GDCClient();
	}
	return (CNX_GDCClient*)m_psInstance;
}

//------------------------------------------------------------------------------
void CNX_GDCClient::ReleaseInstance( )
{
	if( NULL != m_psInstance )
	{
		delete m_psInstance;
	}
	m_psInstance = NULL;
}

//------------------------------------------------------------------------------
int32_t CNX_GDCClient::ReadData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	int32_t readSize, totalSize=0;
	do {
		int32_t ret;
		struct pollfd hPoll;
		
		hPoll.fd		= fd;
		hPoll.events	= POLLIN | POLLERR;
		hPoll.revents	= 0;
		ret = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
		if( 0 < ret ) 
		{
			readSize = read( fd, pBuf, iSize );

			if( 0 >= readSize )
			{
				return -1;
			}

			iSize     -= readSize;
			pBuf      += readSize;
			totalSize += readSize;
		}
		else if( 0 > ret )
		{
			NxErrMsg( "Fail, poll().\n" );
			return -1;
		}
		else
		{
			// printf( "Timeout. ReadData().\n" );
			return 0;
		}
	} while(iSize > 0);

	return totalSize;
}

//------------------------------------------------------------------------------
int32_t CNX_GDCClient::WriteData( int32_t fd, uint8_t *pBuf, int32_t iSize )
{
	return write( fd, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_GDCClient::Verify( const char *pIpAddr, int32_t iPort, const char *pCertFile, const char *pPrivFile, uint8_t *pPlaneText )
{
	int32_t connectSock = -1;
	uint8_t buf[MAX_PAYLOAD_SIZE + 6];
	int32_t iSize;

	uint32_t iKey = 0;
	uint8_t *pPayload = NULL;
	uint16_t iPayloadSize = 0;
	uint32_t iWriteSize = 0;

	connectSock = TCP_Connect( pIpAddr, iPort );
	if( 0 > connectSock )
	{
		printf("Fail, TCP_Connect().\n");
		return -1;
	}

	uint8_t *pCertBuf = NULL;
	CNX_OpenSSL clientSSL;
	clientSSL.ReadPrivateKey( pPrivFile );
	clientSSL.ReadCertificate( pCertFile );

	int32_t iCertSize = clientSSL.GetCertificate( &pCertBuf );
	iWriteSize = GDC_MakePacket( GDC_KEY(1), (void*)pCertBuf, iCertSize, buf, sizeof(buf) );
	WriteData( connectSock, buf, iWriteSize );

	while(1)
	{
		iKey = 0;
		pPayload = NULL;
		iPayloadSize = 0;
		iWriteSize = 0;

		memset( buf, 0x00, sizeof(buf) );

		do {
			uint8_t tempData;
			iSize = ReadData( connectSock, &tempData, 1 );
			if( 1 > iSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				break;
			}

			iKey = (iKey << 8) | tempData;
			if( iKey == SEC_KEY(tempData) )
			{
				break;
			}
		} while( 1 );

		if( 1 > iSize )
		{
			goto ErrorExit;
		}

		iSize = ReadData( connectSock, buf, 2 );
		if( 2 > iSize )
		{
			NxErrMsg( "Fail, ReadData().\n" );
			goto ErrorExit;
		}

		iPayloadSize = MAKE_LENGTH_2BYTE( buf[0], buf[1] );
		if( iPayloadSize != 0 )
		{
			iSize = ReadData( connectSock, buf + 2, iPayloadSize );
			if( 0 >= iSize || iSize != iPayloadSize )
			{
				NxErrMsg( "Fail, ReadData().\n" );
				goto ErrorExit;
			}

			pPayload = buf + 2;
		}

		printf( "-->> Receive Data( 0x%08x, %d )\n", iKey, iPayloadSize );

		//
		//
		//
		if( iKey == SEC_KEY(1) )
		{
			printf("======================================================================\n");
			printf("> Receive Ceriticate.\n");
			printf("payload:\n%s\n", pPayload);
			printf("======================================================================\n");

			//
			// Verify Certification Chain.
			//

			// Read Public key from Server Certificate
			clientSSL.ReadCertificate( pPayload, iPayloadSize );

			// Send Plane Text String
			iWriteSize = GDC_MakePacket( GDC_KEY(2), (void*)pPlaneText, strlen((char*)pPlaneText), buf, sizeof(buf) );
			WriteData( connectSock, buf, iWriteSize );
		}
		else if( iKey == SEC_KEY(2) )
		{
			printf("======================================================================\n");
			printf("> Receive Signed Data.\n");
			clientSSL.DumpHex("payload(hex):", pPayload, iPayloadSize);
			printf("======================================================================\n");

			if( 0 > clientSSL.Verify( pPlaneText, strlen((char*)pPlaneText), pPayload, iPayloadSize ) )
			{
				printf("Fail, Verify().\n");
				goto ErrorExit;
			}

			iWriteSize = GDC_MakePacket( GDC_KEY(3), NULL, 0, buf, sizeof(buf) );
			WriteData( connectSock, buf, iWriteSize );
		}
		else if( iKey == SEC_KEY(3) )
		{
			printf("======================================================================\n");
			printf("> Receive Marriage OK.\n");
			printf("payload:\n%s\n", pPayload);
			printf("======================================================================\n");
			break;
		}
		else
		{
			printf("Unknown Key.\n");
			break;
		}
	}

	if( 0 <= connectSock )
	{
		close( connectSock );
	}
	return 0;

ErrorExit:
	NxErrMsg( "Fail, Marriage.\n" );

	if( 0 <= connectSock )
	{
		close( connectSock );
	}

	return -1;
}

//------------------------------------------------------------------------------
//
//	External APIs
//

//------------------------------------------------------------------------------
int32_t NX_MarriageVerify( const char *pIpAddr, int32_t iPort, const char *pCertFile, const char *pPrivFile, uint8_t *pPlaneText )
{
	CNX_GDCClient *hGdc = CNX_GDCClient::GetInstance();
	return hGdc->Verify(pIpAddr, iPort, pCertFile, pPrivFile, pPlaneText );
}
