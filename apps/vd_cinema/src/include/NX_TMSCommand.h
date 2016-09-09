#ifndef __NX_TMSCommand_h__
#define __NX_TMSCommand_h__

enum {
	BR_MODE_DCI,
	BR_MODE_HDR,
	BR_MODE_EVT_CINEMA,
};

//	for event cinema mode's mode.
enum {
	EVT_CINEMA_PEAKING,
	EVT_CINEMA_GLOBAL,
};


//
//	TCON Command
//
#define TCON_CMD_STATE			0x01	//	Just Status
#define	TCON_CMD_OPEN			0x02	//	LED Open
#define TCON_CMD_OPEN_POS		0x03	//	Get LED Open Position
#define TCON_CMD_SHORT			0x04	//	LED Short
#define TCON_CMD_SHORT_POS		0x05	//	Get LED Short Position
#define TCON_CMD_DOOR			0x06
#define TCON_CMD_ON				0x07	//	LED On/Off

#define TCON_CMD_BR_CTRL		0x20	//	Brightness Control & Event Cinema mode
#define TCON_CMD_ELAPSED_TIME	0x21	//	elapsed time & total time ( in sec )

#define TCON_CMD_VERSION		0x70	//	Get TCON Version

//
//	PFPGA Command
//
#define PFPGA_CMD_STATE			0x101
#define PFPGA_CMD_SOURCE		0x102
#define PFPGA_CMD_VERSION		0x170


//
//	Battery Command
//
#define BAT_CMD_STATE			0x201		//	Battery state command

#endif	// __NX_TMSCommand_h__
