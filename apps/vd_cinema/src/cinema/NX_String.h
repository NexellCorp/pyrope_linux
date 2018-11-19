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

#ifndef __NX_STRING_H__
#define __NX_STRING_H__

#include <stdint.h>

int32_t NX_StringUpper( char *pString );
int32_t NX_StringLower( char *pString );
int32_t NX_StringTrim( char *pString );

int32_t NX_GetString( char *pData, int32_t iIndex, char *pResult );
int32_t NX_GetStringParse( char *pData, const char *pFormat, ... );
int32_t NX_GetStringNum( char *pData );

#endif	// __NX_STRING_H__
