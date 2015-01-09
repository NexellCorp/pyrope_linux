#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <nx_dsp.h>
#include <NX_FilterMoviePlay.h>

#define	EVENT_ENABLE_VIDEO					0x0010
#define	EVENT_VIDEO_ENABLED					0x0020
#define	EVENT_CHANGE_DEFAULT_BUFFERING_TIME	0x0030
#define	EVENT_CHANGE_STREAM_INFO			0x0040
#define	EVENT_END_OF_STREAM					0x1000
#define	EVENT_DEMUX_ERROR					0x10000					
#define	EVENT_VIDEO_DECODER_ERROR			0x10001
#define	EVENT_VIDEO_RENDER_ERROR			0x10002
#define	EVENT_AUDIO_DECODER_ERROR			0x10003
#define	EVENT_AUDIO_CONVERT_ERROR			0x10004
#define	EVENT_AUDIO_RENDER_ERROR			0x10005

//#define SEEK_TEST

typedef struct AppData{
	MP_HANDLE 		hPlayer;
	char			*pUri;
	Media_Info		mediaInfo;	
	
	int32_t			vidReqNumber;
	int32_t			audReqNumber;

	int32_t			volume;

	int32_t 		dspPort;
	int32_t			dspModule;
	int32_t			dspWidth;
	int32_t			dspHeight;

	int32_t			bRun;
} AppData;

static void cbEventCallback( void *privateDesc, unsigned int EventType, unsigned int EventData, unsigned int param2 )
{
	AppData *pAppData = (AppData*)privateDesc;

	if( EventType == EVENT_END_OF_STREAM )
	{
		printf("%s(): End of stream. ( privateDesc = 0x%08x )\n", __func__, (int32_t)pAppData );
		if( pAppData )
		{
			pAppData->bRun = false;
		}
	}
	else if( EventType == EVENT_DEMUX_ERROR )
	{
		printf("%s(): Cannot play contents.\n", __func__);
		if( pAppData )
		{
			pAppData->bRun = false;
			
		}
	}
}

static int32_t GetScreenInfo( int32_t *width, int32_t *height )
{
	int32_t fb;
	struct  fb_var_screeninfo  fbvar;

	*width = *height = 0;

	fb = open( "/dev/fb0", O_RDWR);
	ioctl( fb, FBIOGET_VSCREENINFO, &fbvar);

	*width  = fbvar.xres;
	*height = fbvar.yres;
	if( fb ) close( fb );

	return 0;
}

#ifdef SEEK_TEST
static int64_t GetTickCount( void )
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );

	return ((int64_t)tv.tv_sec)*1000 + (int64_t)(tv.tv_usec/1000);		
}
#endif

static void print_usage( const char *appName )
{
	printf("usage: %s [options]\n", appName);
	printf(" -f [file name]     : file name\n");
}

int32_t main( int32_t argc, char *argv[] )
{	
	int32_t opt = 0;
	uint32_t duration = 0, position = 0;

#ifdef SEEK_TEST
	int64_t prevTime = GetTickCount();
#endif

	AppData appData;
	memset( &appData, 0x00, sizeof(appData) );
	if( 2 > argc )
	{
		print_usage( argv[0] );
		return -1;
	}

	while( -1 != (opt=getopt(argc, argv, "hf:")))
	{
		switch( opt ){
			case 'h':
				print_usage( argv[0] );
				return 0;
			case 'f':
				appData.pUri = strdup( optarg );
				break;
			default:
				break;
		}
	}

	if( !appData.pUri ) {
		print_usage(argv[0]);
		return -1;
	}

	// Confiugratino Parameter
	appData.vidReqNumber 	= 1;
	appData.audReqNumber 	= 1;

	appData.volume			= 100;

	appData.dspPort			= DISPLAY_PORT_LCD;
	appData.dspModule		= DISPLAY_MODULE_MLC0;
	
	appData.bRun			= true;
	GetScreenInfo( &appData.dspWidth, &appData.dspHeight );

	NX_DspVideoSetPriority( DISPLAY_PORT_LCD, DISPLAY_MODULE_MLC0 );
	NX_MPSetFileName( &appData.hPlayer, appData.pUri, &appData.mediaInfo );

	if( !appData.hPlayer )
		return -1;

	NX_MPOpen( appData.hPlayer,
		appData.audReqNumber, appData.vidReqNumber, 
		DISPLAY_LCD, 
		(void *)&appData.volume, 
		(void *)&appData.dspModule, (void *)&appData.dspPort,
		&cbEventCallback, 
		&appData);
	
	NX_MPPlay( appData.hPlayer, 1 );
	NX_MPSetVolume( appData.hPlayer, appData.volume );
	NX_MPSetDspPosition( appData.hPlayer, appData.dspModule, appData.dspPort, 0, 0, appData.dspWidth, appData.dspHeight );

	while( appData.bRun )
	{
#ifdef SEEK_TEST
		if( 10000 < (GetTickCount() - prevTime) ) {
			prevTime = GetTickCount();

			NX_MPGetCurDuration( appData.hPlayer, &duration );
			NX_MPGetCurPosition( appData.hPlayer, &position );

			printf("curPos = %d, seekPos = %d\n", position, position - 5000);
			NX_MPSeek( appData.hPlayer, position - 5000);
		}

		usleep( 100000 );	
#else
		NX_MPGetCurDuration( appData.hPlayer, &duration );
		NX_MPGetCurPosition( appData.hPlayer, &position );
		printf("position( %d mSec ) / duration( %d mSec )\n", position, duration);

		usleep( 500000 );
#endif		
	}

	if( appData.hPlayer) NX_MPStop( appData.hPlayer );
	if( appData.hPlayer) NX_MPClose( appData.hPlayer );

	if( appData.pUri ) free( appData.pUri );

	return 0;
}
