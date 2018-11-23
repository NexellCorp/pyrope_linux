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

#define NX_USE_NAP_I2C			1
#define NX_USE_BROADCAST_I2C0	0

#ifndef __NX_I2CCONTROL_H__
#define __NX_I2CCONTROL_H__

#if (1600 <= _MSC_VER)
#include <stdint.h>
#else
#include "stdint.h"
#endif

#define NX_I2C_DEV			_T("NAP")
#define NX_I2C_SCRIPT_PATH	_T("/system/bin")
#define NX_I2C_SCRIPT_NAME	_T("nx_i2c_script.sh")

BOOL	NX_I2CConnect();

int32_t	NX_I2CWrite( int32_t iPort, int32_t iSlave, uint16_t iAddr, uint16_t iBuf );
int32_t	NX_I2CWrite( int32_t iPort, int32_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize );

int32_t	NX_I2CRead( int32_t iPort, int32_t iSlave, uint16_t iAddr );

#endif	// __NX_I2CCONTROL_H__