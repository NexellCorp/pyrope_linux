#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep, getopt
#include <stdlib.h>	// exit
#include <string.h>

#include <NX_TMSClient.h>

#define NX_DTAG	"[N.AP Con Client]"
#include <NX_DbgMsg.h>

#include "CmdLeaf.h"


void help( char *pAppName )
{
	int32_t i;
	printf("\nUsage : %s [L1 Cmd] [L2 Cmd]  Parameters...\n", pAppName);
	printf(" L1 Command :\n");
	for( i=0 ; true ; i++ )
	{
		if( NULL == gNapCmdInfo[i].exeFunc )
		{
			break;
		}
		printf("\t%s\n", gNapCmdInfo[i].cmd );
	}
	printf(" L2 TCON Commands :\n");
	for( i=0 ; true ; i++ )
	{
		if( NULL == gTconCmdInfo[i].exeFunc )
		{
			break;
		}
		printf("\t%s\n", gTconCmdInfo[i].cmd );
	}
	printf(" L2 PFPGA Commands :\n");
	for( i=0 ; true ; i++ )
	{
		if( NULL == gPFpgaCmdInfo[i].exeFunc )
		{
			break;
		}
		printf("\t%s\n", gPFpgaCmdInfo[i].cmd );
	}
	printf(" L2 BAT Commands :\n");
	for( i=0 ; true ; i++ )
	{
		if( NULL == gBatCmdInfo[i].exeFunc )
		{
			break;
		}
		printf("\t%s\n", gBatCmdInfo[i].cmd );
	}
}


//
//	main routine
//
int32_t main(int32_t argc, char *argv[])
{
	int32_t i, opt;

	while( (opt = getopt(argc, argv, "h")) != -1 )
	{
		if( opt == 'h' )
		{
			help(argv[0]);
			return 0;
		}
	}


	if( argc < 3 )
	{
		printf("[ERROR ARG]\n");
		return -1;
	}

	//	Check Level 1 Command
	i=0;
	while( NULL != gNapCmdInfo[i].exeFunc )
	{
		if( !strncasecmp(argv[1], gNapCmdInfo[i].cmd, strlen(gNapCmdInfo[i].cmd)) )
		{
			return gNapCmdInfo[i].exeFunc( argc-1, &argv[1] );
		}
		i++;
	}

	printf("[ERROR] UNKNOWN\n");

	return 1;
}
