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

#include <string.h>
#include <CNX_VRFilter.h>

#define	NX_DTAG	"[CNX_VRFilter] "
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
CNX_VRFilter::CNX_VRFilter( void )
	: m_bInit( false )
	, m_bRun( false )
	, m_bEnable( false )
	, m_bEnableHdmi( false )
	, m_hDsp( NULL )
	, m_pPrevVideoSample( NULL )
{
	memset( &m_DisplayInfo, 0x00, sizeof(DISPLAY_INFO) );
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_VRFilter::~CNX_VRFilter( void )
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hLock );
}

//------------------------------------------------------------------------------
void	CNX_VRFilter::Init( NX_VIDRENDER_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit )
	{
		if( pConfig->port == 0) {
			m_DisplayInfo.port		= pConfig->port;
			m_DisplayInfo.module	= DISPLAY_MODULE_MLC0;
		}
		else {
			m_DisplayInfo.port		= pConfig->port;
			m_DisplayInfo.module	= DISPLAY_MODULE_MLC1;
		}
		
		m_DisplayInfo.width		= pConfig->width;
		m_DisplayInfo.height	= pConfig->height;

		m_DisplayInfo.numPlane	= 1;

		m_DisplayInfo.dspSrcRect.left		= m_DisplayInfo.dspDstRect.left		= pConfig->left;
		m_DisplayInfo.dspSrcRect.top		= m_DisplayInfo.dspDstRect.top		= pConfig->top;
		m_DisplayInfo.dspSrcRect.right		= m_DisplayInfo.dspDstRect.right	= pConfig->right;
		m_DisplayInfo.dspSrcRect.bottom		= m_DisplayInfo.dspDstRect.bottom	= pConfig->bottom;

		m_bInit = true;
	}
	
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_VRFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NX_ASSERT( true == m_bInit );

	if( true == m_bInit )
	{
		if( m_bRun )	Stop();
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock( &m_hLock );

	CNX_VideoSample *pVideoSample = (CNX_VideoSample *)pSample;
	NX_ASSERT( NULL != pVideoSample );

	pVideoSample->Lock();
	if( m_bEnable || m_bEnableHdmi ) {
		if( m_hDsp ) NX_DspQueueBuffer( m_hDsp, pVideoSample->GetVideoMemory() );
	}

	Deliver( pSample );

	if( m_pPrevVideoSample ) {
		if( m_hDsp ) NX_DspDequeueBuffer( m_hDsp );
		m_pPrevVideoSample->Unlock();
	}
	
	m_pPrevVideoSample = pVideoSample;

	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::ReleaseSample( CNX_Sample *pSample )
{
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Run( void )
{
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( false == m_bRun ) {
		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Stop( void )
{
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bRun = false;
		
		if( m_hDsp ) {
			NX_DspStreamControl( m_hDsp, false );
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;			
		}

		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::EnableRender( uint32_t enable )
{
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnable)?"Enable":"Disable", (enable)?"Enable":"Disable") );

	if( enable ) {
		if( !m_hDsp ) {
			m_hDsp = NX_DspInit( &m_DisplayInfo );
	 		NX_DspStreamControl( m_hDsp, true );			
		}
	}
	else {
		if( m_hDsp ) {
			NX_DspStreamControl( m_hDsp, false );
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;
		}
		
		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}		
	}
	m_bEnable = enable;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;	
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::EnableHdmiRender( uint32_t enable )
{
	CNX_AutoLock lock( &m_hLock );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnableHdmi)?"Enable":"Disable", (enable)?"Enable":"Disable") );

	DISPLAY_INFO dspInfo;
	memset( &dspInfo, 0x00, sizeof(dspInfo) );

	dspInfo.port 		= 1;
	dspInfo.module		= DISPLAY_MODULE_MLC1;
	dspInfo.width		= m_DisplayInfo.width;
	dspInfo.height		= m_DisplayInfo.height;

	dspInfo.numPlane	= 1;

	dspInfo.dspSrcRect.left		= dspInfo.dspDstRect.left	= 0;
	dspInfo.dspSrcRect.top		= dspInfo.dspDstRect.top	= 0;
	dspInfo.dspSrcRect.right	= dspInfo.dspDstRect.right	= 1920;
	dspInfo.dspSrcRect.bottom	= dspInfo.dspDstRect.bottom	= 1080;

	if( enable ) {
		if( !m_hDsp ) {
			m_hDsp = NX_DspInit( &dspInfo );
	 		NX_DspStreamControl( m_hDsp, true );			
		}
	}
	else {
		if( m_hDsp ) {
			NX_DspStreamControl( m_hDsp, false );
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;
		}

		if( m_pPrevVideoSample ) {
			m_pPrevVideoSample->Unlock();
			m_pPrevVideoSample = NULL;
		}		
	}
	m_bEnableHdmi = enable;

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;	
}
