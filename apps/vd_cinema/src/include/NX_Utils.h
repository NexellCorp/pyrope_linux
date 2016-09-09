#ifndef __util_h__
#define __util_h__

#include <stdint.h>

#define	NX_SHELL_MAX_ARG		1024
#define	NX_SHELL_MAX_STR		64

void dumpdata( void *data, int32_t len, const char *msg );
void HexDump( const void *data, int32_t size );
uint64_t NX_GetTickCount( void );

int32_t ping( const char *target );

//	Shell Util
int32_t NX_SHELL_GetArgument (char *pSrc, char arg[][NX_SHELL_MAX_STR] );

#endif	//	__util_h__
