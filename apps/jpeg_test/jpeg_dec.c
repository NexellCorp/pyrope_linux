//------------------------------------------------------------------------------
//
//  Copyright (C) 2014 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      :
//  File        :
//  Description :
//  Author      : 
//  Export      :
//  History     :
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <nx_fourcc.h>
#include <nx_dsp.h>
#include <nx_jpeg.h>

#define DEFAULT_DSP_WIDTH		720
#define DEFAULT_DSP_HEIGHT		480

#define DECODE_FORMAT_MODE0		NX_JPEG_DECODE_FORMAT_YUV
#define DECODE_FORMAT_MODE1		NX_JPEG_DECODE_FORMAT_BGRA

#define INITIAL_COLOR			0x00000000	// Black

#define JPEG_FORMAT_STRING(A) 							\
	(A == NX_JPEG_FORMAT_YUV444)  		? "YUV444"  :	\
	(A == NX_JPEG_FORMAT_YUV422)  		? "YUV422"  :	\
	(A == NX_JPEG_FORMAT_YUV420)  		? "YUV420"  :	\
	(A == NX_JPEG_FORMAT_YUVGRAY) 		? "YUVGRAY" :	\
	(A == NX_JPEG_FORMAT_YUV440)  		? "YUV440"  :	\
	"Unknown"

#define JPEG_DECODE_FORMAT_STRING(A) 					\
	(A == NX_JPEG_DECODE_FORMAT_RGB)  	? "RGB"     :	\
	(A == NX_JPEG_DECODE_FORMAT_BGR)  	? "BGR"     :	\
	(A == NX_JPEG_DECODE_FORMAT_RGBX) 	? "RGBX"    :	\
	(A == NX_JPEG_DECODE_FORMAT_BGRX) 	? "BGRX"    :	\
	(A == NX_JPEG_DECODE_FORMAT_XBGR) 	? "XBGR"    :	\
	(A == NX_JPEG_DECODE_FORMAT_XRGB) 	? "XRGB"    :	\
	(A == NX_JPEG_DECODE_FORMAT_GRAY) 	? "GRAY"    :	\
	(A == NX_JPEG_DECODE_FORMAT_RGBA) 	? "RGBA"    :	\
	(A == NX_JPEG_DECODE_FORMAT_BGRA) 	? "BGRA"    :	\
	(A == NX_JPEG_DECODE_FORMAT_ABGR) 	? "ABGR"    :	\
	(A == NX_JPEG_DECODE_FORMAT_ARGB) 	? "ARGB"    :	\
	(A == NX_JPEG_DECODE_FORMAT_YUV)  	? "YUV"     :	\
	"Unknown"

static struct termios gstOldTermios, gstNewTermios;

static int32_t GetScreenInfo( int32_t *width, int32_t *height );
static int32_t FillFrameBuffer( int32_t width, int32_t height, uint8_t *pBuf, uint32_t pixel );

////////////////////////////////////////////////////////////////////////////////
//
//	Signal Handler
//
static void signal_handler(int sig)
{
	int32_t width, height;
	printf("Aborted by signal %s (%d)...\n", (char*)strsignal(sig), sig);

	switch(sig)
	{   
		case SIGINT :
			printf("SIGINT..\n");   break;
		case SIGTERM :
			printf("SIGTERM..\n");  break;
		case SIGABRT :
			printf("SIGABRT..\n");  break;
		default :
			break;
	}   

	// Resstore Terminal Configuration	
	tcsetattr( STDIN_FILENO, TCSANOW, &gstOldTermios );

	GetScreenInfo( &width, &height );
	FillFrameBuffer( width, height, NULL, INITIAL_COLOR );

	exit(EXIT_FAILURE);
}

static void register_signal(void)
{
	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Program Utility
//
static uint64_t gstCurTime;

static uint64_t GetSystemTick( void )
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	return ((uint64_t)tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

static void CHECK_TIME_START( int bEnable )
{
	if( !bEnable ) return;
	gstCurTime = GetSystemTick();
}

static void CHECK_TIME_STOP( int bEnable, const char *pLabel )
{
	if( !bEnable ) return;
	printf("%s : %lld msec\n", pLabel, GetSystemTick() - gstCurTime);
}

static int32_t getch( void )
{
	int32_t ch;
	tcgetattr( STDIN_FILENO, &gstOldTermios );
	gstNewTermios = gstOldTermios;
	gstNewTermios.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &gstNewTermios );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &gstOldTermios );
	return ch;
}

