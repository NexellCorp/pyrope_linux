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
#ifndef __NX_Command_h__
#define __NX_Command_h__
#include <stdint.h>

//	TCON Commands
int32_t	TCON_RegWrite( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_RegRead( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_RegBurstWrite( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

int32_t	TCON_Init( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_Status( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_DoorStatus( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_LvdsStatus( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_BootingStatus( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_LedModeNormal( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_LedModeLod( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_LedOpenNum( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_LedOpenPos( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_TestPattern( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_TargetGamma( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_DeviceGamma( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_PIXELCorrection( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_PIXELCorrectionExtract( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_WhiteSeamRead( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_WhiteSeamWrite( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

int32_t	TCON_OptionalData( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_SwReset( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_EEPRomRead( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	TCON_Version( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

//	PFPGA Commands
int32_t	PFPGA_RegWrite( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PFPGA_RegRead( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

int32_t	PFPGA_Status( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PFPGA_Mute( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PFPGA_UniformityData( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PFPGA_Version( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

//	Platform Commands
int32_t	PLAT_NapVersion( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PLAT_SapVersion( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PLAT_IpcServerVersion( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t	PLAT_IpcClientVersion( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

//	Burst
int32_t CMD_BURSTStart( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);
int32_t CMD_BURSTStop( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

//	IMB Commands
int32_t IMB_ChangeContents( uint32_t cmd, uint8_t *pBuf,uint32_t nSize , uint8_t** pResult ,uint32_t* pResultSize);

#endif //__NX_Command_h__
