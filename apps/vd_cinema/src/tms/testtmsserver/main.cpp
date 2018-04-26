#include <stdio.h>
#include <stdint.h>
#include "NX_TMSServer.h"
#include <unistd.h>
int main (void)
{

	int32_t iRet = NX_TMSServerStart();
	printf("main , NX_TMSServerStart ret : %d\n",iRet);

	while(1)
	{
		usleep(100000);
	}

	NX_TMSServerStop();
	printf("end of main cpp\n");
	return 0;
}
