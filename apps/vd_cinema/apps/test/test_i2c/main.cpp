#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>	// usleep, getopt

#include <CNX_I2C.h>


static void help( char *appName )
{
	printf("\nUsage : %s [options]\n", appName );
	printf("\t-i : i2c port\n");
	printf("\t-a : address\n");
	printf("\t-r : register\n");
	printf("\n");
}

int32_t main( int32_t argc, char *argv[] )
{
	CNX_I2C *i2c;
	int32_t i=0, j=0;
	int32_t ret;
#if 0
	int32_t reg = 0;
	int32_t port = 1;

	int32_t i, opt;

	while( (opt = getopt(argc, argv, "ha:p:")) != -1 )
	{
		switch( opt )
		{
		case 'h':
			help(argv[0]);
			return 0;
		case 'p':
			port = atoi(optarg);
			break;
		case 'a':
			reg = atoi(optarg);
			break;
		case 'p':
			break;
		}
		if( opt == 'h' )
		{
			help(argv[0]);
			return 0;
		}
		else if
	}


#endif

	i2c = new CNX_I2C( atoi(argv[1]) );

	if( !i2c->Open() )
	{
		printf("Error : Open\n");
		return -1;
	}

	for( i=0 ; i<128 ; i++ )
	{
		ret = i2c->ReadNoStop( i , 0x13 );

		if( ret < 0 )
		{
			continue;
		}
		printf( "Slave ID %d = 0x%04x\n", i, ret );
	}

	i2c->Close();
	return 0;
}
