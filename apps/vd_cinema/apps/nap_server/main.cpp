#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep
#include <stdlib.h>	// exit

#include <NX_TMSServer.h>
#include <NX_SecureLinkServer.h>

#define NX_DTAG	"[N.AP Server APP]"
#include <NX_DbgMsg.h>

static int32_t SecurelinkeEventCallback( void *pParam, int32_t eventCode, void *pEvtData, int32_t dataSize )
{
	(void*)pParam;
	(void*)pEvtData;
	NxDbgMsg(NX_DBG_INFO, "Receive Event Message (0x%08x, %d)\n", eventCode, dataSize);
	return 0;
}

static int32_t SecurelinkeAliveCallback( void *pParam )
{
	(void*)pParam;
	NxDbgMsg(NX_DBG_INFO, "Receive Alive Message\n");
	return 0;
}

int32_t main(void)
{
	//	Start TMS Server
	if( 0 != NX_TMSServerStart() )
	{
		NxErrMsg("TMS service demon start failed!!!\n");
		exit(-1);
	}

	//	Start Secure Link Service
	NX_SLinkServerRegEventCB( SecurelinkeEventCallback, NULL );
	NX_SLinkServerRegAliveCB( SecurelinkeAliveCallback, NULL );

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
