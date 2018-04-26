#include <stdio.h>
#include <stdint.h>
#include "NX_IPCServer.h"
#include <unistd.h>

int main (void)
{

	int32_t iRet = NX_IPCServerStart();
	printf("main , NX_IPCServerStart ret : %d\n",iRet);

	while(1)
	{
		usleep(100000);
	}

	NX_IPCServerStop();
	printf("end of main cpp\n");
	return 0;
}
