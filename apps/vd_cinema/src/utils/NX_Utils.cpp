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
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

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
void NX_WaitTime( uint64_t iWaitTime )
{
	uint64_t iTimeout = NX_GetTickCount() + iWaitTime;
	uint64_t iCurTime;
	uint64_t iMiliSecond = 0;

	{
#ifndef ANDROID
		fprintf(stdout, "Wait time : %llu sec.\n", iWaitTime / 1000);
		fflush(stdout);
#else
		__android_log_print( ANDROID_LOG_DEBUG, "", "Wait time : %llu mSec.\n", iWaitTime);
#endif
	}

	do {
#ifndef ANDROID
		fprintf( stdout, "Wait %llu mSec\r", iMiliSecond * 100 );
		fflush(stdout);
#else
		__android_log_print( ANDROID_LOG_DEBUG, "", "Wait %llu mSec\r", iMiliSecond * 100);
#endif

		iCurTime = NX_GetTickCount();
		usleep(100000);
		iMiliSecond++;
	} while( iCurTime <= iTimeout );

#ifndef ANDROID
	printf("\n");
#endif
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
void NX_HexDump( const void *data, int32_t size, const char *msg )
{
	const uint8_t *byte = (const uint8_t *)data;

	printf("%s ( %d bytes ):", msg, size);

	for( int32_t i = 0; i < size; ++i)
	{
		if ((i % 16) == 0)
		{
			printf("\n%04x", i);
		}

		printf(" %02x", byte[i]);
	}
}

//------------------------------------------------------------------------------
void NX_HexDump( const void *data, int32_t size )
{
	int32_t i=0, offset = 0;
	char tmp[32];
	static char lineBuf[1024];
	const uint8_t *_data = (const uint8_t*)data;
	while( offset < size )
	{
		sprintf( lineBuf, "%08lx :  ", (unsigned long)offset );
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
		__android_log_print(ANDROID_LOG_DEBUG, "", "%s", lineBuf );
#endif
		offset += 16;
	}
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
int32_t NX_SHELL_GetArgument (char *pSrc, char arg[][NX_SHELL_MAX_STR] )
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
