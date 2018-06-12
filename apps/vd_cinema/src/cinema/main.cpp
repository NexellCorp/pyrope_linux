//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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

#include <stdio.h>
#include <stdint.h>

#include <NX_Utils.h>
#include <NX_CinemaCommand.h>

#include "CNX_PixelCorrection.h"
#include "CNX_TReg.h"

int32_t main( void )
{
	// CNX_CinemaManager* pCtrl = CNX_CinemaManager::GetInstance();

	// uint8_t inBuf[256] = { 0x67, };
	// int32_t iInSize = 1;

	// uint8_t outBuf[256];
	// int32_t iOutSize;

	// pCtrl->GetSlave();
	// pCtrl->SendCommand(TCON_CMD_STATUS, inBuf, iInSize, outBuf, &iOutSize );

	// CNX_CinemaManager::ReleaseInstance();


	// CNX_PixelCorrection parser;
	// parser.Parse( "./DCI/DOT/ID102/RGB_P2_5_ID102_L0A.txt" );

	// uint16_t *pResult = parser.GetDataReorder();
	// int32_t iResultNum = parser.GetDataReorderNum();


	// for( int32_t i = 0; i < iResultNum; i++ )
	// {
	// 	printf("%d ", pResult[i]);
	// 	if( i % 8 == 7 ) printf("\n");
	// }


	CNX_TReg parser;
	parser.Parse( "./SAMSUNG/TCON_EEPROM/T_REG.txt" );
//	parser.Update( "./SAMSUNG/TCON_EEPROM/T_REG_UPDATE.txt", 4 );
	parser.Make( "./dump.txt" );

	return 0;
}
