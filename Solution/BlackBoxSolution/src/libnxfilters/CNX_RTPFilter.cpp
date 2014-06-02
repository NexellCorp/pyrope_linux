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

#include "CNX_RTPFilter.h"

#define	NX_DTAG	"[CNX_RTPFilter] "
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_RTPFilter::CNX_RTPFilter()
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
{
	m_pSemIn	= new CNX_Semaphore( MAX_BUFFER, 0 );
	NX_ASSERT( m_pSemIn );
	m_SampleInQueue.Reset();
}

//------------------------------------------------------------------------------
CNX_RTPFilter::~CNX_RTPFilter()
{
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::Init( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( false == m_bInit ) {
		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bInit ) {
		if( m_bRun ) {
			Stop();
		}
		m_bInit = false;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Receive( CNX_Sample *pSample )
{
	NX_ASSERT( NULL != pSample );

	pSample->Lock();
	Deliver( pSample );
	m_SampleInQueue.PushSample( pSample );
	m_pSemIn->Post();

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::ReleaseSample( CNX_Sample *pSample )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bThreadExit 	= false;
		NX_ASSERT( !m_hThread );

		if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Create Thread\n"), __func__) );
			return false;
		}

		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_RTPFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bThreadExit = true;
		m_pSemIn->Post();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
		m_bRun = false;

	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::AllocateMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::FreeMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	m_pSemIn->Post();		//	Send Dummy
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::GetSample( CNX_Sample **ppSample)
{
	m_pSemIn->Pend();
	if( true == m_SampleInQueue.IsReady() ) {
		m_SampleInQueue.PopSample( ppSample );
		return true;
	}
	return false;

}

//------------------------------------------------------------------------------
int32_t CNX_RTPFilter::GetDeliverySample( CNX_Sample **ppSample )
{
	return false;
}

//------------------------------------------------------------------------------
void CNX_RTPFilter::ThreadLoop(void)
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	CNX_MuxerSample *pSample = NULL;

	while( !m_bThreadExit )
	{
		if( false == GetSample((CNX_Sample **)&pSample) )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("GetSample() Failed\n")) );
			continue;
		}
		if( NULL == pSample )
		{
			NxDbgMsg( NX_DBG_WARN, (TEXT("Sample is NULL\n")) );
			continue;
		}

		// Transfer Stream Data. (RTP)

		if( pSample )
			pSample->Unlock();
	}

	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void* CNX_RTPFilter::ThreadMain(void*arg)
{
	CNX_RTPFilter *pClass = (CNX_RTPFilter *)arg;
	
	pClass->ThreadLoop();

	return (void*)0xDEADDEAD;
}
