#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#include <NX_MoviePlay.h>
#include <NX_TypeFind.h>
#include <uevent.h>

#define			MAX_COMMAND_QUEUE		1024

typedef struct CommandBuffer {
	int32_t		cmd_cnt;	
	char		cmd[10][10];	
} CommandBuffer;

typedef struct AppData {
	// Player Handle, Uri, Info
	MP_HANDLE 		hPlayer;
	char			uri[1024];
	TYMEDIA_INFO	input_media_info;

	// Player Parameter
	int32_t			volume;

	int32_t			video_request_track_num;
	int32_t			audio_request_track_num;

	int32_t			display;
	int32_t			dsp_port;
	int32_t 		dsp_module;
	int32_t			hdmi_detect;
	int32_t			hdmi_detect_init;

	// Player Status
	int32_t			position_save;
	int32_t			isPaused;

	// Internal Thread
	pthread_t		hCmdThread;	
	pthread_mutex_t CmdMutex;

	pthread_t		hHdmiThread;
	pthread_mutex_t HdmiMutex;

	int32_t			thread_flag;

	// Command Queue
	CommandBuffer	CommandQueue[MAX_COMMAND_QUEUE];
	int32_t			CmdHead;
	int32_t			CmdTail;
	int32_t			NumCmd;

	// Application Mode
	int32_t			shell_mode;
} AppData;


////////////////////////////////////////////////////////////////////////////////
//
//	G-Streamer Type String
//
static const char *DemuxTypeString[20] =
{
//	mpegtsdemux					qtdemux						oggdemux						rmdemux
//	avidemux					asfdemux					matroskademux					flvdemux
//	mpegpsdemux					audio mp3					audio flac						audio m4a
//	audio wav					audio mpeg					audio ac3						audio dts
	"video/mpegts",				"video/quicktime",			"application/ogg",				"application/vnd.rn-realmedia",
	"video/x-msvideo",			"video/x-ms-asf",			"video/x-matroska",				"video/x-flv",
	"video/mpeg",				"application/x-id3",		"audio/x-flac",					"audio/x-m4a",
	"audio/x-wav",				"audio/mpeg",				"audio/x-ac3",					"audio/x-dts",
	"application/x-3gp",		"NULL",						"NULL",							"NULL",
};

static const char *AudioTypeString[15] =
{
//	AUDIO_TYPE_MPEG				AUDIO_TYPE_MP3				AUDIO_TYPE_AAC(mpeg4 lc)		AUDIO_TYPE_WMA
//	AUDIO_TYPE_OGG				AUDIO_TYPE_AC3				AUDIO_TYPE_AC3_PRI				AUDIO_TYPE_FLAC
//	AUDIO_TYPE_RA
	"audio/mpeg",				"audio/mp3",				"audio/aac",					"audio/x-wma",
	"audio/x-vorbis",			"audio/x-ac3",				"audio/x-private1-ac3",			"audio/x-flac",
	"audio/x-pn-realaudio",		"audio/x-dts",				"audio/x-private1-dts",			"audio/x-wav",
	"NULL",						"NULL",						"NULL",
};

static const char *VideoTypeString[12] =
{
// 	VIDEO_TYPE_H264				VIDEO_TYPE_H263				VIDEO_TYPE_MP4V(mpeg4 video)	VIDEO_TYPE_MP2V(mpeg2 video)
// 	VIDEO_TYPE_FLV				VIDEO_TYPE_RV(realvideo)	VIDEO_TYPE_DIVX					VIDEO_TYPE_ASF
// 	VIDEO_TYPE_WMV				VIDEO_TYPE_THEORA
	"video/x-h264",				"video/x-h263",				"video/mpeg",					"video/mpeg",
	"video/x-flash-video",		"video/x-pn-realvideo",		"video/x-divx",					"video/x-ms-asf",
	"video/x-wmv",				"video/x-theora",			"video/x-xvid",					"NULL",
};


