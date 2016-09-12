#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep
#include <stdlib.h>	// exit

#include <NX_TMSClient.h>

#define NX_DTAG	"[N.AP Net Client]"
#include <NX_DbgMsg.h>

int32_t main(int32_t argc, char *argv[])
{
	//	Start TMS Server
	if( 0 != NX_TMSServerStart() )
	{
		NxErrMsg("TMS service demon start failed!!!\n");
		exit(-1);
	}

	//	Start Secure Link Service
	NX_SLinkServerRegEventCB( SecurelinkeEventCallback, NULL );

	if( 0 != NX_SLinkServerStart() )
	{
		NxErrMsg("Secure linke demon start failed!!!\n");
		exit(-1);
	}

	while( 1 )
	{
		usleep(100000);
	}

	return 0;
}
