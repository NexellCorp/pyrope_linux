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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <SockUtils.h>
#include <tms_protocol.h>
#include <NX_Utils.h>

#define	TMS_SERVER_FILE "/data/local/tmp/ipc_server"
#define	MAX_PAYLOAD_SIZE	4096

//------------------------------------------------------------------------------
static int32_t ReadData(int32_t fd, uint8_t *pBuf, int32_t size)
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
	}while(size > 0);
	return totalSize;
}

//------------------------------------------------------------------------------
int main( void )
{
	int32_t svrSock;
	int32_t readLen;
	//	client socket
	int32_t clntSock;
	struct sockaddr_un clntAddr;
	int32_t clntAddrSize;

	int32_t readSize;

	uint8_t *pBuf = (uint8_t *)malloc(MAX_PAYLOAD_SIZE);
	uint8_t *payload;
	int32_t payloadSize;
	int32_t len;
	uint32_t key=0, cmd=0;
	uint8_t tmp;

	svrSock = LS_Open( TMS_SERVER_FILE );
	if( svrSock < 0 ){
		printf("LS_Open Failed !!!\n");
		return -1;
	}

	while(1)
	{
		if( -1 == listen(svrSock, 5) )
		{
			printf( "Error : listen (err = %d)\n", errno);
			exit( 1);
		}

		clntAddrSize = sizeof( clntAddr );
		clntSock = accept( svrSock, (struct sockaddr*)&clntAddr, &clntAddrSize );

		if ( -1 == clntSock)
		{
			printf( "Error : accept (err = %d)\n", errno);
			exit( 1);
		}

		//
		//	Find Key Code
		//
		do{
			readSize = ReadData( clntSock, &tmp, 1 );
			if( readSize != 1 )
			{
				printf("Read Error!!!\n");
				goto ERROR;
			}
			key = (key<<8) | tmp;
			if( key == TMS_KEY_VALUE )
			{
				break;
			}
		}while(1);

		//	Read Length
		readSize = ReadData(clntSock, pBuf, 2);
		if( readSize <= 0 ){
			printf("Error : Read Length\n");
			goto ERROR;
		}
		len = TMS_GET_LENGTH(pBuf[0], pBuf[1]);

		//	Read Data
		readLen = read ( clntSock, pBuf+2, len );
		if( readLen <=0 )
		{
			printf("Error : Read Data\n");
			goto ERROR;
		}

		// 	Command
		cmd = TMS_GET_COMMAND(pBuf[2], pBuf[3]);

		//	Payload
		payload = pBuf + 4;
		payloadSize = len - 2;

		printf( "Receive Data: Length(%d), Command(0x%08x), PayloadSize(%d)\n",
				len, cmd, payloadSize );

ERROR:
		close( clntSock );
	}
	return 0;
}