static int32_t GetScreenInfo( int32_t *width, int32_t *height )
{
	int32_t fb;
	struct	fb_var_screeninfo  fbvar;

	*width = *height = 0;

	fb = open( "/dev/fb0", O_RDWR);
	ioctl( fb, FBIOGET_VSCREENINFO, &fbvar);

	*width	= fbvar.xres;
	*height	= fbvar.yres;
	if( fb ) close( fb );

	return 0;
}

static int32_t FillFrameBuffer( int32_t width, int32_t height, uint8_t *pBuf, uint32_t pixel )
{
	int32_t fb, i, j;
	struct	fb_var_screeninfo  fbvar;

	int32_t	screen_width;
	int32_t	screen_height;
	int32_t bytes_per_pixel;

	void	*fb_map;
	uint8_t	*ptr;

	fb = open( "/dev/fb0", O_RDWR);
	ioctl( fb, FBIOGET_VSCREENINFO, &fbvar);

	screen_width	= fbvar.xres;
	screen_height	= fbvar.yres;
	bytes_per_pixel	= fbvar.bits_per_pixel / 8;
	
	fb_map = (void*)mmap( 0, screen_width * screen_height * bytes_per_pixel, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0 );

	for( i = 0; i < height; i++ )
	{
		for( j = 0; j < width * bytes_per_pixel; j++ )
		{
			ptr = (uint8_t*)fb_map + (screen_width * bytes_per_pixel) * i + j;
			
			if( pBuf ) {
				*ptr = pBuf[j + width * bytes_per_pixel * i];
			}
			else {
				if( 0 == (j % bytes_per_pixel) ) *ptr = (uint8_t)((pixel & 0xFF000000) >> 24);
				else if( 1 == (j % bytes_per_pixel) ) *ptr = (uint8_t)((pixel & 0x00FF0000) >> 16);
				else if( 2 == (j % bytes_per_pixel) ) *ptr = (uint8_t)((pixel & 0x0000FF00) >>  8);
				else if( 3 == (j % bytes_per_pixel) ) *ptr = (uint8_t)((pixel & 0x000000FF) >>  0);
			}
		}
	}

	munmap( fb_map, screen_width * screen_height * bytes_per_pixel );
	if( fb ) close( fb );

	return 0;
}

#if(0)
#define EVEN_ALIGNED(A) 	(A % 2) ? (A + 1) : A
#else
static int32_t EVEN_ALIGNED( int32_t value )
{
	return (value % 2) ? (value + 1) : value;
}
#endif


////////////////////////////////////////////////////////////////////////////////
//
//	Test Application
//
static void print_usage( char *pName )
{
	printf("Usage : %s [options]                                                   \n", pName);
	printf("   -h                  : Help                                                 \n");
	printf("   -i [file]           : Input filename.                 (mandatory)          \n");
	printf("   -o [file]           : Output filename. (Raw Format)   (optional)           \n");
	printf("   -m [mode]           : 0(YUV->VideoLayer), 1(RGB->FB)  (default : 0)        \n");
	printf("   -p [width],[height] : Display Region. (Support Mode0) (default : 720 x 480)\n");
}