////////////////////////////////////////////////////////////////////////////////
//
//	Player Event Callback
//
static void callback( void *privateDesc, uint32_t message, uint32_t param1, uint32_t param2 )
{
	AppData *pAppData = privateDesc;
	
	if( message == CALLBACK_MSG_EOS )
	{
		printf("%s(): End of stream. ( privateDesc = 0x%p )\n", __func__, privateDesc );
		if( pAppData )
		{
			NX_MPStop( pAppData->hPlayer );
			if( !pAppData->shell_mode ) pAppData->thread_flag = true;
		}
	}
	else if( message == CALLBACK_MSG_PLAY_ERR )
	{
		printf("%s(): Cannot player contents. ( privateDesc = 0x%p )\n", __func__, privateDesc );
		if( pAppData )
		{
			NX_MPStop( pAppData->hPlayer );
			if( !pAppData->shell_mode ) pAppData->thread_flag = true;
		}
	}
	else {
		printf("%s(): Unknown error.\n", __func__);
	}
}


////////////////////////////////////////////////////////////////////////////////
//
//	Usage & Debugging Print
//
static void print_usage( const char *appName )
{
	printf( "usage: %s [options]\n", appName );
	printf( "    -f [file name]     : file name\n" );
	printf( "    -s                 : shell command mode\n" );
}

