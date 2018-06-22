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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "NX_Token.h"

//------------------------------------------------------------------------------
int32_t NX_GetToken( char *pData, int32_t iIndex, char *pResult )
{
	char szToken[64];
	int32_t iReadCnt = 0;
	int32_t iRet = -1;

	while( 0 < sscanf( pData, "%s", szToken ) )
	{
		pData = strstr( pData, szToken ) + strlen( szToken ) + 1;
		if( iReadCnt == iIndex )
		{
			sprintf( pResult, "%s", szToken );
			iRet = 0;
			break;
		}
		iReadCnt++;
	}

	return iRet;
}

//------------------------------------------------------------------------------
int32_t NX_GetTokenParse( char *pData, const char *pFormat, ... )
{
	if( NULL == pData )
		return -1;

	va_list args;
	va_start( args, pFormat );
	int32_t iRet = vsscanf( pData, pFormat, args );
	va_end( args );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t NX_GetTokenNum( char *pData )
{
	char szToken[64];
	int32_t iTokenNum = 0;

	while( 0 < sscanf( pData, "%s", szToken ) )
	{
		pData = strstr( pData, szToken ) + strlen( szToken ) + 1;
		iTokenNum++;
	}

	return iTokenNum;
}