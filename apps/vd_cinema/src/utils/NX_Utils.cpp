#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include <NX_Utils.h>

uint64_t NX_GetTickCount( void )
{
	uint64_t ret;
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
	return ret;
}

void dumpdata( void *data, int32_t len, const char *msg )
{
	int32_t i=0;
	uint8_t *byte = (uint8_t *)data;
	printf("Dump Data : %s", msg);
	for( i=0 ; i<len ; i ++ )
	{
		if( i!=0 && i%16 == 0 )	printf("\n\t");
		printf("%.2x", byte[i] );
		if( i%4 == 3 ) printf(" ");
	}
	printf("\n");
}

void HexDump( const void *data, int32_t size )
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

//
// NX_SHELL_GetArgument
//
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