static void shell_help( void )
{
	//               1         2         3         4         5         6         7
	//      12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n");
	printf("================================================================================\n");
	printf("                            Play Control Commands								\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("    open( or o) [filename] [Moudle] [Port] : open player for specific file		\n");
	printf("    close                                  : close player						\n");
	printf("    info                                   : show file information				\n");
	printf("    play                                   : play								\n");
	printf("    pause                                  : pause playing						\n");
	printf("    stop                                   : stop playing						\n");
	printf("    seek (or s) [milli seconds]            : seek								\n");
	printf("    status (or st)                         : display current player status		\n");
	printf("    p                                      : toggle play & pause				\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("                            Other Control Commands								\n");
	printf("--------------------------------------------------------------------------------\n");
	printf("    pos [module] [port] [x] [y] [w] [h]    : change play position				\n");
	printf("    vol [gain(%%), 0~100]                   : change volume(0:mute)				\n");
	printf("================================================================================\n");
	printf("\n");
}

static void typefind_debug(TYMEDIA_INFO *ty_handle)
{
	uint32_t i = 0;

	printf("\n");
	printf("===============================================================================\n");
	printf("    DemuxType : %d, %s \n", ty_handle->DemuxType, DemuxTypeString[ty_handle->DemuxType]);
	if( ty_handle->VideoTrackTotNum > 0 )
	{
		printf("-------------------------------------------------------------------------------\n");
		printf("                       Video Information \n");
		printf("-------------------------------------------------------------------------------\n");
		printf("    VideoTrackTotNum: %ld  \n",  ty_handle->VideoTrackTotNum);
		printf("\n");
		for(i = 0; i < ty_handle->VideoTrackTotNum; i++)
		{
			printf("    VideoTrackNum   : %ld	\n", ty_handle->VideoInfo[i].VideoTrackNum);
			printf("    VCodecType      : %ld, %s\n", ty_handle->VideoInfo[i].VCodecType, VideoTypeString[ty_handle->VideoInfo[i].VCodecType] );
			printf("    Width           : %ld  	\n", ty_handle->VideoInfo[i].Width);
			printf("    Height          : %ld  	\n", ty_handle->VideoInfo[i].Height);
			if(ty_handle->VideoInfo[i].Framerate.value_numerator == 0 || ty_handle->VideoInfo[i].Framerate.value_denominator == 0)
				printf("    Framerate       : %f	\n", 0.); 
			else
				printf("    Framerate       : %f  	\n", (double)(ty_handle->VideoInfo[i].Framerate.value_numerator/ty_handle->VideoInfo[i].Framerate.value_denominator));
			printf("\n");
			printf("\n");
		}
	}
	if( ty_handle->AudioTrackTotNum > 0 )
	{
		printf("-------------------------------------------------------------------------------\n");
		printf("                       Audio Information \n");
		printf("-------------------------------------------------------------------------------\n");
		printf("    AudioTrackTotNum: %ld  \n",  ty_handle->AudioTrackTotNum);
		printf("\n");
		for(i = 0; i < ty_handle->AudioTrackTotNum; i++)
		{
			printf("    AudioTrackNum   : %ld  	\n", ty_handle->AudioInfo[i].AudioTrackNum);
			printf("    ACodecType      : %ld, %s\n", ty_handle->AudioInfo[i].ACodecType, AudioTypeString[ty_handle->AudioInfo[i].ACodecType] );
			printf("    samplerate      : %ld  	\n", ty_handle->AudioInfo[i].samplerate);
			printf("    channels        : %ld  	\n", ty_handle->AudioInfo[i].channels);
			printf("\n");
			printf("\n");
		}
	}
	printf("===============================================================================\n\n");
}


////////////////////////////////////////////////////////////////////////////////
//
//	Command Handling
//
#define	SHELL_MAX_ARG	32
#define	SHELL_MAX_STR	1024

static int32_t GetArgument( char *pSrc, char arg[][SHELL_MAX_STR] )
{
	int32_t	i, j;

	// Reset all arguments
	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		arg[i][0] = 0;
	}

	for( i=0 ; i<SHELL_MAX_ARG ; i++ )
	{
		// Remove space char
		while( *pSrc == ' ' )
			pSrc++;
		// check end of string.
		if( *pSrc == 0 || *pSrc == '\n' )
			break;

		j=0;
		while( (*pSrc != ' ') && (*pSrc != 0) && *pSrc != '\n' )
		{
			arg[i][j] = *pSrc++;
			j++;
			if( j > (SHELL_MAX_STR-1) )
				j = SHELL_MAX_STR-1;
		}
		arg[i][j] = 0;
	}
	return i;
}

static int32_t PopCommand( AppData *pAppData, CommandBuffer *cmd )
{
	//	Command Pop Operation
	pthread_mutex_lock( &pAppData->CmdMutex );
	*cmd = pAppData->CommandQueue[ pAppData->CmdTail++ ];
	if( pAppData->CmdTail >= MAX_COMMAND_QUEUE )
	{
		pAppData->CmdTail = 0;
	}
	pAppData->NumCmd--;
	pthread_mutex_unlock( &pAppData->CmdMutex );
	return 0;
}

static int32_t PushCommand( AppData *pAppData, CommandBuffer *cmd )
{
	//	Command Push Operation
	pthread_mutex_lock( &pAppData->CmdMutex );
	pAppData->CommandQueue[ pAppData->CmdHead++ ] = *cmd;
	if( pAppData->CmdHead >= MAX_COMMAND_QUEUE )
	{
		pAppData->CmdHead = 0;
	}
	pAppData->NumCmd ++;
	pthread_mutex_unlock( &pAppData->CmdMutex );
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Thread ( HDMI Detect / Command Handling )
//
#define HDMI_STATE_FILE     "/sys/class/switch/hdmi/state"

void *HdmiDetectThread( void *arg )
{
	int32_t err;
	int32_t fd;
	uint32_t hdmi;
	struct pollfd fds[1];
	unsigned char uevent_desc[2048];
	char val;

	CommandBuffer cmd_st;
	AppData *pAppData = (AppData*)arg;

	pthread_mutex_init( &pAppData->HdmiMutex, NULL );
	uevent_init();

    fds[0].fd = uevent_get_fd();
    fds[0].events = POLLIN;

	fd = open(HDMI_STATE_FILE, O_RDONLY);
	if( fd > 0 ){
		if( read(fd, (char *)&val, 1) == 1 && val == '1' ) {
			printf("=== HDMI ON ==== %d\n", pAppData->hdmi_detect);
			if( pAppData->hdmi_detect != ON ){
				pAppData->hdmi_detect = ON;
				memset( &cmd_st, 0, sizeof(CommandBuffer) );
				cmd_st.cmd_cnt = 1;
				strcpy( cmd_st.cmd[0], "display" );
				PushCommand( pAppData, &cmd_st );
			}
			
		}else {
			printf("=== HDMI OFF ==== %d\n", pAppData->hdmi_detect);
			if( pAppData->hdmi_detect != OFF ){
				pAppData->hdmi_detect = OFF;
				memset( &cmd_st, 0, sizeof(CommandBuffer) );
				cmd_st.cmd_cnt = 1;
				strcpy( cmd_st.cmd[0], "display" );
				PushCommand( pAppData, &cmd_st );
			}
		}
		if( fd ) close(fd);
	}

	while( !pAppData->thread_flag )
	{
        err = poll(fds, 1, 1000);

		if( (pAppData->hdmi_detect == ON) && (pAppData->display != DISPLAY_PORT_DUAL))
		{
			pAppData->hdmi_detect = ON;
			memset(&cmd_st, 0, sizeof(CommandBuffer));
			cmd_st.cmd_cnt = 1;
			strcpy(cmd_st.cmd[0], "display");
			//printf("=== display ==== \n");
			PushCommand( pAppData, &cmd_st );
		}

		if( err > 0 ) {
			if( fds[0].revents & POLLIN ) {
				//int32_t len = uevent_next_event((char *)uevent_desc, sizeof(uevent_desc) - 2);
				hdmi = !strcmp((const char *)uevent_desc, (const char *)"change@/devices/virtual/switch/hdmi");
				if( hdmi ) {
					fd = open(HDMI_STATE_FILE, O_RDONLY);
					if( fd < 0 ) {
						printf("failed to open hdmi state fd: %s", HDMI_STATE_FILE);
					}
					else if( fd > 0 ) {
						if( read(fd, &val, 1) == 1 && val == '1' ) {
							//printf("=== HDMI ON ==== %d, %d\n", pAppData->hdmi_detect, pAppData->display );
							if( pAppData->hdmi_detect != ON ) {
								pAppData->hdmi_detect = ON;
								memset( &cmd_st, 0, sizeof(CommandBuffer) );
								cmd_st.cmd_cnt = 1;
								strcpy( cmd_st.cmd[0], "display" );
								PushCommand( pAppData, &cmd_st );
							}						
						} else {
							//printf("=== HDMI OFF ==== %d\n", pAppData->hdmi_detect);
							if( pAppData->hdmi_detect != OFF ){
								pAppData->hdmi_detect = OFF;
								memset( &cmd_st, 0, sizeof(CommandBuffer) );
								cmd_st.cmd_cnt = 1;
								strcpy( cmd_st.cmd[0], "display" );
								PushCommand( pAppData, &cmd_st );
							}
						}
						if( fd ) close(fd);
					}
				}
			}
		} else if( err == -1 ) {
			printf("error in vsync thread \n");
		}
    }
	pthread_mutex_destroy( &pAppData->HdmiMutex );

	return (void*)0xdeaddead;
}

void *CommandThread( void *arg )
{
	AppData *pAppData = (AppData*)arg;
	MP_RESULT mpResult = ERROR_NONE;

	static 	char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	int32_t i = 0, cmdCnt = 0;
	
	CommandBuffer cmd_st;

	pthread_mutex_init( &pAppData->CmdMutex, NULL );

	while( !pAppData->thread_flag )
	{
		//	1. Check Input Command
		if( pAppData->NumCmd < 1 ) {
			usleep( 30 );
			continue;
		}
		PopCommand( pAppData, &cmd_st );

		//	2. Prepare Command & Argument
		memset(cmd, 0, sizeof(cmd));
		cmdCnt = cmd_st.cmd_cnt;
		for(i = 0; i < cmdCnt; i++) {
			strcpy(cmd[i], cmd_st.cmd[i]);
		}

		//	3. Parsing Command & Argument
		if( (0 == strcasecmp( cmd[0], "display" )) ){
			if( pAppData->hdmi_detect == OFF )
			{
				if( pAppData->hPlayer ){
					NX_MPGetCurPosition( pAppData->hPlayer, (uint32_t*)&pAppData->position_save);
					NX_MPClose( pAppData->hPlayer );
					pAppData->hPlayer = NULL;
					
					pAppData->dsp_module	= DISPLAY_PORT_LCD;
					pAppData->dsp_port		= DISPLAY_MODULE_MLC0;
					pAppData->display 		= DISPLAY_PORT_LCD;

					if( ERROR_NONE != (mpResult = NX_MPSetFileName( &pAppData->hPlayer, pAppData->uri, (char *)&pAppData->input_media_info )) )
					{
						printf("Error : NX_MPSetFileName Failed! ( uri = %s, mpResult = %d )\n", pAppData->uri, mpResult);
						continue;	
					}

					mpResult = NX_MPOpen( pAppData->hPlayer,
										pAppData->volume, pAppData->dsp_module, pAppData->dsp_port,
										pAppData->audio_request_track_num, pAppData->video_request_track_num,
										pAppData->display, &callback, pAppData );

					if( ERROR_NONE != mpResult )
					{
						printf("Error : NX_MPOpen Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}

					if( ERROR_NONE != (mpResult = NX_MPPlay( pAppData->hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}

					if( ERROR_NONE != (mpResult = NX_MPSeek( pAppData->hPlayer, pAppData->position_save )) )
					{
						printf("Error : NX_MPSeek Failed! ( mpResult = %d )\n", mpResult);
					}

					pAppData->hdmi_detect_init = 0;
				}
			}
			else if(pAppData->hdmi_detect == ON)
			{
				if( pAppData->hPlayer ){
					NX_MPGetCurPosition( pAppData->hPlayer, (uint32_t*)&pAppData->position_save);
					NX_MPClose( pAppData->hPlayer );
					pAppData->hPlayer = NULL;

					pAppData->dsp_module	= DISPLAY_PORT_LCD;
					pAppData->dsp_port		= DISPLAY_MODULE_MLC0;
					pAppData->display 		= DISPLAY_PORT_DUAL;
					
					if( ERROR_NONE != (mpResult = NX_MPSetFileName( &pAppData->hPlayer, pAppData->uri, (char *)&pAppData->input_media_info))) {
						printf("Error : NX_MPSetFileName Failed! ( uri = %s, mpResult = %d )\n", pAppData->uri, mpResult);
						continue;	
					}

					mpResult = NX_MPOpen( pAppData->hPlayer,
										pAppData->volume, pAppData->dsp_module, pAppData->dsp_port,
										pAppData->audio_request_track_num, pAppData->video_request_track_num,
										pAppData->display, &callback, pAppData );

					if( ERROR_NONE != mpResult )
					{
						printf("Error : NX_MPOpen Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}

					pAppData->dsp_module = DISPLAY_PORT_HDMI;
					pAppData->dsp_port 	= DISPLAY_MODULE_MLC1;
					if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( pAppData->hPlayer, pAppData->dsp_module, pAppData->dsp_port, 0, 0, 1920, 1080 ) ) )
					{
						printf("Error : NX_MPSetDspPosition Failed! ( mpResult = %d )\n", mpResult);
					}

					if( ERROR_NONE != (mpResult = NX_MPPlay( pAppData->hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}

					if( ERROR_NONE != (mpResult = NX_MPSeek( pAppData->hPlayer, pAppData->position_save )) )
					{
						printf("Error : NX_MPSeek Failed! ( mpResult = %d )\n", mpResult);
					}

					pAppData->hdmi_detect_init = 1;
				}
			}
			else {
				printf("Warnning : HDMI is not connected!\n");
			}
		}
		else if( (0 == strcasecmp( cmd[0], "open" )) | (0 == strcasecmp( cmd[0], "o" )) ) 
		{
			if( cmdCnt > 3)
			{
				pAppData->dsp_module = atoi(cmd[1]);
				pAppData->dsp_port	 = atoi(cmd[2]);
				strcpy( pAppData->uri, cmd[3] );
			}
			else if( cmdCnt > 1 )
			{
				strcpy( pAppData->uri, cmd[1] );
			}
			else
			{
				if( pAppData->uri[0] == 0x00 )
				{
					printf("Error : Invalid argument!!!, Usage : open [filename]\n");
					continue;
				}
			}

			if( pAppData->hPlayer )
			{
				NX_MPClose( pAppData->hPlayer );
				pAppData->hPlayer = NULL;
			}

			if( pAppData->dsp_module == 1 && pAppData->dsp_port == 1 ){
				pAppData->display = DISPLAY_PORT_DUAL;
			}
			else if( pAppData->dsp_module == 0 && pAppData->dsp_port == 0 ){
				pAppData->display = DISPLAY_PORT_LCD;
			}

			if( ERROR_NONE != (mpResult = NX_MPSetFileName( &pAppData->hPlayer, pAppData->uri, (char *)&pAppData->input_media_info))) {
				printf("Error : NX_MPSetFileName Failed! ( uri = %s, mpResult = %d )\n", pAppData->uri, mpResult);
				continue;
			}

			mpResult = NX_MPOpen( pAppData->hPlayer,
								pAppData->volume, pAppData->dsp_module, pAppData->dsp_port,
								pAppData->audio_request_track_num, pAppData->video_request_track_num,
								pAppData->display, &callback, pAppData );

			if( ERROR_NONE != mpResult )
			{
				printf("Error : NX_MPOpen Failed! ( mpResult = %d )\n", mpResult);
				continue;
			}
			pAppData->isPaused = 0;

			if( pAppData->dsp_module == 1 && pAppData->dsp_port == 1 )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( pAppData->hPlayer, pAppData->dsp_module, pAppData->dsp_port, 0, 0, 1920, 1080 ) ) )
				{
					printf("Error : NX_MPSetDspPosition Failed! ( mpResult = %d )\n", mpResult);
					continue;
				}
			}

		}
		else if( 0 == strcasecmp( cmd[0], "close" ) )
		{
			if( pAppData->hPlayer ) {
				NX_MPClose( pAppData->hPlayer );
				pAppData->hPlayer	= NULL;
				pAppData->isPaused 	= 0;
			}
			else {
				printf("MediaPlayer is already closed or is not opened!\n");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "info" ) )
		{
			if( pAppData->hPlayer ) {
				MP_MEDIA_INFO media_info;

				if( ERROR_NONE != (mpResult = NX_MPGetMediaInfo( pAppData->hPlayer, 0, &media_info)) ) {
					printf("Error : NX_MPGetMediaInfo Failed! ( mpResult = %d )\n", mpResult);
					continue;
				}
			}
			else {
				printf("MediaPlayer is not opened!\n");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "play" ) )
		{
			printf("Play.\n");
			if( pAppData->hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPPlay( pAppData->hPlayer, 1.0 )) ) {
					printf("Error : NX_MPPlay Failed!( mpResult = %d )\n", mpResult);
					continue;
				}
				pAppData->isPaused = false;
			}
			else {
				printf("MediaPlayer is not opened!\n");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "pause" ) )
		{
			if( pAppData->hPlayer )
			{
				printf("Pause.\n");
				if( ERROR_NONE != (mpResult = NX_MPPause( pAppData->hPlayer )) )
				{
					printf("Error : NX_MPPause Failed! ( mpResult = %d )\n", mpResult);
					continue;
				}
				pAppData->isPaused = true;
			}
			else
			{
				printf("MediaPlayer is not opened!\n");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "p" ) )
		{
			if( pAppData->hPlayer )
			{
				if( pAppData->isPaused )
				{
					printf("Play.\n");
					if( ERROR_NONE != (mpResult = NX_MPPlay( pAppData->hPlayer, 1.0 )) )
					{
						printf("Error : NX_MPPlay Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}
					pAppData->isPaused = false;
				}
				else
				{
					printf("Pause.\n");
					if( ERROR_NONE != (mpResult = NX_MPPause( pAppData->hPlayer )) )
					{
						printf("Error : NX_MPPause Failed! ( mpResult = %d )\n", mpResult);
						continue;
					}
					pAppData->isPaused = true;
				}
			}
		}
		else if( 0 == strcasecmp( cmd[0], "stop" ) )
		{
			printf("Stop.\n");
			if( pAppData->hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPStop( pAppData->hPlayer )) )
				{
					printf("Error : NX_MPStop Failed! ( mpResult = %d )\n", mpResult);
					continue;
				}
			}
			else
			{
				printf("MediaPlayer is not opened!\n");
			}
		}
		else if( (0 == strcasecmp( cmd[0], "seek" )) || (0 == strcasecmp( cmd[0], "s" )) )
		{
			int32_t seekTime;
			if( cmdCnt < 2 )
			{
				printf("Error : Invalid argument !!!, Usage : seek (or s) [milli seconds]\n");
				continue;
			}

			seekTime = atoi( cmd[1] );

			if( pAppData->hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSeek( pAppData->hPlayer, seekTime )) )
				{
					printf("Error : NX_MPSeek Failed! ( mpResult = %d )\n", mpResult);
				}
			}
			else
			{
				printf("MediaPlayer is not opened!\n");
			}
		}
		else if( (0 == strcasecmp( cmd[0], "status" )) || (0 == strcasecmp( cmd[0], "st" )) )
		{
			if( pAppData->hPlayer )
			{
				uint32_t duration, position;
				NX_MPGetCurDuration(pAppData->hPlayer, &duration);
				NX_MPGetCurPosition(pAppData->hPlayer, &position);
				printf("Postion : %d / %d msec\n", position, duration);
			}
			else
			{
				printf("MediaPlayer is not initialized!!\n");
			}
		}
		else if( 0 == strcasecmp( cmd[0], "pos" ) )
		{
			int32_t x, y, w, h;
			if( cmdCnt < 7) {
				printf("\nError : Invalid argument, Usage : pos [module] [port] [x] [y] [w] [h]\n");
				continue;
			}

			pAppData->dsp_module= atoi(cmd[1]);
			pAppData->dsp_port 	= atoi(cmd[2]);
			x 					= atoi(cmd[3]);
			y 					= atoi(cmd[4]);
			w 					= atoi(cmd[5]);
			h 					= atoi(cmd[6]);

			if( pAppData->hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetDspPosition( pAppData->hPlayer, pAppData->dsp_module, pAppData->dsp_port, x, y, w, h) ) )
				{
					printf("Error : NX_MPSetDspPosition Failed!!!(%d)\n", mpResult);
				}
			}
		}
		else if( 0 == strcasecmp( cmd[0], "vol" ) )
		{
			if( cmdCnt < 2 ) {
				printf("Error : Invalid argument, Usage : vol [volume]\n");
				continue;
			}

			pAppData->volume = atoi( cmd[1] ) % 101;
			if( pAppData->volume > 100 )
			{
				pAppData->volume = 100;
			}
			else if( pAppData->volume < 0 )
			{
				pAppData->volume = 0;
			}

			if( pAppData->hPlayer )
			{
				if( ERROR_NONE != (mpResult = NX_MPSetVolume( pAppData->hPlayer, pAppData->volume ) ) )
				{
					printf("Error : NX_MPSeek Failed! ( mpResult = %d )\n", mpResult);
				}
			}
		}
		else if( (0 == strcasecmp( cmd[0], "help" )) || (0 == strcasecmp( cmd[0], "h" )) || (0 == strcasecmp( cmd[0], "?" )) )
		{
			shell_help();
		}
		else
		{
			if( cmdCnt != 0 ) {
				printf("Unknown command : %s (cmdSize=%d), cmdCnt=%d\n", cmd[0], strlen(cmd[0]), cmdCnt );
			}
		}
	}

	if( pAppData->hPlayer )
	{
		NX_MPClose( pAppData->hPlayer );
		pAppData->hPlayer = NULL;
	}

	pthread_mutex_destroy( &pAppData->CmdMutex );

	return (void*)0xDEADDEAD;
}

