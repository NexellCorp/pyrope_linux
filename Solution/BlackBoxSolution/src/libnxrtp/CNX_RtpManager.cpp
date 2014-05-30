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

#include "CNX_RtpNotify.h"
#include "CNX_RtpManager.h"

#define NX_DTAG		"[CNX_RtpManager] "
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
CNX_RtpManager::CNX_RtpManager()
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
CNX_RtpManager::~CNX_RtpManager()
{
	if( m_bInit ) Deinit();
	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::BuildFilter( void )
{
	m_pRefClock			= new CNX_RefClock();
	m_pNotifier 		= new CNX_RtpNotify();

	m_pVipFilter		= new CNX_VIPFilter();
	m_pVrFilter			= new CNX_VRFilter();
	m_pAvcEncFilter 	= new CNX_H264Encoder();

	m_pAudCapFilter		= new CNX_AudCaptureFilter();
	m_pAacEncFilter		= new CNX_AacEncoder();

	m_pTsMuxerFilter	= new CNX_TsMuxerFilter();
	m_pRtpFilter		= new CNX_RTPFilter();

	m_pVipFilter->Connect( m_pVrFilter );
	m_pVrFilter->Connect( m_pAvcEncFilter );
	m_pAvcEncFilter->Connect( m_pTsMuxerFilter );

	m_pAudCapFilter->Connect( m_pAacEncFilter );
	m_pAvcEncFilter->Connect( m_pTsMuxerFilter );
	m_pTsMuxerFilter->Connect( m_pRtpFilter );

	m_pVipFilter->Init( &m_VipConfig );
	m_pVrFilter->Init( &m_VidRenderConfig );
	m_pAvcEncFilter->Init( &m_VidEncConfig );

	m_pAudCapFilter->Init( &m_AudCapConfig );
	m_pAacEncFilter->Init( &m_AudEncConfig );

	m_pTsMuxerFilter->Init( &m_TsMuxerConfig );
	//m_pRtpFilter->Init( &m_RtpConfig );
	
	// etc configuration
	m_pVrFilter->EnableRender( true );
	m_pAvcEncFilter->SetPacketID( 0 );
	m_pAacEncFilter->SetPacketID( 1 );

	return 0;
}

#ifndef SET_EVENT_NOTIFIER
#define SET_EVENT_NOTIFIER(A, B)	if(A){(A)->SetNotifier((INX_EventNotify *)B);};
#endif
void CNX_RtpManager::SetNotifier( void )
{
	if( m_pNotifier )
	{
		SET_EVENT_NOTIFIER( m_pVipFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pVrFilter,		m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAvcEncFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAudCapFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pAacEncFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pTsMuxerFilter,	m_pNotifier );
		SET_EVENT_NOTIFIER( m_pRtpFilter,		m_pNotifier );
	}
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::SetConfig( NX_RTP_MGR_CONFIG *pConfig )
{
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


	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_RtpManager::Init( NX_RTP_MGR_CONFIG *pConfig )
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
int32_t CNX_RtpManager::Deinit( void )
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
		SAFE_DEINIT_FILTER( m_pRtpFilter );

		SAFE_DELETE_FILTER( m_pVipFilter );
		SAFE_DELETE_FILTER( m_pVrFilter );
		SAFE_DELETE_FILTER( m_pAvcEncFilter );
		SAFE_DELETE_FILTER( m_pAudCapFilter );
		SAFE_DELETE_FILTER( m_pAacEncFilter );
		SAFE_DELETE_FILTER( m_pTsMuxerFilter );
		SAFE_DELETE_FILTER( m_pRtpFilter );
	}

	return 0;	
}

int32_t CNX_RtpManager::Start( void )
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
		SAFE_START_FILTER( m_pRtpFilter );

		m_bRun = true;
	}

	return 0;
}

int32_t CNX_RtpManager::Stop( void )
{
	
	if( m_bRun )
	{
		SAFE_STOP_FILTER( m_pVipFilter );
		SAFE_STOP_FILTER( m_pVrFilter );
		SAFE_STOP_FILTER( m_pAvcEncFilter );
		SAFE_STOP_FILTER( m_pAudCapFilter );
		SAFE_STOP_FILTER( m_pAacEncFilter );
		SAFE_STOP_FILTER( m_pTsMuxerFilter );
		SAFE_STOP_FILTER( m_pRtpFilter );

		m_bRun = false;
	}

	return 0;
}

int32_t CNX_RtpManager::RegisterNotifyCallback(uint32_t (*cbNotify)(uint32_t, uint8_t*, uint32_t)
)
{
	CNX_AutoLock lock( &m_hLock );
	
	if( cbNotify )
	{
		m_pNotifier->RegisterNotifyCallback( cbNotify );
	}

	return 0;
}
