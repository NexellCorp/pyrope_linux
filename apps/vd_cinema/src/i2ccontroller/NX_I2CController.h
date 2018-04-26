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

#ifndef __CNX_I2CController_h__
#define __CNX_I2cController_h__

#include <stdint.h>

int32_t NX_SendData(void* pOwner, uint32_t cmd, void *pPayload, uint32_t payloadSize, uint8_t** pResult , uint32_t* iResultSize);

int32_t NX_ReleaseInstance();

#endif //__CNX_I2cController_h__
