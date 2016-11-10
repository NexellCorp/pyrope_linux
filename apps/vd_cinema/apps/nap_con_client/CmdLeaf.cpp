#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>	//	atoi
#include <string.h>
#define NX_DTAG	"[N.AP Con Client]"
#include <NX_DbgMsg.h>

#include <NX_TMSClient.h>

//#include <CmdLeaf.h>
#include "CmdLeaf.h"

//////////////////////////////////////////////////////////////////////////////
//																			//
//							L2 Commands : TCON								//
//																			//
//////////////////////////////////////////////////////////////////////////////
static int32_t TconState(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	if( 0 != NX_TCONCommand( id, TCON_CMD_STATUS, buf, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %d]\n", buf[0]);
	return 0;
}

static int32_t TconOpenState(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	if( 0 != NX_TCONCommand( id, TCON_CMD_OPEN, buf, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %d]\n", buf[0]);
	return 0;
}

static int32_t TconOpenPos(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	NX_TCONCommand( id, TCON_CMD_OPEN_POS, buf, &size );
	return 0;
}

static int32_t TconShortState(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	if( 0 != NX_TCONCommand( id, TCON_CMD_SHORT, buf, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %d]\n", buf[0]);
	return 0;
}

static int32_t TconShortPos(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	NX_TCONCommand( id, TCON_CMD_SHORT_POS, buf, &size );
	return 0;
}

static int32_t TconDoorState(int32_t argc, char *argv[])
{
	uint8_t buf[128];
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	int32_t id = atoi(argv[1]);
	int32_t size = sizeof(buf);
	if( 0 != NX_TCONCommand( id, TCON_CMD_DOOR_STATUS, buf, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %d]\n", buf[0]);
	return 0;
}

static int32_t TconOn(int32_t argc, char *argv[])
{
	uint8_t buf;
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	buf = atoi(argv[1]);
	int32_t size = sizeof(buf);
	if( 0 != NX_TCONCommand( 0, TCON_CMD_ON, &buf, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[OK]\n");
	return 0;
}


static int32_t TconBRMode (int32_t argc, char *argv[])
{
	uint8_t mode[2];
	if( argc < 3 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}

	mode[0] = atoi(argv[1]);
	mode[1] = atoi(argv[2]);
	int32_t size = sizeof(mode);
	if( 0 != NX_TCONCommand( 0, TCON_CMD_BR_CTRL, mode, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[OK]\n");
	return 0;
}

static int32_t TconElapsedTime( int32_t argc, char *argv[] )
{
	uint32_t time;
	int32_t size = sizeof(time);
	(void) argc;
	(void*) argv;
	if( 0 != NX_TCONCommand( 0, TCON_CMD_ELAPSED_TIME, (uint8_t*)&time, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[TIME %dsec]\n", time);
	return 0;
}

static int32_t TconVersion(int32_t argc, char *argv[])
{
	uint8_t version[4];
	int32_t size = sizeof(version);
	(void) argc;
	(void*) argv;
	if( 0 != NX_TCONCommand( 0, TCON_CMD_VERSION, version, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[VERSION %d.%d.%d.%d]\n", version[0], version[1], version[2], version[3]);
	return 0;
}




//////////////////////////////////////////////////////////////////////////////
//																			//
//							L2 Commands : PFPGA								//
//																			//
//////////////////////////////////////////////////////////////////////////////
static int32_t PFpgaState(int32_t argc, char *argv[])
{
	uint8_t state;
	int32_t size = sizeof(state);
	(void) argc;
	(void*) argv;
	if( 0 != NX_PFPGACommand( PFPGA_CMD_STATE, &state, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %d]\n", state);
	return 0;
}

static int32_t PFpgaSource(int32_t argc, char *argv[])
{
	if( argc < 2 )
	{
		printf("[ERROR ARG]\n");
		return 1;
	}
	uint8_t source = atoi(argv[1]);
	int32_t size = sizeof(source);
	if( 0 != NX_PFPGACommand( PFPGA_CMD_SOURCE, &source, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[OK]\n");
	return 0;
}

static int32_t PFpgaVersion(int32_t argc, char *argv[])
{
	uint8_t version[4];
	int32_t size = sizeof(version);
	(void) argc;
	(void*) argv;
	if( 0 != NX_PFPGACommand( PFPGA_CMD_VERSION, version, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[VERSION %d.%d.%d.%d]\n", version[0], version[1], version[2], version[3]);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//							L2 Commands : Battery							//
//																			//
//////////////////////////////////////////////////////////////////////////////
static int32_t BatteryState(int32_t argc, char *argv[])
{
	uint32_t batState[2];
	int32_t size = sizeof(batState);
	(void) argc;
	(void*) argv;
	if( 0 != NX_BATCommand( BAT_CMD_STATE, (uint8_t*)batState, &size ) )
	{
		printf("[ERROR CMD]\n");
		return -1;
	}
	printf("[STATE %dmV, %dmA]\n", batState[0], batState[1]);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//							L2 Commands Parser								//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t TCONCommandProc(int32_t argc, char *argv[])
{
	//	Check Level 2 TCON Command
	int32_t i=0;
	while( NULL != gTconCmdInfo[i].exeFunc )
	{
		if( !strncasecmp(argv[1], gTconCmdInfo[i].cmd, strlen(gTconCmdInfo[i].cmd)) )
		{
			return gTconCmdInfo[i].exeFunc( argc-1, &argv[1] );
		}
		i++;
	}
	printf("[ERROR UNKNOWN]\n");
	return 1;
}

int32_t PFPGACommandProc(int32_t argc, char *argv[])
{
	//	Check Level 2 PFPGA Command
	int32_t i=0;
	while( NULL != gPFpgaCmdInfo[i].exeFunc )
	{
		if( !strncasecmp(argv[1], gPFpgaCmdInfo[i].cmd, strlen(gPFpgaCmdInfo[i].cmd)) )
		{
			return gPFpgaCmdInfo[i].exeFunc( argc-1, &argv[1] );
		}
		i++;
	}
	printf("[ERROR UNKNOWN]\n");
	return 1;
}


int32_t BATCommandProc(int32_t argc, char *argv[])
{
	//	Check Level 2 PFPGA Command
	int32_t i=0;
	while( NULL != gBatCmdInfo[i].exeFunc )
	{
		if( !strncasecmp(argv[1], gBatCmdInfo[i].cmd, strlen(gBatCmdInfo[i].cmd)) )
		{
			return gBatCmdInfo[i].exeFunc( argc-1, &argv[1] );
		}
		i++;
	}
	printf("[ERROR UNKNOWN]\n");
	return 1;
}


//
//	level 1 Commands
//
CMD_INFO gNapCmdInfo[] = {
	{ "TCON" , TCONCommandProc  },
	{ "PFPGA", PFPGACommandProc },
	{ "BAT", BATCommandProc },			//	Battery
	{ "", NULL },
};


//
//	Level 2 Commands : TCON
//
CMD_INFO gTconCmdInfo[] = {
	{ "STATE"     , TconState       },
	{ "OPEN"      , TconOpenState   },
	{ "OPEN_POS"  , TconOpenPos     },
	{ "SHORT"     , TconShortState  },
	{ "SHORT_POS" , TconShortPos    },
	{ "DOOR"      , TconDoorState   },
	{ "ON"        , TconOn          },
	{ "BR_MODE"   , TconBRMode      },
	{ "E_TIME"    , TconElapsedTime },
	{ "VERSION"   , TconVersion     },
	{ "", NULL },
};

//
//	Level 2 Commands : PFPGA
//
CMD_INFO gPFpgaCmdInfo[] = {
	{ "STATE"   , PFpgaState   },
	{ "SOURCE"  , PFpgaSource  },
	{ "VERSION" , PFpgaVersion },
	{ "", NULL },
};


//
//	Level 2 Commands : BATTERY
//
CMD_INFO gBatCmdInfo[] = {
	{ "STATE"   , BatteryState },
	{ "", NULL },
};
