#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "NX_IPCClient.h"

int main (void)
{
	//Command 2bytes
	uint32_t cmd = 0x0001;
	uint8_t pPayload[6];

	//length 4bytes
	uint32_t payloadSize = sizeof(pPayload);

	//value[]
	pPayload[0] = 0x06;
	pPayload[1] = 0x05;
	pPayload[2] = 0x04;
	pPayload[3] = 0x03;
	pPayload[4] = 0x02;
	pPayload[5] = 0x01;

	uint64_t count = 0;
	printf("UINT64_MAX : %lu\n ", UINT64_MAX);
	while( UINT64_MAX > count )
	{
		usleep( 1000000 );
		int32_t iRet = NX_IPCSendCommand( cmd , pPayload , payloadSize );
		printf("main , count : %lu NX_IPCSendCommand ret : %d\n" , count ,iRet);
		if(0 > iRet)
		{
			printf("main , NX_IPCSendCommand fail..\n");
			break;
		}
		count++;
	}

	printf("tried count : %lu\n",count);



	printf("end of main cpp\n");
	return 0;
}
