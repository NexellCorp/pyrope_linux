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

#include "CNX_HlsNotify.h"
#include "CNX_HlsManager.h"

#define NX_DTAG		"[CNX_DvrManager]"
#include <NX_DbgMsg.h>

#ifndef	SAFE_DELETE_FILTER
#define	SAFE_DELETE_FILTER(A)	if(A){delete A;A=NULL;}
#endif
#ifndef	SAFE_START_FILTER
#define	SAFE_START_FILTER(A)    if(A){A->Run();};
#endif
#ifndef	SAFE_STOP_FILTER
#define	SAFE_STOP_FILTER(A)     if(A){A->Stop();};
#endif
#ifndef	SAFE_DEINIT_FILTER
#define	SAFE_DEINIT_FILTER(A)   if(A){A->Deinit();};
#endif

//------------------------------------------------------------------------------
CNX_HlsManager::CNX_HlsManager()
	: m_pRefClock( NULL )
	, m_pNotifier( NULL )
	, m_pVipFilter( NULL )
	, m_pVrFilter( NULL )
	, m_pAvcEncFilter( NULL )
	, m_pAudCapFilter( NULL )
	, m_pAacEncFilter( NULL )
	, m_pTsMuxerFilter( NULL )
	, m_bInit( 0 )
	, m_bRun( 0 )
{
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_HlsManager::~CNX_HlsManager()
{
	if( m_bInit ) Deinit();
	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
int32_t CNX_HlsManager::BuildFilter( void )
{
	m_pRefClock			= new CNX_RefClock();
	m_pNotifier 		= new CNX_HlsNotify();

	m_pVipFilter		= new CNX_VIPFilter();
	m_pVrFilter			= new CNX_VRFilter();
	m_pAvcEncFilter 	= new CNX_H264Encoder();

	m_pAudCapFilter		= new CNX_AudCaptureFilter();
	m_pAacEncFilter		= new CNX_AacEncoder();

	m_pTsMuxerFilter	= new CNX_TsMuxerFilter();
	m_pHlsFilter		= new CNX_HLSFilter();

	m_pVipFilter->Connect( m_pVrFilter );
	m_pVrFilter->Connect( m_pAvcEncFilter );
	m_pAvcEncFilter->Connect( m_pTsMuxerFilter );

	m_pAudCapFilter->Connect( m_pAacEncFilter );
	m_pAvcEncFilter->Connect( m_pTsMuxerFilter );
	m_pTsMuxerFilter->Connect( m_pHlsFilter );

	m_pVipFilter->Init( &m_VipConfig );
	m_pVrFilter->Init( &m_VidRenderConfig );
	m_pAvcEncFilter->Init( &m_VidEncConfig );

	m_pAudCapFilter->Init( &m_AudCapConfig );
	m_pAacEncFilter->Init( &m_AudEncConfig );

	m_pTsMuxerFilter->Init( &m_TsMuxerConfig );
	m_pHlsFilter->Init( &m_HlsConfig );
	
	// etc configuration
	m_pVrFilter->EnableRender( true );
	m_pAvcEncFilter->SetPacketID( 0 );
	m_pAacEncFilter->SetPacketID( 1 );

	return 0;
}

#ifndef SET_EVENT_NOTIFIER
#define SET_EVENT_NOTIFIER(A, B)	if(A){(A)->SetNotifier((INX_EventNotify *)B);};
#endif
void CNX_HlsManager::SetNotifier( void )
{
	if( m_pNotifier )
	{
		SET_EVENT_NOTIFIER( m_pVipFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pVrFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAvcEncFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAudCapFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAacEncFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pTsMuxerFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pHlsFilter,		m_pNotifier );
	}
}

//------------------------------------------------------------------------------
int32_t CNX_HlsManager::SetConfig( NX_HLS_MGR_CONFIG *pConfig )
{
#if(1)
	m_VipConfig.port			= pConfig->nPort;
	m_VipConfig.width			= pConfig->nWidth;
	m_VipConfig.height			= pConfig->nHeight;
	m_VipConfig.fps				= pConfig->nFps;

	m_VidRenderConfig.port 		= 0;
	m_VidRenderConfig.width		= pConfig->nWidth;
	m_VidRenderConfig.height 	= pConfig->nHeight;
	m_VidRenderConfig.top		= 0;
	m_VidRenderConfig.left		= 0;
	m_VidRenderConfig.right		= pConfig->nWidth;
	m_VidRenderConfig.bottom	= pConfig->nHeight;

	m_VidEncConfig.width		= pConfig->nWidth;
	m_VidEncConfig.height		= pConfig->nHeight;
	m_VidEncConfig.fps			= pConfig->nFps;
	m_VidEncConfig.bitrate		= 3000000;
	m_VidEncConfig.codec		= 0x21;

	m_AudCapConfig.channels		= 2;
	m_AudCapConfig.frequency	= 48000;
	m_AudCapConfig.samples		= 1152;

	m_AudEncConfig.channels		= 2;
	m_AudEncConfig.frequency	= 48000;
	m_AudEncConfig.bitrate		= 128000;
	m_AudEncConfig.codec		= 0x40;

	m_TsMuxerConfig.videoTrack	= 1;
	m_TsMuxerConfig.audioTrack	= 1;
	m_TsMuxerConfig.textTrack	= 0;
	m_TsMuxerConfig.codecType[0]= STREAM_TYPE_H264;

	strcpy( (char*)m_HlsConfig.MetaFileName,(char*)pConfig->pMetaFileName );
	strcpy( (char*)m_HlsConfig.SegmentName, (char*)pConfig->pSegmentFileName );
	strcpy( (char*)m_HlsConfig.SegmentRoot, (char*)pConfig->pSegmentRoot );

	m_HlsConfig.SegmentDuration = pConfig->nSegmentDuration;
	m_HlsConfig.SegmentNumber	= pConfig->nSegmentNumber;
#else
	m_VipConfig.port			= 2;
	m_VipConfig.width			= 1024;
	m_VipConfig.height			= 768;
	m_VipConfig.fps				= 30;

	m_VidRenderConfig.port 		= 0;
	m_VidRenderConfig.width		= 1024;
	m_VidRenderConfig.height 	= 768;
	m_VidRenderConfig.top		= 0;
	m_VidRenderConfig.left		= 0;
	m_VidRenderConfig.right		= 1024;
	m_VidRenderConfig.bottom	= 768;

	m_VidEncConfig.width		= 1024;
	m_VidEncConfig.height		= 768;
	m_VidEncConfig.fps			= 30;
	m_VidEncConfig.bitrate		= 1000000;
	m_VidEncConfig.codec		= 0x21;

	m_AudCapConfig.channels		= 2;
	m_AudCapConfig.frequency	= 48000;
	m_AudCapConfig.samples		= 1152;

	m_AudEncConfig.channels		= 2;
	m_AudEncConfig.frequency	= 48000;
	m_AudEncConfig.bitrate		= 128000;
	m_AudEncConfig.codec		= 0x40;

	m_TsMuxerConfig.videoTrack		= 1;
	m_TsMuxerConfig.audioTrack		= 0;
	m_TsMuxerConfig.textTrack		= 0;
	m_TsMuxerConfig.codecType[0]	= STREAM_TYPE_H264;

	strcpy((char*)m_HlsConfig.MetaFileName, "test.m3u8");
	strcpy((char*)m_HlsConfig.SegmentName, "segment");
	strcpy((char*)m_HlsConfig.SegmentRoot, "/www");

	m_HlsConfig.SegmentDuration = 10;
	m_HlsConfig.SegmentNumber = 3;
#endif
	
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_HlsManager::Init( NX_HLS_MGR_CONFIG *pConfig )
{
	CNX_AutoLock lock( &m_hLock );
	int32_t ret = 0;

	SetConfig( pConfig );

	if( !(ret = BuildFilter()) )
	{
		SetNotifier();
		m_bInit = true;
	}

	return ret;
}

//------------------------------------------------------------------------------
int32_t CNX_HlsManager::Deinit( void )
{
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_DEINIT_FILTER( m_pVipFilter );
		SAFE_DEINIT_FILTER( m_pVrFilter );
		SAFE_DEINIT_FILTER( m_pAvcEncFilter );
		SAFE_DEINIT_FILTER( m_pAudCapFilter );
		SAFE_DEINIT_FILTER( m_pAacEncFilter );
		SAFE_DEINIT_FILTER( m_pTsMuxerFilter );
		SAFE_DEINIT_FILTER( m_pHlsFilter );

		SAFE_DELETE_FILTER( m_pVipFilter );
		SAFE_DELETE_FILTER( m_pVrFilter );
		SAFE_DELETE_FILTER( m_pAvcEncFilter );
		SAFE_DELETE_FILTER( m_pAudCapFilter );
		SAFE_DELETE_FILTER( m_pAacEncFilter );
		SAFE_DELETE_FILTER( m_pTsMuxerFilter );
		SAFE_DELETE_FILTER( m_pHlsFilter );
	}

	return 0;	
}

int32_t CNX_HlsManager::Start( void )
{
	CNX_AutoLock lock( &m_hLock );

	if( m_bInit )
	{
		SAFE_START_FILTER( m_pVipFilter );
		SAFE_START_FILTER( m_pVrFilter );
		SAFE_START_FILTER( m_pAvcEncFilter );
		SAFE_START_FILTER( m_pAudCapFilter );
		SAFE_START_FILTER( m_pAacEncFilter );
		SAFE_START_FILTER( m_pTsMuxerFilter );
		SAFE_START_FILTER( m_pHlsFilter );

		m_bRun = true;
	}

	return 0;
}

int32_t CNX_HlsManager::Stop( void )
{
	
	if( m_bRun )
	{
		SAFE_STOP_FILTER( m_pVipFilter );
		SAFE_STOP_FILTER( m_pVrFilter );
		SAFE_STOP_FILTER( m_pAvcEncFilter );
		SAFE_STOP_FILTER( m_pAudCapFilter );
		SAFE_STOP_FILTER( m_pAacEncFilter );
		SAFE_STOP_FILTER( m_pTsMuxerFilter );
		SAFE_STOP_FILTER( m_pHlsFilter );

		m_bRun = false;
	}

	return 0;
}

int32_t CNX_HlsManager::RegisterNotifyCallback(uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t)
)
{
	CNX_AutoLock lock( &m_hLock );
	
	if( cbNotify )
	{
		m_pNotifier->RegisterNotifyCallback( cbNotify );
	}

	return 0;
}
