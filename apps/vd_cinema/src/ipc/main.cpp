#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ipc_protocol.h"

//
//test ipc_protocol
//
int main (void)
{
	//key 4bytes
	uint32_t key = NXP_KEY_VALUE;

	//command 2bytes
	uint32_t cmd = 0x0001;

	uint8_t pPayload[6];

	//length 4bytes
	uint32_t payloadSize = sizeof(pPayload);

	pPayload[0] = 0x06;
	pPayload[1] = 0x05;
	pPayload[2] = 0x04;
	pPayload[3] = 0x03;
	pPayload[4] = 0x02;
	pPayload[5] = 0x01;



	uint32_t outBufSize = 4 + 2 + 4 + payloadSize;
	uint8_t* pOutBuf;
	pOutBuf = (uint8_t*)malloc( sizeof(uint8_t) * outBufSize );



	int32_t iRet = IPC_MakePacket( key, cmd, pPayload, payloadSize, pOutBuf, outBufSize );

	printf("main , IPC_MakePacket ret : %d\n",iRet);

	printf("main , DumpIpcPacket(pOutBuf, (4+2+4+payloadSize), 1);\n\n");
	DumpIpcPacket(pOutBuf, (4+2+4+payloadSize), 1);



	printf("end of main cpp\n");
	return 0;
}
