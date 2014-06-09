//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
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
#include <nx_alloc_mem.h>

#ifndef __NX_DVRCONTROL_H__
#define __NX_DVRCONTROL_H__

typedef struct CNX_DvrManager*	NX_DVR_HANDLE;

typedef enum {
	DVR_CAMERA_VIP0,
	DVR_CAMERA_VIP1,
	DVR_CAMERA_MIPI,
} NX_DVR_CAMERA_TYPE;

typedef enum {
	DVR_ENCODE_NORMAL,
	DVR_ENCODE_MOTION,
} NX_DVR_ENCODE_TYPE;

typedef enum {
	DVR_CODEC_H264,
	DVR_CODEC_MPEG4,	// Not Support
	DVR_CODEC_AAC,
	DVR_CODEC_MP3,		// Not Support
	DVR_CODEC_PCM,		// Not Support
} NX_DVR_CODEC_FORMAT;

typedef enum {
	DVR_CONTAINER_MP4,
	DVR_CONTAINER_TS,
} NX_DVR_CONTAINER_TYPE;

typedef enum {
	DVR_NOTIFY_NORALWIRTHING_DONE	= 0x1001,
	DVR_NOTIFY_EVENTWRITING_DONE	= 0x1002,
	DVR_NOTIFY_JPEGWRITING_DONE		= 0x1003,
	DVR_NOTIFY_MOTION				= 0x1004,
	DVR_NOTIFY_ERR_VIDEO_INPUT		= 0xF001,
	DVR_NOTIFY_ERR_VIDEO_ENCODE		= 0xF002,
	DVR_NOTIFY_ERR_OPEN_FAIL		= 0xF003,
	DVR_NOTIFY_ERR_WRITE			= 0xF004,
} NX_DVR_NOTIFY_TYPE;

typedef struct tag_NX_DVR_VIDEO_CONFIG {
	uint32_t				nPort;		// Camera Capture Port
	uint32_t				nSrcWidth;	// Source Height
	uint32_t				nSrcHeight;	// Source Width
	uint32_t				nDstWidth;	// Destination Width
	uint32_t				nDstHeight;	// Destination Height
	uint32_t				nFps;		// Video Frame rate
	uint32_t				nBitrate;	// Video Bitrate
	NX_DVR_CODEC_FORMAT		nCodec;		// Video Codec : H264 / MPEG4
	uint32_t				bExternProc;
} NX_DVR_VIDEO_CONFIG;

typedef struct tag_NX_DVR_AUDIO_CONFIG {
	uint32_t				nChannel;	// Audio Channel : Mono(1), Stereo(2)
	uint32_t				nFrequency;	// Audio Sampling frequency
	uint32_t				nBitrate;	// Audio bitrate
	NX_DVR_CODEC_FORMAT		nCodec;		// Audio codec : AAC / MP3 / PCM
} NX_DVR_AUDIO_CONFIG;

typedef struct tag_NX_DVR_TEXT_CONFIG {
	uint32_t				nInterval;	// User Data Writing Interval
	uint32_t				nBitrate;	// User Data bitrate ( Not Support )
} NX_DVR_TEXT_CONFIG;

typedef struct tag_NX_DVR_HLS_CONFIG {
	uint32_t				nSegmentDuration;		// Segment duration
	uint32_t				nSegmentNumber;			// Segment created number
	uint8_t					MetaFileName[256];		// hls meta file name (m3u8)
	uint8_t					SegmentFileName[256];	// Segment file name
	uint8_t					SegmentRootDir[256];	// Segment root directory
} NX_DVR_HLS_CONFIG;

typedef struct tag_NX_DVR_MD_CONFIG {
	uint32_t				nMdThreshold;			// Internal motion detect threshold
	uint32_t				nMdSensitivity;			// Internal motion detect sensitivity
	uint32_t				nMdSampingFrame;		// Internal motion detect sampling frame
} NX_DVR_MD_CONFIG;

typedef struct tag_NX_DVR_MEDIA_CONFIG {
	NX_DVR_VIDEO_CONFIG		videoConfig[4];		// Video Configuration (MAX 4EA)
	NX_DVR_AUDIO_CONFIG		audioConfig;		// Audio Configuration (1EA)
	NX_DVR_TEXT_CONFIG		textConfig;			// UserData Configuation (1EA)
	uint32_t				nVideoChannel;		// Video Channel
	uint32_t				bEnableAudio;		// Audio Enable
	uint32_t				bEnableUserData;	// UserData Enable
	NX_DVR_CONTAINER_TYPE	nContainer;			// Meidia Cotainer Type
} NX_DVR_MEDIA_CONFIG;

typedef struct tag_NX_DVR_RECORD_CONFIG {
	// Blackbox Basic Configuration (file duration :: ms)
	uint32_t				nNormalDuration;		// normal file duration
	uint32_t				nEventDuration;			// event file duration ( after event action )
	uint32_t				nEventBufferDuration;	// event file duration ( before event action ) :: total event duration = nEventBufferDuration + nEventDuration
	// Blackbox Optional Configuration (HLS Service)
	uint32_t				bHlsEnable;				// hls enable
	NX_DVR_HLS_CONFIG		hlsConfig;				// hls config
	// Blackbox Optional Configuration (Motion Detection)
	uint32_t				bMdEnable[4];			// motion detect enable
	NX_DVR_MD_CONFIG 		mdConfig[4];			// motion detect internal config
} NX_DVR_RECORD_CONFIG;

