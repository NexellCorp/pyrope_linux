#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "NX_TMSClient.h"
#include "NX_Utils.h"
#include <stdlib.h>     /* srand, rand */
#include <string.h> //memset
//------------------------------------------------------------------------------

int main (void)
{
	//Command 2bytes
	uint32_t cmd = 0x0001;

	uint8_t* pPayload;
	uint32_t payloadSize;

	//test for malloc fail start ===============================================
	// payloadSize = UINT64_MAX;
	// printf("main , UINT64_MAX : %u\n", payloadSize);
	printf("main , UINT16_MAX : %u\n", UINT16_MAX);

	printf("main , UINT32_MAX : %u\n", UINT32_MAX);
	printf("main , 1MB : 1048576\n");

	// pPayload = (uint8_t*)malloc( sizeof(uint8_t) * payloadSize );
	// if( NULL == pPayload)
	// {
	// 	printf("main , NULL,, malloc is failed\n");
	// }
	// else
	// {
	// 	free(pPayload);
	// }
	//test for malloc fail end =================================================

	uint64_t sucessCount = 0;
	uint64_t failCount = 0;
	uint64_t minFailSize = UINT64_MAX;
	// uint64_t maxFailSize = 0;
	uint64_t count = 0;
	while( UINT64_MAX > count )
	{
		usleep( 10000 );
		pPayload = NULL;
		payloadSize = 0;

		srand( (uint32_t)NX_GetTickCount() );


		// payloadSize = rand() % UINT32_MAX +1;
		// payloadSize = 8;
		payloadSize = 1048576;

		printf("\n\n\nmain , payloadSize : %u\n" , payloadSize);

		pPayload = (uint8_t*)malloc( sizeof(uint8_t) * payloadSize );
		if( NULL == pPayload )
		{
			printf(">>>>>>>>>>>>main , malloc fail.. with size : %u\n" , payloadSize);
			continue;
		}

		memset( pPayload , 1 , payloadSize );
		// for(uint32_t i = 0 ; i < payloadSize ; i++)
		// {
		// 	//pPayload[i] = 1;//rand() % 256 +1;;
		// 	printf("pPayload[%u] : %d\n", i , pPayload[i]);
		// }


		int32_t iRet = NX_TMSSendCommand( "127.0.0.1" , cmd , pPayload , payloadSize );
		printf("main , count : %lu NX_TMSSendCommand ret : %d\n" , count ,iRet);
		if(0 > iRet)
		{
			printf("main , NX_TMSSendCommand fail..\n");
			// break;
			failCount++;

			if(payloadSize < minFailSize)
			{
				minFailSize = payloadSize;
			}
			// if(payloadSize > maxFailSize)
			// {
			// 	maxFailSize = payloadSize;
			// }
		}
		else
		{
			sucessCount++;
		}


		if( NULL != pPayload ) free(pPayload);
		count++;

		printf("tried count : %lu\n",count);
		printf("sucessCount  : %lu\n",sucessCount);
		printf("failCount : %lu\n",failCount);
		printf("minFailSize : %lu\n",minFailSize);
	}


	//==

	// uint8_t pPayload[6];

	//length 4bytes
	// uint32_t payloadSize = sizeof(pPayload);

	//value[]
	// pPayload[0] = 0x06;
	// pPayload[1] = 0x05;
	// pPayload[2] = 0x04;
	// pPayload[3] = 0x03;
	// pPayload[4] = 0x02;
	// pPayload[5] = 0x01;

	// uint64_t count = 0;
	// printf("UINT64_MAX : %lu\n ", UINT64_MAX);
	// while( UINT64_MAX > count )
	// {
	// 	usleep( 1000000 );
	// 	int32_t iRet = NX_TMSSendCommand( "127.0.0.1" , cmd , pPayload , payloadSize );
	// 	printf("main , count : %lu NX_TMSSendCommand ret : %d\n" , count ,iRet);
	// 	if(0 > iRet)
	// 	{
	// 		printf("main , NX_TMSSendCommand fail..\n");
	// 		break;
	// 	}
	// 	count++;
	// }

	printf("tried count : %lu\n",count);
	printf("sucessCount  : %lu\n",sucessCount);
	printf("failCount : %lu\n",failCount);
	printf("minFailSize : %lu\n",minFailSize);



	printf("end of main cpp\n");
	return 0;
}
