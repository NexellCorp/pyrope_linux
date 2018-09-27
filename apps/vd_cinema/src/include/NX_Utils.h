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

#ifndef __NX_UTILS_H__
#define __NX_UTILS_H__

#include <stdint.h>

#define	NX_SHELL_MAX_ARG		1024
#define	NX_SHELL_MAX_STR		64

uint64_t	NX_GetTickCount( void );
int32_t		NX_GetRandomValue( int32_t iStartNum, int32_t iEndNum );

void		NX_HexDump( const void *data, int32_t size, const char *msg );
void		NX_HexDump( const void *data, int32_t size );

int32_t		ping( const char *target );

int32_t		NX_SHELL_GetArgument (char *pSrc, char arg[][NX_SHELL_MAX_STR] );

#endif	// __NX_UTILS_H__
