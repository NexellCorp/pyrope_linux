#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	//	getopt & optarg
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#include <NX_FilterMoviePlay.h>
#include <uevent.h>

#define ON	1
#define OFF	0

#define	SCREEN_WIDTH	1024
#define	SCREEN_HEIGHT	600

typedef struct CommandBuffer{
	int		cmd_cnt;	
	char	cmd[10][10];	
}CommandBuffer;

#define			MAX_COMMAND_QUEUE		1024
pthread_mutex_t	CmdMutex;
pthread_mutex_t	HdmiMutex;
pthread_t		hHdmiThread;
pthread_t		hCmdThread;


char uri_tmp[1024];
unsigned int  position_save = 0;

typedef struct Static_player_st{
	int display;
	int	hdmi_detect;
	int	hdmi_detect_init;
	int	volume;
	int	audio_request_track_num;
	int	video_request_track_num;
}Static_player_st;

Static_player_st	static_player;

struct AppData{
	MP_HANDLE hPlayer;
};



typedef struct AppData AppData;

#define		EVENT_ENABLE_VIDEO					(0x0010)		//	Require Video Layer Control
#define		EVENT_VIDEO_ENABLED					(0x0020)		//	Video Layer Enabled : Enabled Display
#define		EVNT_CHANGE_DEFAULT_BUFFERING_TIME	(0x0030)		//	Change default buffering time
#define		EVENT_CHANGE_STREAM_INFO			(0x0040)		//	Change Stream Information

#define		EVENT_END_OF_STREAM					(0x1000)		//	End of stream event

#define		EVENT_DEMUX_ERROR					(0x10000)					
#define		EVENT_VIDEO_DECODER_ERROR			(EVENT_DEMUX_ERROR + 1)		
#define		EVENT_VIDEO_RENDER_ERROR			(EVENT_VIDEO_DECODER_ERROR + 1)		//	
#define		EVENT_AUDIO_DECODER_ERROR			(EVENT_VIDEO_RENDER_ERROR + 1)		//	
#define		EVENT_AUDIO_CONVERT_ERROR			(EVENT_AUDIO_DECODER_ERROR + 1)		//	
#define		EVENT_AUDIO_RENDER_ERROR			(EVENT_AUDIO_CONVERT_ERROR + 1)		//	

typedef struct AppData AppData;
static void callback(void *privateDesc, unsigned int EventType, unsigned int EventData, unsigned int param2)
{

	if (EventType == EVENT_END_OF_STREAM)
	{
		printf("App : callback(privateDesc = %d)\n", privateDesc);
		if (privateDesc)
		{
			AppData *appData = (AppData*)privateDesc;
			if (appData->hPlayer)
			{
				printf("NX_MPStop ++\n");
				NX_MPStop(appData->hPlayer);
				printf("NX_MPStop --\n");
			}
		}
		printf("CALLBACK_MSG_EOS\n");
	}
	else if (EventType == EVENT_DEMUX_ERROR)
	{
		printf("Cannot Play Contents\n");

	}

}

void print_usage(const char *appName)
{
	printf( "usage: %s [options]\n", appName );
	printf( "    -f [file name]     : file name\n" );
	printf( "    -s                 : shell command mode\n" );
}

int main( int argc, char *argv[] )
{	

	int opt = 0, count = 0;
	unsigned int duration = 0, position = 0;
	int shell_mode = 0;
	static char uri[2048];
	MP_HANDLE handle = NULL;

	int audio_request_track_num = 1;						//multi track, default 0
	int video_request_track_num = 1;						//multi track, default 0

	Media_Info media_info;

	int display = DISPLAY_LCD;


	int volumem, dspModule = 0, dspPort = 0;			//	Currently can support only dspModule = 0 and dspPort = 0

	if( 2>argc )
	{
		print_usage( argv[0] );
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "hsf:")))
	{
		switch( opt ){
			case 'h':	print_usage(argv[0]);		return 0;
			case 'f':	strcpy(uri, optarg);		break;
			case 's':	shell_mode = 1;				break;		//test -f test.mp4 -s
			default:								break;
		}
	}

	if( shell_mode )
	{
//		return shell_main( uri );
	}


	//
//	NX_DspVideoSetPriority(0, 0);
	NX_MPSetFileName(&handle, uri, &media_info);

	printf("NX_MPOpen ++\n");
	NX_MPOpen(handle, audio_request_track_num, video_request_track_num, display,	(void *)&volumem, (void *)&dspModule, (void *)&dspPort,
		&callback, NULL);
	
	if( handle )
	{

		printf("Init Play Done\n");

		if( NX_MPPlay( handle, 1 ) != 0 )
		{
			printf("NX_MPPlay failed\n");
		}
		printf("Start Play Done\n");
	}
	count = 0;

	//	We are set always full screen mode.
	NX_MPSetDspPosition(handle, dspModule, dspPort, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	while(1)
	{
		usleep(300000);

		NX_MPGetCurDuration(handle, &duration);
		NX_MPGetCurPosition(handle, &position);

		printf("Postion : %d / %d msec\n", position, duration);

		count ++;	
	}

}