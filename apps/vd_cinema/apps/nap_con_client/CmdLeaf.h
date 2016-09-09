#ifndef __CmdLeaf_h__
#define __CmdLeaf_h__

struct _CMD_INFO{
	char cmd[64];
	int32_t (*exeFunc)(int32_t argc, char *argv[]);
};
typedef struct _CMD_INFO CMD_INFO;

extern CMD_INFO gNapCmdInfo[];
extern CMD_INFO gTconCmdInfo[];
extern CMD_INFO gPFpgaCmdInfo[];
extern CMD_INFO gBatCmdInfo[];

#endif	// __CmdLeaf_h__
