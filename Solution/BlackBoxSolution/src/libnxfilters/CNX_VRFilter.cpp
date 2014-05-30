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
	, m_hDsp( NULL )
	, m_pPrevVideoSample( NULL )
{
	memset( &m_DisplayInfo, 0x00, sizeof(DISPLAY_INFO) );
	pthread_mutex_init( &m_hEnableLock, NULL );
}

//------------------------------------------------------------------------------
CNX_VRFilter::~CNX_VRFilter( void )
{
	if( true == m_bInit )
		Deinit();

	pthread_mutex_destroy( &m_hEnableLock );
}

//------------------------------------------------------------------------------
void	CNX_VRFilter::Init( NX_VIDRENDER_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	
	NX_ASSERT( false == m_bInit );
	NX_ASSERT( NULL != pConfig );

	if( false == m_bInit )
	{
		m_DisplayInfo.port		= pConfig->port;
		m_DisplayInfo.module	= DISPLAY_MODULE_MLC0;
		m_DisplayInfo.width		= pConfig->width;
		m_DisplayInfo.height	= pConfig->height;

		m_DisplayInfo.numPlane	= 1;

		m_DisplayInfo.dspSrcRect.left		= pConfig->left;
		m_DisplayInfo.dspSrcRect.top		= pConfig->top;
		m_DisplayInfo.dspSrcRect.right		= pConfig->right;	// pConfig->right - pConfig->left;
		m_DisplayInfo.dspSrcRect.bottom		= pConfig->bottom;	// pConfig->bottom - pConfig->top;

		m_DisplayInfo.dspDstRect.left		= pConfig->left;
		m_DisplayInfo.dspDstRect.top		= pConfig->top;
		m_DisplayInfo.dspDstRect.right		= pConfig->right;	// pConfig->right - pConfig->left;
		m_DisplayInfo.dspDstRect.bottom		= pConfig->bottom;	// pConfig->bottom - pConfig->top;

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

		if( m_hDsp ) {
			NX_DspStreamControl( m_hDsp, false );
			NX_DspClose( m_hDsp );
			m_hDsp = NULL;			
		}
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_VRFilter::Receive( CNX_Sample *pSample )
{
	CNX_VideoSample *pVideoSample = (CNX_VideoSample *)pSample;
	NX_ASSERT( NULL != pVideoSample );

	pthread_mutex_lock( &m_hEnableLock );
	pVideoSample->Lock();
	if( m_bEnable ) {
		if( m_hDsp ){
			NX_DspQueueBuffer( m_hDsp, pVideoSample->GetVideoMemory() );
			NX_DspDequeueBuffer( m_hDsp );
		}
	}
	pthread_mutex_unlock( &m_hEnableLock );

	Deliver( pSample );

	if( m_pPrevVideoSample )
		m_pPrevVideoSample->Unlock();
	
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
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bRun = false;
		
		if( m_pPrevVideoSample )
			m_pPrevVideoSample->Unlock();
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_VRFilter::EnableRender( uint32_t enable )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnable)?"Enable":"Disable", (enable)?"Enable":"Disable") );

	pthread_mutex_lock( &m_hEnableLock );
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
	}
	m_bEnable = enable;
	pthread_mutex_unlock( &m_hEnableLock );

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;	
}
