//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __NX_IPCCOMMAND_H__
#define __NX_IPCCOMMAND_H__

//
//	TCON Command
//
#define TCON_CMD_STATUS				0x0001	//	TCON Status
#define TCON_CMD_DOOR_STATUS		0x0002	//	Door Tamper Status

#define TCON_CMD_MODE_NORMAL		0x0011	//	TCON Normal Mode
#define TCON_CMD_MODE_LOD			0x0012	//	TCON LOD Mode for LED Open Check
#define TCON_CMD_OPEN_NUM			0x0013	//	LED Open Num
#define TCON_CMD_OPEN_POS			0x0014	//	LED Open Pos (x1, y1, x2, y2, ... )
#define TCON_CMD_SHORT_NUM			0x0015	//	LED Short Num
#define TCON_CMD_SHORT_POS			0x0016	//	LED Short Pos (x1, y1, x2, y2, ... )

#define TCON_CMD_PATTERN			0x0021	//	TEST Pattern

#define TCON_CMD_MASTERING			0x0031

#define TCON_CMD_ELAPSED_TIME		0x0041	//	LED Elapsed Time
#define TCON_CMD_ACCUMULATE_TIME	0x0042	//	Total LED Accumulate Time

#define TCON_CMD_VERSION			0x0070	//	TCON Version


//
//	PFPGA Command
//
#define PFPGA_CMD_STATUS			0x0101	//	PFPGA Status
#define PFPGA_CMD_SOURCE			0x0102	

#define PFPGA_CMD_VERSION			0x0170	//	PFPGA Version


//
//	Battery Command
//
#define BAT_CMD_STATUS				0x0201	//	Battery Status


//
//	IMB Command
//
#define IMB_CMD_STATUS				0x0301	//	IMB Status
#define IMB_CMD_VERSION				0x0302	//	IMB Version

#endif	// __NX_IPCCOMMAND_H__