int main( int argc, char *argv[] )
{
	int32_t opt;
	
	uint8_t *inFileName = NULL, *outFileName = NULL;
	FILE *outFile = NULL;

	uint8_t *outBuf;
	int64_t outSize;

	int32_t mode = 0;
	int32_t bCheckTime = 0;

	NX_JPEG_HEADER_INFO 	headerInfo = { 0, };
	NX_JPEG_BUFFER_ALIGN 	alignInfo = { 0, };

	NX_JPEG_HANDLE 			hJpeg = NULL;
	NX_VID_MEMORY_HANDLE 	hVidMem = NULL;
	DISPLAY_HANDLE			hDsp = NULL;
	
	int32_t scrWidth	= 0;
	int32_t scrHeight	= 0;

	int32_t dspWidth 	= DEFAULT_DSP_WIDTH;
	int32_t dspHeight 	= DEFAULT_DSP_HEIGHT;

	register_signal();

	while( -1 != (opt = getopt(argc, argv, "hi:o:m:p:t")) )
	{
		switch( opt )
		{
			case 'h' :
				print_usage(argv[0]);
				goto END;
			case 'i' :
				inFileName = (uint8_t*)strdup( optarg );
				break;
			case 'o' :
				outFileName = (uint8_t*)strdup( optarg );
				break;
			case 'm' :
				mode = atoi( optarg );
				break;
			case 'p' :
				sscanf(optarg, "%d,%d", &dspWidth, &dspHeight);
				break;
			case 't' :
				bCheckTime = 1;
				break;
		}
	}

	if( !inFileName ) {
		printf("Error: No input file.\n");
		print_usage(argv[0]);
		goto END;
	}

	if( outFileName ) {
		if( NULL == (outFile = fopen((char*)outFileName, "w")) ) {
			printf("Error: Cannot open file.\n");
			goto END;
		}
	}

	if( mode != 0 && mode != 1 ) {
		printf("Error: Unkown mode. ( 0 or 1 )\n");
		print_usage(argv[0]);
		goto END;
	}

	if( mode == 0 ) {
		if( dspWidth < 1 || dspHeight < 1 ) {
			printf("Error: Invalid display region. (w: %d, h: %d)\n", dspWidth, dspHeight);
			goto END;
		}
	}

	GetScreenInfo( &scrWidth, &scrHeight );
	
	CHECK_TIME_START( bCheckTime );
	hJpeg = NX_JpegOpen( inFileName );
	if( !hJpeg ) {
		printf("Error: Cannot create Jpeg Handle.\n");
		goto END;
	}
	CHECK_TIME_STOP( bCheckTime, "-> Jpeg Open" );

	CHECK_TIME_START( bCheckTime );
	NX_JpegGetHeaderInfo( hJpeg, &headerInfo );
	NX_JpegGetYuvBufferAlign( hJpeg, &alignInfo );
	CHECK_TIME_STOP( bCheckTime, "-> Jpeg Information" );

	printf("---------------------------------------------------------------------\n");
	if( inFileName )	printf(" Input File     : %s\n", inFileName);
	if( inFileName )	printf(" InFile Info    : width( %d ), height( %d ), format( %s )\n",
							headerInfo.width, headerInfo.height, JPEG_FORMAT_STRING(headerInfo.format));
	if( outFileName )	printf(" Output File    : %s\n", outFileName);
						printf(" OutFile Info   : output format( %s )\n", 
							(mode == 0) ? JPEG_DECODE_FORMAT_STRING(DECODE_FORMAT_MODE0) : JPEG_DECODE_FORMAT_STRING(DECODE_FORMAT_MODE1));
	if( mode == 0 )		printf(" Display Region : dspWidth( %d ) x dspHeight( %d )\n", dspWidth, dspHeight);
	printf("---------------------------------------------------------------------\n");

	if( mode == 0 ) {
		int64_t bufPos = 0, memPos = 0;
		int32_t i = 0;

		outSize = NX_JpegRequireBufSize( hJpeg, DECODE_FORMAT_MODE0 );
		outBuf 	= (uint8_t*)malloc( outSize );

		CHECK_TIME_START( bCheckTime );
		NX_JpegDecode( hJpeg, outBuf, DECODE_FORMAT_MODE0 );
		CHECK_TIME_STOP( bCheckTime, "-> Jpeg Decoding" );

		// Video Memory Allocate
		hVidMem = NX_VideoAllocateMemory( 4096, headerInfo.width, headerInfo.height, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );

		CHECK_TIME_START( bCheckTime );
		memPos = 0;
		for( i = 0; i < alignInfo.luHeight; i++ ) {
			memcpy( (char*)hVidMem->luVirAddr + memPos, outBuf + bufPos, alignInfo.luWidth );
			memPos += hVidMem->luStride;
			bufPos += alignInfo.luWidth;
		}
		memPos = 0;
		for( i = 0; i < alignInfo.cbHeight; i++ ) {
			memcpy( (char*)hVidMem->cbVirAddr + memPos, outBuf + bufPos, alignInfo.cbWidth );
			memPos += hVidMem->cbStride;
			bufPos += alignInfo.cbWidth;
		}
		memPos = 0;
		for( i = 0; i < alignInfo.crHeight; i++ ) {
			memcpy( (char*)hVidMem->crVirAddr + memPos, outBuf + bufPos, alignInfo.crWidth );
			memPos += hVidMem->crStride;
			bufPos += alignInfo.crWidth;
		}
		CHECK_TIME_STOP( bCheckTime, "-> Copy Buf to VidMem" );

		if( outFile ) {
			CHECK_TIME_START( bCheckTime );
			bufPos = 0;
			for( i = 0; i < EVEN_ALIGNED(headerInfo.height); i++ )
			{
				fwrite( (uint8_t*)hVidMem->luVirAddr + bufPos, EVEN_ALIGNED(headerInfo.width), 1, outFile );
				bufPos += hVidMem->luStride;
			}

			bufPos = 0;
			for( i = 0; i < EVEN_ALIGNED(headerInfo.height)/2; i++ )
			{
				fwrite( (uint8_t*)hVidMem->cbVirAddr + bufPos, EVEN_ALIGNED(headerInfo.width) / 2, 1, outFile );
				bufPos += hVidMem->cbStride;
			}

			bufPos = 0;
			for( i = 0; i < EVEN_ALIGNED(headerInfo.height)/2; i++ )
			{
				fwrite( (uint8_t*)hVidMem->crVirAddr + bufPos, EVEN_ALIGNED(headerInfo.width) / 2, 1, outFile );
				bufPos += hVidMem->crStride;
			}
			fclose( outFile );
			CHECK_TIME_STOP( bCheckTime, "-> File Writing" );
			printf("Raw-image Writing Done. ( %d x %d )\n", EVEN_ALIGNED(headerInfo.width), EVEN_ALIGNED(headerInfo.height));
		}

		// VIdeo Layer Display
		NX_DspVideoSetPriority(DISPLAY_MODULE_MLC0, 0);
		{
			DISPLAY_INFO	dspInfo;
			DSP_IMG_RECT	srcRect;
			DSP_IMG_RECT	dstRect;

			CHECK_TIME_START( bCheckTime );
			memset( &dspInfo, 0x00, sizeof(dspInfo) );
			srcRect.left 		= 0;
			srcRect.top 		= 0;
			srcRect.right		= headerInfo.width;
			srcRect.bottom		= headerInfo.height;

			dstRect.left 		= 0;
			dstRect.top 		= 0;
			dstRect.right		= dspWidth;
			dstRect.bottom		= dspHeight;

			dspInfo.port		= 0;
			dspInfo.module		= 0;

			dspInfo.width		= headerInfo.width;
			dspInfo.height		= headerInfo.height;
			dspInfo.dspSrcRect	= srcRect;
			dspInfo.dspDstRect	= dstRect;

			dspInfo.numPlane = 1;
			
			hDsp = NX_DspInit( &dspInfo );
			NX_DspQueueBuffer( hDsp, hVidMem );
			CHECK_TIME_STOP( bCheckTime, "-> Video Layer Drawing" );
		}
	}
	else {
		outSize = NX_JpegRequireBufSize( hJpeg, DECODE_FORMAT_MODE1 );
		outBuf = (uint8_t*)malloc( outSize );

		CHECK_TIME_START( bCheckTime );
		NX_JpegDecode( hJpeg, outBuf, DECODE_FORMAT_MODE1 );
		CHECK_TIME_STOP( bCheckTime, "-> Jpeg Decoding" );

		if( outFile ) {
			CHECK_TIME_START( bCheckTime );
			fwrite( outBuf, outSize, 1, outFile );
			fclose( outFile );
			CHECK_TIME_STOP( bCheckTime, "-> File Writing" );
			printf("Raw-image Writing Done. ( %d x %d )\n", headerInfo.width, headerInfo.height);
		}

		// RGB Layer Display
		NX_DspVideoSetPriority(DISPLAY_MODULE_MLC0, 1);
		{
			CHECK_TIME_START( bCheckTime );
			FillFrameBuffer( headerInfo.width, headerInfo.height, outBuf, 0 );
			CHECK_TIME_STOP( bCheckTime, "-> RGB Layer Drawing" );
		}
	}

	printf("Enter 'q' for exit.\n");
	while(1)
	{
		if( 'q' == getch() ) {
			break;
		}
	}

	FillFrameBuffer( scrWidth, scrHeight, NULL, INITIAL_COLOR );

	if( hDsp ) 		NX_DspClose( hDsp );
	if( hVidMem ) 	NX_FreeVideoMemory( hVidMem );
	
	CHECK_TIME_START( bCheckTime );
	if( hJpeg ) 	NX_JpegClose( hJpeg );
	CHECK_TIME_STOP( bCheckTime, "-> NX_JpegClose()" );

END:
	if( inFileName )	free( inFileName );
	if( outFileName )	free( outFileName );

	return 0;
}