typedef struct tag_NX_DVR_DISPLAY_CONFIG {
	uint32_t				bEnable;			// Display Enable
	uint32_t				nChannel;			// Preview Channel ( depend on Video Channel number )
	uint32_t				nModule;			// Display Module ( LCD / MIPI )
	uint32_t				nX;					// Display StartX
	uint32_t				nY;					// Display StartY
	uint32_t				nWidth;				// Display Width
	uint32_t				nHeight;			// Display Height
} NX_DVR_DISPLAY_CONFIG;

#ifdef __cplusplus
extern "C"{
#endif

// DVR Control Interface Function.
NX_DVR_HANDLE	NX_DvrInit					( NX_DVR_MEDIA_CONFIG *pMediaConfig, NX_DVR_RECORD_CONFIG *pRecordConfig, NX_DVR_DISPLAY_CONFIG *pDisplayConfig );
void			NX_DvrDeinit				( NX_DVR_HANDLE hDvr);

int32_t			NX_DvrStart					( NX_DVR_HANDLE hDvr, NX_DVR_ENCODE_TYPE nEncodeType );
int32_t			NX_DvrStop					( NX_DVR_HANDLE hDvr );
int32_t			NX_DvrEvent					( NX_DVR_HANDLE hDvr );
int32_t 		NX_DvrChangeMode 			( NX_DVR_HANDLE hDvr, NX_DVR_ENCODE_TYPE nEncodeType );
int32_t			NX_DvrCapture				( NX_DVR_HANDLE hDvr, int32_t channel );
int32_t 		NX_DvrSetPreview			( NX_DVR_HANDLE hDvr, int32_t preview );
int32_t			NX_DvrSetPreviewHdmi		( NX_DVR_HANDLE hDvr, int32_t preview );

// For Debugging Tools
int32_t			NX_DvrChgDebugLevel			( NX_DVR_HANDLE hDvr, uint32_t dbgLevel );
int32_t 		NX_DvrGetAPIVersion			( NX_DVR_HANDLE hDvr, int32_t *pMajor, int32_t *pMinor, int32_t *pRevision );

// Not Implementation
int32_t			NX_DvrSetDisplay			( NX_DVR_HANDLE hDvr, NX_DVR_DISPLAY_CONFIG *pDisplayConfig );
int32_t			NX_DvrVideoLayerEnable		( NX_DVR_HANDLE hDvr, uint32_t nEnable );

int32_t			NX_DvrGetVideoLayerPriority	( NX_DVR_HANDLE hDvr, uint32_t *pPriority );
int32_t			NX_DvrSetVideoLayerPriority	( NX_DVR_HANDLE hDvr, uint32_t nPriority );

int32_t			NX_DvrGetStatistics			( NX_DVR_HANDLE hDvr, NX_DVR_MEDIA_CONFIG *pMediaConfig );

// Registration DVR Callback Function
int32_t			NX_DvrRegisterFileNameCallback ( NX_DVR_HANDLE hDvr,
					int32_t (*cbNormalFileName)		( uint8_t*, uint32_t ), 
					int32_t (*cbEventFileName)		( uint8_t*, uint32_t ),
					int32_t (*cbParkingFileName)	( uint8_t*, uint32_t ),		// Not Support
					int32_t (*cbJpegFileName)		( uint8_t*, uint32_t )
				);

int32_t			NX_DvrRegisterUserDataCallback ( NX_DVR_HANDLE hDvr, 
					int32_t (*cbUserData)			( uint8_t*, uint32_t ) 
				);

int32_t			NX_DvrRegisterTextOverlayCallback ( NX_DVR_HANDLE hDvr,
					int32_t (*cbFrontTextOverlay)	( uint8_t*, uint32_t*, uint32_t*, uint32_t* ),
					int32_t (*cbRearTextOverlay)	( uint8_t*, uint32_t*, uint32_t*, uint32_t* )
				);

int32_t			NX_DvrRegisterNotifyCallback ( NX_DVR_HANDLE hDvr,
					int32_t (*cbNotify)				( uint32_t, uint8_t*, uint32_t )
				);

int32_t 		NX_DvrRegisterImageEffectCallback( NX_DVR_HANDLE hDvr, 
					int32_t (*cbFrontImageEffect)	( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO * ),
					int32_t (*cbRearImageEffect)	( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO * )
				);

int32_t			NX_DvrRegisterMotionDetectCallback( NX_DVR_HANDLE hDvr,
					int32_t (*cbFrontMotionDetect)	( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO * ),
					int32_t (*cbRearMotionDetect)	( NX_VID_MEMORY_INFO *, NX_VID_MEMORY_INFO * )
				);

#ifdef __cplusplus
};
#endif

#endif	// __NX_DVRCONTROL_H__
