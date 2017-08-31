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
//	Re-mapping Command for Cinema LED Protocol. ( 4 byte command -> 1byte command )
//
#define CMD_TYPE_TCON					0x00	// 0000 0000
#define CMD_TYPE_PFPGA					0x40	// 0100 0000
#define CMD_TYPE_PLATFORM				0x80	// 1000 0000
#define CMD_TYPE_RESERVED				0xC0	// 1100 0000

#define GDC_COMMAND( type, cmd )		(type | (cmd & 0x00FF))


//
//	TCON Command
//
#define TCON_CMD_REG_WRITE				0x0000	//	TCON Register Write					// 0x00 | 0x00
#define TCON_CMD_REG_READ				0x0001	//	TCON_Register Read					// 0x00 | 0x01
#define TCON_CMD_REG_BURST_WRITE		0x0002	//	TCON Register Burst Write ( Not implemenation )

#define TCON_CMD_INIT					0x0010	//	TCON Init							// 0x00 | 0x10	:: Internal Command
#define TCON_CMD_STATUS					0x0011	//	TCON Status							// 0x00 | 0x11
#define TCON_CMD_DOOR_STATUS			0x0012	//	Door Tamper Status					// 0x00 | 0x12
#define TCON_CMD_LVDS_STATUS			0x0013	//	TCON LVDS Status					// 0x00 | 0x13
#define TCON_CMD_BOOTING_STATUS			0x0014	//	TCON Booting Status --> 0x7E1		// 0x00 | 0x14	:: Internal Command
#define TCON_CMD_MODE_NORMAL			0x0015	//	TCON Normal Mode					// 0x00 | 0x15
#define TCON_CMD_MODE_LOD				0x0016	//	TCON LOD Mode for LED Open Check	// 0x00 | 0x16
#define TCON_CMD_OPEN_NUM				0x0017	//	LED Open Num						// 0x00 | 0x17
#define TCON_CMD_OPEN_POS				0x0018	//	LED Open Pos						// 0x00 | 0x18
#define TCON_CMD_PATTERN_RUN			0x0019	//	Test Pattern Run					// 0x00 | 0x19
#define TCON_CMD_PATTERN_STOP			0x001A	//	Test Pattern Stop					// 0x00 | 0x1A
#define TCON_CMD_TGAM_R					0x001B	//	TCON Target Gamma Red				// 0x00 | 0x1B
#define TCON_CMD_TGAM_G					0x001C	//	TCON Target Gamma Green				// 0x00 | 0x1C
#define TCON_CMD_TGAM_B					0x001D	//	TCON Target Gamma Blue				// 0x00 | 0x1D
#define TCON_CMD_DGAM_R					0x001E	//	TCON Device Gamma Red				// 0x00 | 0x1E
#define TCON_CMD_DGAM_G					0x001F	//	TCON Device Gamma Green				// 0x00 | 0x1F
#define TCON_CMD_DGAM_B					0x0020	//	TCON Device Gamma Blue				// 0x00 | 0x20
#define TCON_CMD_DOT_CORRECTION			0x0021	//	TCON Dot Correction					// 0x00 | 0x21
#define TCON_CMD_DOT_CORRECTION_EXTRACT	0x0022	//	TCON Dot Correction Extract			// 0x00 | 0x22
#define TCON_CMD_WHITE_SEAM_READ		0x0023	//	TCON White Seam	Read				// 0x00 | 0x23
#define TCON_CMD_WHITE_SEAM_WRITE		0x0024	//	TCON White Seam Write				// 0x00 | 0x24
#define TCON_CMD_ELAPSED_TIME			0x0025	//	LED Elapsed Time					// 0x00 | 0x25
#define TCON_CMD_ACCUMULATE_TIME		0x0026	//	Total LED Accumulate Time			// 0x00 | 0x26
#define TCON_CMD_VERSION				0x0027	//	TCON Version						// 0x00 | 0x27


//
//	PFPGA Command
//
#define PFPGA_CMD_REG_WRITE				0x0100	//	PFPGA Register Write				// 0x40 | 0x00
#define PFPGA_CMD_REG_READ				0x0101	//	PFPGA Register Read					// 0x40 | 0x01
#define PFPGA_CMD_REG_BURST_WRITE		0x0102	//	PFPGA Register Burst Write ( Not implemenation )

#define PFPGA_CMD_STATUS				0x0110	//	PFPGA Status						// 0x40 | 0x10
#define PFPGA_CMD_UNIFORMITY_DATA		0x0111	//	PFPGA Uniformity Data Write			// 0x40 | 0x11
#define PFPGA_CMD_MUTE					0x0112	//	PFPGA Mute							// 0x40 | 0x12
#define PFPGA_CMD_VERSION				0x0113	//	PFPGA Version						// 0x40 | 0x13


//
//	Platform Command
//
#define PLATFORM_CMD_NAP_VERSION		0x0210	//	N.AP Software Version				// 0x80 | 0x10
#define PLATFORM_CMD_SAP_VERSION		0x0211	//	S.AP Software Version				// 0x80 | 0x11
#define PLATFORM_CMD_IPC_SERVER_VERSION	0x0212	//	IPC Server Version					// 0x80 | 0x12
#define PLATFORM_CMD_IPC_CLIENT_VERSION	0x0213	//	IPC Client Version					// 0x80 | 0x13


//
//	IMB Command
//
#define IMB_CMD_CHANGE_CONTENTS			0x0C10	//	IMB Change Contents					// 0xC0 | 0x10

#endif	// __NX_IPCCOMMAND_H__