static int32_t shell_main( AppData *pAppData )
{
	static char cmdstring[SHELL_MAX_ARG * SHELL_MAX_STR];
	static char cmd[SHELL_MAX_ARG][SHELL_MAX_STR];
	
	int32_t i = 0, cmdCnt = 0;
	CommandBuffer cmdBuffer;

	if( pthread_create( &pAppData->hHdmiThread, NULL, HdmiDetectThread, (void*)pAppData ) != 0 )
	{
		return -2;
	}

	if( pthread_create( &pAppData->hCmdThread, NULL, CommandThread, (void*)pAppData ) != 0 )
	{
		return -2;
	}

	shell_help();

	while( !pAppData->thread_flag )
	{
		printf("File Player> ");

		//	1. command parsing
		memset(cmd, 0, sizeof(cmd));
		fgets( cmdstring, sizeof(cmdstring), stdin );
		cmdCnt = GetArgument( cmdstring, cmd );

		//	2. push command
		memset( &cmdBuffer, 0, sizeof(CommandBuffer) );
		cmdBuffer.cmd_cnt = cmdCnt;
		for( i = 0; i < cmdCnt; i++ ) {
			strcpy( cmdBuffer.cmd[i], cmd[i] );
		}
		PushCommand( pAppData, &cmdBuffer );

		//	3. exit command
		if( (0 == strcasecmp( cmd[0], "exit" )) || (0 == strcasecmp( cmd[0], "q" )) ) {
			pAppData->thread_flag = true;
		}
	}

	pthread_join( pAppData->hHdmiThread, NULL );
	pthread_join( pAppData->hCmdThread, NULL );

	return 0;
}

