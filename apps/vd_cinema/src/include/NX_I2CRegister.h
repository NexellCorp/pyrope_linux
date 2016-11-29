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

#ifndef __NX_I2CREGISTER_H__
#define __NX_I2CREGISTER_H__

#define PFPGA_I2C_PORT					2

//
//	TCON Status
//
#define TCON_REG_CHECK_STATUS			0x0170


//
//	Cabinet Door
//
#define TCON_REG_CHECK_DOOR_READ		0x0171


//
//	LED Open Check
//
#define TCON_REG_LOD_REMOVAL_EN			0x00E6
#define TCON_REG_LIVE_LOD_EN			0x0100
#define TCON_REG_ERROR_OUT_SEL			0x0106
#define TCON_REG_ERROR_OUT_CLK			0x0107
#define TCON_REG_ERROR_OUT_RDY			0x0108
#define TCON_REG_LOD_100_CHK_DONE		0x0109
#define TCON_REG_100_DONE_TIME			0x010A
#define TCON_REG_CURRENT_PERCENT		0x010B
#define TCON_REG_ERROR_NUM_M1			0x010C
#define TCON_REG_ERROR_NUM_M2			0x010D
#define TCON_REG_ERROR_NUM_M3			0x010E
#define TCON_REG_ERROR_NUM_M4			0x010F
#define TCON_REG_ERROR_NUM_M5			0x0110
#define TCON_REG_ERROR_NUM_M6			0x0111
#define TCON_REG_ERROR_NUM_M7			0x0112
#define TCON_REG_ERROR_NUM_M8			0x0113
#define TCON_REG_ERROR_NUM_M9			0x0114
#define TCON_REG_ERROR_NUM_M10			0x0115
#define TCON_REG_ERROR_NUM_M11			0x0116
#define TCON_REG_ERROR_NUM_M12			0x0117
#define TCON_REG_ERROR_NUM_OVR			0x0118
#define TCON_REG_X_COORDINATE			0x0119
#define TCON_REG_Y_COORDINATE			0x011A


//
//	LED Short Check
//

/* Not Support */


//
//	LED Pattern
//
#define TCON_REG_CABINET_ID				0x0023
#define TCON_REG_PATTERN				0x0024
#define TCON_REG_LEFT					0x0025
#define TCON_REG_RIGHT					0x0026
#define TCON_REG_TOP					0x0027
#define TCON_REG_DOWN					0x0028
#define TCON_REG_BOX_R					0x0029
#define TCON_REG_BOX_G					0x002A
#define TCON_REG_BOX_B					0x002B


//
//	LED Mastering
//

/* Not Yet */


//
//	LED Elapsed/Accumulate Time
//

/* Not Yet */


//
//	Version
//
#define TCON_REG_FPGA_MODE_NAME			0x0013
#define TCON_REG_FLASH_SEL				0x008D
#define TCON_REG_F_LED_WR_EN			0x0140

#define TCON_REG_F_LED_DATA00			0x0142
#define TCON_REG_F_LED_DATA01			0x0143
#define TCON_REG_F_LED_DATA02			0x0144
#define TCON_REG_F_LED_DATA03			0x0145
#define TCON_REG_F_LED_DATA04			0x0146
#define TCON_REG_F_LED_DATA05			0x0147
#define TCON_REG_F_LED_DATA06			0x0148
#define TCON_REG_F_LED_DATA07			0x0149
#define TCON_REG_F_LED_DATA08			0x014A
#define TCON_REG_F_LED_DATA09			0x014B
#define TCON_REG_F_LED_DATA10			0x014C
#define TCON_REG_F_LED_DATA11			0x014D
#define TCON_REG_F_LED_DATA12			0x014E
#define TCON_REG_F_LED_DATA13			0x014F
#define TCON_REG_F_LED_DATA14			0x0150
#define TCON_REG_F_LED_DATA15			0x0151
#define TCON_REG_F_LED_DATA00_READ		0x0152
#define TCON_REG_F_LED_DATA01_READ		0x0153
#define TCON_REG_F_LED_DATA02_READ		0x0154
#define TCON_REG_F_LED_DATA03_READ		0x0155
#define TCON_REG_F_LED_DATA04_READ		0x0156
#define TCON_REG_F_LED_DATA05_READ		0x0157
#define TCON_REG_F_LED_DATA06_READ		0x0158
#define TCON_REG_F_LED_DATA07_READ		0x0159
#define TCON_REG_F_LED_DATA08_READ		0x015A
#define TCON_REG_F_LED_DATA09_READ		0x015B
#define TCON_REG_F_LED_DATA10_READ		0x015C
#define TCON_REG_F_LED_DATA11_READ		0x015D
#define TCON_REG_F_LED_DATA12_READ		0x015E
#define TCON_REG_F_LED_DATA13_READ		0x015F
#define TCON_REG_F_LED_DATA14_READ		0x0160
#define TCON_REG_F_LED_DATA15_READ		0x0161


#endif	// __NX_I2CREGISTER_H__
