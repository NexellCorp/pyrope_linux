#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <NX_SecureLinkServer.h>

int main( int32_t argc, char *argv[] )
{
	if( argc < 2 )
	{
		printf("Usage : %s [on or off]\n", argv[0]);
	}

	if( !strcasecmp( argv[1], "off" ) )
	{
		NX_SapPowerOn( 0 );
	}
	else if( !strcasecmp( argv[1], "on" ) )
	{
		NX_SapPowerOn( 1 );
	}
	printf("Done\n");
	return 0;
}
