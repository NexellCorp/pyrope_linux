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

#ifndef __NX_TOKEN_H__
#define __NX_TOKEN_H__

#include <stdint.h>

int32_t NX_GetToken( char *pData, int32_t iIndex, char *pResult );
int32_t NX_GetTokenParse( char *pData, const char *pFormat, ... );
int32_t NX_GetTokenNum( char *pData );

#endif	// __NX_TOKEN_H__
