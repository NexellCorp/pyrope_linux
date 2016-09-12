#include <unistd.h>	//usleep
#include <stdio.h>

#include <NX_TMSServer.h>
#include <NX_TMSClient.h>
#include <NX_TMSCommand.h>


int32_t main()
{
	uint8_t buf[1024] = {0};
	int32_t size = sizeof(buf);

#if 0
	if( 0 == NX_PFPGACommand( PFPGA_CMD_STATE, buf, size ) )
	{
		printf( "PFPGA_CMD_STATE ok : status = %d\n", buf[0] );
	}

	if( 0 == NX_PFPGACommand( PFPGA_CMD_SOURCE, buf, size ) )
	{
		printf( "PFPGA_CMD_SOURCE ok\n");
	}

	if( 0 == NX_PFPGACommand( PFPGA_CMD_VERSION, buf, size ) )
	{
		printf( "PFPGA_CMD_SOURCE ok : version = %d.%d.%d.%d\n",
			buf[0], buf[1], buf[2], buf[3] );
	}
#endif


#if 0
	buf[0] = 0xff;
	NX_TConCommand( 1, TCON_CMD_STATE, buf, size );
#endif

	// buf[0] = 0x1;
	// NX_TConCommand( 1, TCON_CMD_STATE, buf, size );
	// NX_TConCommand( 1, TCON_CMD_OPEN, buf, size );
	// NX_TConCommand( 1, TCON_CMD_OPEN_POS, buf, size );
	// NX_TConCommand( 1, TCON_CMD_SHORT, buf, size );
	// NX_TConCommand( 1, TCON_CMD_SHORT_POS, buf, size );
	// NX_TConCommand( 1, TCON_CMD_DOOR_STATUS, buf, size );
	// NX_TConCommand( 1, TCON_CMD_ON, buf, size );
	// NX_TConCommand( 1, TCON_CMD_VERSION, buf, size );


	NX_TConCommand( 1, TCON_CMD_BR_CTRL, buf, &size );
	return 0;
}
