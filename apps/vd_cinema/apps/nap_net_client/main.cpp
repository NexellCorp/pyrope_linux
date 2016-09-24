#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep
#include <stdlib.h>	// exit

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include <NX_TMSClient.h>
#include <SockUtils.h>

#define NX_DTAG	"[N.AP Net Client]"
#include <NX_DbgMsg.h>

#define PORT_NUM	(5555)

#define MAX_BUFF_SIZE	(4096)

static  int32_t server_main( short port )
{
	int32_t svrSock = -1;
	int32_t clntLen, sendSize, recvSize;
	struct sockaddr_in clientAddr;
	static uint8_t recvBuf[MAX_BUFF_SIZE];
	static uint8_t sendBuf[MAX_BUFF_SIZE];

	svrSock = UDP_Open( port );

	clntLen = sizeof(clientAddr);
	while( 1 )
	{
		//	Receive From Data
		memset( recvBuf, 0, sizeof(recvBuf) );
		recvSize = recvfrom( svrSock, recvBuf, MAX_BUFF_SIZE, 0, (struct sockaddr*)&clientAddr, &clntLen );
		printf( "recvSize = %d\n", recvSize );

		//	Send Data
		memset( recvBuf, 0, sizeof(recvBuf) );
		sprintf( (char*)sendBuf, "%s = %d\n", recvBuf, recvSize);
		sendSize = strlen((char*)sendBuf);
		sendto( svrSock, sendBuf, sendSize, 0, (struct sockaddr*)&clientAddr, clntLen );
	}

	return 0;
}

static int32_t client_main( const char *ipAddr, short port )
{
	int32_t clntSock = -1;
	int32_t svrLen;
	int32_t sendSize, recvSize;
	struct sockaddr_in serverAddr;
	static uint8_t sendBuf[MAX_BUFF_SIZE];
	static uint8_t recvBuf[MAX_BUFF_SIZE];

	if(ipAddr==NULL)
		return -1;

	clntSock  = socket( AF_INET, SOCK_DGRAM, 0);

	if( clntSock <= 0 )
	{
		return -1;
	}

	memset( &serverAddr, 0, sizeof(serverAddr) );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ipAddr);

	svrLen = sizeof( serverAddr );

	sprintf( (char*)sendBuf, "send client data" );
	sendSize = strlen((char*)sendBuf);

	sendSize = sendto( clntSock, sendBuf, sendSize, 0, (struct sockaddr*)&serverAddr, svrLen );

	if( sendSize < 0 )
	{
		printf("sendto error %s(%d)\n", strerror(errno), errno);
		goto ErrorExit;
	}

	recvSize = recvfrom( clntSock, recvBuf, MAX_BUFF_SIZE, 0, (struct sockaddr*)&serverAddr, &svrLen );

ErrorExit:
	close( clntSock );
	return 0;
}


void help( char *pAppName )
{
	int32_t i;
	printf("\nUsage : %s [options]\n", pAppName);
	printf("    -a : ip address (client only)\n");
	printf("    -p : port(def:%d)\n", PORT_NUM);
	printf("    -c : client\n");
	printf("    -s : server\n");
}


int32_t main(int32_t argc, char *argv[])
{
	int32_t i, opt;
	short port = PORT_NUM;
	char *ipAddr = NULL;
	bool isServer = true;

	while( (opt = getopt(argc, argv, "ha:p:sc")) != -1 )
	{
		if( opt == 'h' )
		{
			help(argv[0]);
			return 0;
		}
		else if( opt == 'a' )
		{
			ipAddr = optarg;
		}
		else if( opt == 'p' )
		{
			port = atoi(optarg);
		}
		else if( opt == 's' )
		{
			isServer = true;
		}
		else if( opt == 'c' )
		{
			isServer = false;
		}
	}

	if( isServer )
	{
		printf("Server Mode\n");
		return server_main(port);
	}
	else
	{
		printf("Client Mode\n");
		return client_main(ipAddr, port);
	}

	return 0;
}