int32_t main( int32_t argc, char *argv[] )
{	
	AppData appData;
	MP_RESULT mpResult = ERROR_NONE;

	int32_t opt = 0, count = 0;
	int32_t aging = 0, agingCnt = 0, shellMode = 0;
	uint32_t duration = 0, position = 0;
	char *uri = NULL;

	if( 2 > argc ) {
		print_usage( argv[0] );
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "ahsf:")))
	{
		switch( opt ){
			case 'h':	print_usage( argv[0] );		 0;
			case 'f':	uri = strdup( optarg );		break;
			case 's':	shellMode = 1;				break;
			case 'a':	aging = 1;					break;
			default:								break;
		}
	}

AGING_LOOP:
	if( aging )
	{
		printf("\n==================== Aging Count = %d ====================\n", ++agingCnt);
	}

	//	Initialize Parameters
	count = 0;
	duration = position = 0;

	memset( &appData, 0x00, sizeof(AppData) );
	// appData Configuration
	appData.shell_mode				= shellMode;
	appData.audio_request_track_num = 0;
	appData.video_request_track_num = 0;
	appData.volume					= 10;
	appData.display					= DISPLAY_PORT_LCD;
	appData.dsp_port				= DISPLAY_PORT_LCD;
	appData.dsp_module				= DISPLAY_MODULE_MLC0;
	strcpy(appData.uri, uri);

	if( appData.shell_mode )
	{
		free(uri);
		return shell_main( &appData );
	}

	// 	0. Check Paramter 
	if( appData.uri[0] == 0x00 ) {
		printf("Error : Uri is invalid.\n");
		return -1;
	}

	//	1. Set FileName
	mpResult = NX_MPSetFileName( &appData.hPlayer, appData.uri, (char *)&appData.input_media_info );
	if( ERROR_NONE != mpResult ) {
		printf("Error : NX_MPSetFileName Failed! ( uri = %s, mpResult = %d )\n", appData.uri, mpResult);
		return -1;
	}
	
	if( appData.hPlayer == NULL ) {
		printf("Error : handle is NULL!\n");
		return -1;
	}
	printf("handle = %p\n", appData.hPlayer);

	//	2. Media Player Open
	NX_MPOpen( appData.hPlayer,
			   appData.volume, appData.dsp_module, appData.dsp_port,
			   appData.audio_request_track_num, appData.video_request_track_num,
			   appData.display, &callback, &appData );

	if( ERROR_NONE != mpResult ) {
		printf("Error : NX_MPOpen Failed! ( mpResult = %d )\n", mpResult);
		return -1;
	}



	//	3. Media Player Start
	mpResult = NX_MPPlay( appData.hPlayer, 1 );
	if(  ERROR_NONE != mpResult ) {
		printf("Error : NX_MPPlay Failed! ( mpResult = %d )\n", mpResult);
		return -1;
	}

	//	4. Loop wait
	while( !appData.thread_flag )
	{
		usleep(300000);

		NX_MPGetCurDuration( appData.hPlayer, &duration );
		NX_MPGetCurPosition( appData.hPlayer, &position );

		printf("Postion : %d / %d msec\n", position, duration);

		// Test Code
		if( ++count == 50 )
		{
			// NX_MPSeek( hPlayer, 90000 );
			// NX_MPPause( hPlayer );
			// NX_MPSetVolume( hPlayer, 0 );
		}
		if( aging && count>10 )
			break;
	}

printf("NX_MPStop++\n");
	NX_MPStop(appData.hPlayer);
printf("NX_MPStop--\n");
	NX_MPClose(appData.hPlayer);
printf("NX_MPClose--\n");
	appData.hPlayer = NULL;


	if( aging )
	{
		goto AGING_LOOP;
	}
	if( uri )
		free(uri);

	return 0;
}
