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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include <NX_Utils.h>

//------------------------------------------------------------------------------
uint64_t NX_GetTickCount( void )
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	return ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
}

//------------------------------------------------------------------------------
int32_t NX_GetRandomValue( int32_t iStartNum, int32_t iEndNum )
{
	if( iStartNum >= iEndNum )
		return -1;

	srand( (uint32_t)NX_GetTickCount() );
	return rand() % (iEndNum - iStartNum + 1) + iStartNum;
}

//------------------------------------------------------------------------------
void NX_WaitTime( uint64_t iWaitTime, const char *pMsg, const char *tag )
{
	uint64_t iCurTime = NX_GetTickCount();
	uint64_t iTimeout = iCurTime + iWaitTime;
	uint64_t iMiliSecond = 0;

	char msg[256] = "";
	if( NULL != pMsg )
		snprintf(msg, sizeof(msg), "[%s] ", pMsg);

#ifndef ANDROID
	fprintf(stdout, "Wait time : %llu sec.\n", iWaitTime / 1000);
	fflush(stdout);
#else
	__android_log_print(ANDROID_LOG_DEBUG, tag, "%sWait time : %llu mSec.\n", msg, iWaitTime);
#endif

	while( iCurTime < iTimeout )
	{
#ifndef ANDROID
		fprintf( stdout, "Wait %llu mSec\r", iMiliSecond );
		fflush(stdout);
#else
		__android_log_print(ANDROID_LOG_DEBUG, tag, "%sWait %llu mSec\r", msg, iMiliSecond * 100);
#endif

		iCurTime = NX_GetTickCount();
		usleep(100000);
		iMiliSecond++;
	}

#ifndef ANDROID
	printf("\n");
#endif
}

//------------------------------------------------------------------------------
void NX_MakeDirectory( const char *pFormat, ... )
{
	va_list args;
	char szBuf[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szBuf, sizeof(szBuf), pFormat, args);
	va_end(args);

	char *pBuf = szBuf;
	szBuf[sizeof(szBuf)-1] = 0x00;

	while( *pBuf )
	{
		if( '/' == *pBuf )
		{
			*pBuf = 0x00;
			if( 0 != access( szBuf, F_OK ) && (pBuf != szBuf) )
			{
				printf("Make Directory. ( %s )\n", szBuf);
				mkdir( szBuf, 0777 );
			}
			*pBuf = '/';
		}
		pBuf++;
	}

	if( 0 != access( szBuf, F_OK) )
	{
		printf("Make Directory. ( %s )\n", szBuf);
		mkdir( szBuf, 0777 );
	}
}

//------------------------------------------------------------------------------
int32_t NX_RemoveFile( const char *pFormat, ... )
{
	va_list args;
	char szName[1024] = { 0x00, };

	va_start(args, pFormat);
	vsnprintf(szName, sizeof(szName), pFormat, args);
	va_end(args);

	int32_t iRet = remove(szName);
	if( !iRet )
	{
		printf("Remove File. ( %s )\n", szName );
	}

	return iRet;
}

//------------------------------------------------------------------------------
static void HexDump( const void *data, int32_t size, const char *tag )
{
	int32_t i=0, offset = 0;
	char tmp[32];
	static char lineBuf[1024];
	const uint8_t *_data = (const uint8_t*)data;
	while( offset < size )
	{
		sprintf( lineBuf, "%08lx    ", (unsigned long)offset );
		for( i=0 ; i<16 ; ++i )
		{
			if( i == 8 ){
				strcat( lineBuf, " " );
			}
			if( offset+i >= size )
			{
				strcat( lineBuf, "   " );
			}
			else{
				sprintf(tmp, "%02x ", _data[offset+i]);
				strcat( lineBuf, tmp );
			}
		}
		strcat( lineBuf, "   " );

		//     Add ACSII A~Z, & Number & String
		for( i=0 ; i<16 ; ++i )
		{
			if( offset+i >= size )
			{
				break;
			}
			else{
				if( isprint(_data[offset+i]) )
				{
					sprintf(tmp, "%c", _data[offset+i]);
					strcat(lineBuf, tmp);
				}
				else
				{
					strcat( lineBuf, "." );
				}
			}
		}

		strcat(lineBuf, "\n");
#ifndef ANDROID
		printf( "%s", lineBuf );
#else
		__android_log_print(ANDROID_LOG_DEBUG, tag, "%s", lineBuf );
#endif
		offset += 16;
	}
}

//------------------------------------------------------------------------------
void NX_HexDump( const void *data, int32_t size, const char *msg )
{
#ifndef ANDROID
	printf("%s ( %d bytes ):\n", msg, size);
#else
	__android_log_print(ANDROID_LOG_DEBUG, "", "%s ( %d bytes ):", msg, size );
#endif

	HexDump( data, size, "" );
}

//------------------------------------------------------------------------------
void NX_HexDump( const void *data, int32_t size, const char *tag, const char *msg )
{
#ifndef ANDROID
	printf("%s ( %d bytes ):\n", msg, size);
#else
	__android_log_print(ANDROID_LOG_DEBUG, tag, "%s ( %d bytes ):", msg, size );
#endif

	HexDump( data, size, tag );
}

//------------------------------------------------------------------------------
void NX_HexDump( const void *data, int32_t size )
{
	HexDump( data, size, "" );
}

//------------------------------------------------------------------------------
int32_t NX_CompareData( uint8_t *pData1, int32_t iSize1, uint8_t *pData2, int32_t iSize2 )
{
	if( iSize1 != iSize2 )
	{
		// NxDbgMsg(NX_DBG_DEBUG, "Fail, Mismatch Size.\n");
		return -1;
	}

	for( int32_t i = 0; i < iSize1; i++ )
	{
		if(  pData1[i] != pData2[i] )
		{
			// NxDbgMsg(NX_DBG_DEBUG, "Fail, Mismatch Data.\n");
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_SHELL_GetArgument( char *pSrc, char arg[][NX_SHELL_MAX_STR] )
{
	int32_t	i, j;

	// Reset all arguments
	for( i=0 ; i<NX_SHELL_MAX_ARG ; i++ )
	{
		arg[i][0] = 0;
	}

	for( i=0 ; i<NX_SHELL_MAX_ARG ; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )	pSrc++;

		// check end of string.
		if( *pSrc == 0 )  		break;

		j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) )
		{
			arg[i][j] = *pSrc++;
//			if( arg[i][j] >= 'a' && arg[i][j] <= 'z' ) 		// to upper char
//				arg[i][j] += ('A' - 'a');

			j++;
			if( j > (NX_SHELL_MAX_STR-1) ) 	j = NX_SHELL_MAX_STR-1;
		}

		arg[i][j] = 0;
	}

	return i;
}
