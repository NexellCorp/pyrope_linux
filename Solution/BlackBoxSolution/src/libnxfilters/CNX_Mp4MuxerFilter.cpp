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
#include <unistd.h>
#include <fcntl.h>

#include <CNX_RefClock.h>
#include <NX_FilterSysApi.h>
#include <CNX_Mp4MuxerFilter.h>

#define	NX_DTAG	"[CNX_Mp4MuxerFilter] "
#include <NX_DbgMsg.h>

#define MAKE4CC(ch4) (	( ( (uint32_t)(ch4) & 0xFF      ) << 24) | \
						( ( (uint32_t)(ch4) & 0xFF00    ) <<  8) | \
                        ( ( (uint32_t)(ch4) & 0xFF0000  ) >>  8) | \
                        ( ( (uint32_t)(ch4) & 0xFF000000) >> 24)  )

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
	((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
	((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
#endif 

//------------------------------------------------------------------------------
CNX_Mp4MuxerFilter::CNX_Mp4MuxerFilter( void )
	: m_bInit( false )
	, m_bRun( false )
	, m_bThreadExit( true )
	, m_hThread( 0x00 )
	, m_hMp4Mux( 0x00 )
	, m_bEnableMux( false )
	, m_bStartMuxing( 0 )
	, m_MuxStartTime( 0 )
	, m_OutFd( 0 )
	, m_nWritingMode( WRITING_MODE_NORMAL )
{
	for( int32_t i = 0; i < NUM_STRM_BUFFER; i++ )
		m_pStreamBuffer[i] = NULL;

	pthread_mutex_init( &m_hWriteLock, NULL );

	m_pSemStream = new CNX_Semaphore( NUM_STRM_BUFFER, 0 );
	NX_ASSERT( m_pSemStream );

	m_pSemFile = new CNX_Semaphore( NUM_STRM_BUFFER, 0 );
	NX_ASSERT( m_pSemFile );

	AllocateMemory();
}

//------------------------------------------------------------------------------
CNX_Mp4MuxerFilter::~CNX_Mp4MuxerFilter( void )
{
	uint32_t i;

	if( true == m_bInit )
		Deinit();

	if( m_pSemStream )
		delete m_pSemStream;

	if( m_pSemFile )
		delete m_pSemFile;

	for(i = 0; i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack; i++ )
		delete []m_pTrackTempBuf[i];

	FreeMemory();

	pthread_mutex_destroy( &m_hWriteLock );
}

//------------------------------------------------------------------------------
void	CNX_Mp4MuxerFilter::Init( NX_MP4MUXER_CONFIG *pConfig )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	uint32_t i;

	if( false == m_bInit )
	{
		memcpy( &m_MP4Config, pConfig, sizeof(m_MP4Config) );
		
		for( i = 0; i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack; i++ )
		{
			if( i < m_MP4Config.videoTrack ) {
				m_pTrackTempBuf[i] = new uint8_t[1024 * 1024 * 2];
			}
			else if( i < m_MP4Config.videoTrack + m_MP4Config.audioTrack ) {
				m_pTrackTempBuf[i] = new uint8_t[64 * 1024];
			}
			else if( i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack) {
				m_pTrackTempBuf[i] = new uint8_t[MAX_USERDATA_SIZE];
			}

			NX_ASSERT( m_pTrackTempBuf[i] );
		}

		m_bInit = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_Mp4MuxerFilter::Deinit( void )
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
int32_t	CNX_Mp4MuxerFilter::Receive( CNX_Sample *pSample )
{
	CNX_AutoLock lock (&m_hWriteLock );
	pSample->Lock();

	//static long long _prevtime = 0;
	//if( ((long long)((CNX_MuxerSample*)pSample)->GetTimeStamp() - _prevtime) < 0 && _prevtime != 0) {
	//	printf("[Warnning] Timestamp is not correct.\n");
	//	pSample->Unlock();
	//	return false;
	//}
	//_prevtime = ((CNX_MuxerSample*)pSample)->GetTimeStamp();	

	if( m_bEnableMux )
	{
		return MuxEncodedSample((CNX_MuxerSample *)pSample);
	}
	pSample->Unlock();
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::ReleaseSample( CNX_Sample *pSample )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp4MuxerFilter::Run( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( m_bRun == false ) {
		m_bRun = true;
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp4MuxerFilter::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	if( true == m_bRun ) {
		m_bRun = false;
		if( m_bEnableMux ){
			StopMuxing();
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
	return true;
}

//------------------------------------------------------------------------------
void CNX_Mp4MuxerFilter::AllocateMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	for( int32_t i = 0; i < NUM_STRM_BUFFER; i++ )
	{
		m_pStreamBuffer[i] = new uint8_t[SIZE_STRM_BUFFER];
		NX_ASSERT( m_pStreamBuffer[i] );
		//NxDbgMsg( NX_DBG_DEBUG, (TEXT("Alloc Memory = %d, %p\n"), i, m_pStreamBuffer[i]) );
		if( m_pStreamBuffer[i] == NULL ) {
			NxDbgMsg( NX_DBG_ERR, (TEXT("%s(): Fail, Allocate Stream Memory\n"), __func__) );
			return;
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void CNX_Mp4MuxerFilter::FreeMemory( void )
{
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()++\n"), __func__) );
	int i;

	for( i=0; i<NUM_STRM_BUFFER ; i++ )
	{
		if( m_pStreamBuffer[i] ){
			//NxDbgMsg( NX_DBG_DEBUG, (TEXT("Free Memory = %d, %p\n"), i, m_pStreamBuffer[i]) );

			delete [] m_pStreamBuffer[i];
			m_pStreamBuffer[i] = NULL;
		}
	}
	NxDbgMsg( NX_DBG_VBS, (TEXT("%s()--\n"), __func__) );
}

//------------------------------------------------------------------------------
void	CNX_Mp4MuxerFilter::ThreadLoop(void)
{
	uint8_t* buf;
	int32_t size;

	NxDbgMsg( NX_DBG_VBS, ("%s IN\n", __func__) );
	//	File Writing
	while( !m_bThreadExit )
	{
		if( m_pSemFile->Pend() )
		{
			break;
		}
		while( m_FileQueue.GetSampleCount() > 0 )
		{
			m_FileQueue.Pop( (void**)&buf, &size );

			if( m_OutFd ) {
				int32_t ret = write( m_OutFd, buf, size );

				if( 0 > ret ) {
					if( m_pNotify )
						m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
				}				
			}
			//{
			//	static int writeCnt = 0;
			//	NxDbgMsg( DBG_VBS, ("Write Buffer : Address:%p Write Count = %d\n", buf, ++writeCnt) );
			//}
			m_StreamQueue.Push( buf, SIZE_STRM_BUFFER );
			m_pSemStream->Post();
		}
	}
	NxDbgMsg( NX_DBG_VBS, ("%s OUT\n", __func__) );
}

//------------------------------------------------------------------------------
void*	CNX_Mp4MuxerFilter::ThreadMain(void*arg)
{
	CNX_Mp4MuxerFilter *pClass = (CNX_Mp4MuxerFilter *)arg;
	pClass->ThreadLoop();
	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_Mp4MuxerFilter::FileWriter( void *pObj, unsigned char *pBuffer, int bufSize )
{
	CNX_Mp4MuxerFilter *pMux = (CNX_Mp4MuxerFilter *)pObj;
	if( pMux ){
		if( bufSize > 0 ){
			pMux->m_FileQueue.Push( pBuffer, bufSize );
			pMux->m_pSemFile->Post();
		}
	}
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::GetBuffer( void *pObj, unsigned char **pBuffer, int *bufSize )
{
	CNX_Mp4MuxerFilter *pMux = (CNX_Mp4MuxerFilter *)pObj;
	if( pMux )
	{
		pMux->m_pSemStream->Pend();
		pMux->m_StreamQueue.Pop( (void**)pBuffer, bufSize );
		//static int getBuffCnt = 0;
		//NxDbgMsg( DBG_VBS, ("GetBuffer : Address=%p, getBuffCnt = %d\n", *pBuffer,  ++getBuffCnt));
		return 0;
	}
	return -1;
}

//------------------------------------------------------------------------------
static int32_t FindMpeg4DSIinKeyFrame( uint8_t *inBuf, int32_t len, int32_t *outSize )
{
	int32_t i;
	*outSize = 0;
	for( i = 0 ; i < (len - 4) ; i++ )
	{
		//	Find VOP Start Code
		if( inBuf[0]==0x00 && inBuf[1]==0x00 && inBuf[2]==0x01 && inBuf[3]==0xb6 ) {
			*outSize = i;
			return true;
		}
		inBuf++;
	}
	return false;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::MuxEncodedSample( CNX_MuxerSample *pSample )
{
	NX_ASSERT( NULL != pSample );

	uint8_t *pBuffer;
	int32_t bufSize;
	int64_t duration = 0;
	uint64_t PTS = pSample->GetTimeStamp();
	uint32_t trackID = pSample->GetDataType();
	uint32_t i;

	if( !m_bRun )
		return false;

	if( !m_bStartMuxing ) {
		m_MuxStartTime = PTS;
		for( i = 0; i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack; i++)
		{
			m_bTrackStart[i] = false;
			m_TrackInfo[i].totalDuration = 0;
		}
		m_bStartMuxing = true;
	}

	//	Delete Previeous Samples
	if( m_MuxStartTime > PTS ){
		printf("Drop Sample (%12lld, %12lld)\n", m_MuxStartTime, PTS );
		pSample->Unlock();
		return false;
		PTS = m_TrackInfo[trackID].totalDuration;
	}

	PTS -= m_MuxStartTime;

	if( pSample->GetBuffer( &pBuffer, &bufSize) ) {
		bufSize = pSample->GetActualDataLength();

		if( m_bTrackStart[trackID] ) {
			//if( trackID == 0)
				NxDbgMsg( NX_DBG_VBS, ("[TRACK][%02d] : PTS = %lld, totlaDuration = %lld\n", trackID, PTS, m_TrackInfo[trackID].totalDuration) );
			duration = PTS - m_TrackInfo[trackID].totalDuration;
			NxMP4MuxPutData( 
				m_hMp4Mux, 
				m_TrackInfo[trackID].pData, 
				m_TrackInfo[trackID].size, 
				duration, 
				trackID, 
				m_TrackInfo[trackID].flag 
			);
		}
		else {
			if( trackID < m_MP4Config.videoTrack ) { // if video track
				// 최초 frame이 key frame일때까지 버려야 함.
				if( pSample->GetSyncPoint() )
				{
					if( CODEC_MPEG4 == m_MP4Config.trackConfig[trackID].codecType ) {
						int32_t dsiSize = 0;
						if( FindMpeg4DSIinKeyFrame( pBuffer, bufSize, &dsiSize ) == true ){
							NxMp4MuxSetDsiInfo( m_hMp4Mux, pBuffer, dsiSize, trackID );
						}
					}
					else {
						//printf("Video Dsi Info (trackID = %d, size = %d) : ", trackID, MAX_VID_DSI_SIZE);
						//for( i = 0; i < MAX_VID_DSI_SIZE; i++ )
						//{
						//	printf("0x%02x ", m_TrackDsiInfo[trackID][i]);
						//}
						//printf("\n");
						NxMp4MuxSetDsiInfo( m_hMp4Mux, m_TrackDsiInfo[trackID], MAX_VID_DSI_SIZE, trackID );	
					}
					m_bTrackStart[trackID] = true;
				} 
				else {
					//printf("[Warnning] Is not keyframe.\n");
					return true;
				}

			}
			else if( trackID < m_MP4Config.videoTrack + m_MP4Config.audioTrack ) {	// audio track
				if( CODEC_AAC == m_MP4Config.trackConfig[trackID].codecType ) {
					//printf("Audio Dsi Info (trackID = %d, size = %d) : ", trackID, MAX_AUD_DSI_SIZE);
					//for( i = 0; i < MAX_AUD_DSI_SIZE; i++ )
					//{
					//	printf("0x%02x ", m_TrackDsiInfo[trackID][i]);
					//}
					//printf("\n");
					NxMp4MuxSetDsiInfo( m_hMp4Mux, m_TrackDsiInfo[trackID], MAX_AUD_DSI_SIZE, trackID );
				}
				m_bTrackStart[trackID] = true;
			}
			else {	// text track
				m_bTrackStart[trackID] = true;
			}
		}	
		memcpy( m_TrackInfo[trackID].pData, pBuffer, bufSize );
		m_TrackInfo[trackID].size = bufSize;
		
		if( trackID < m_MP4Config.videoTrack )
			m_TrackInfo[trackID].flag = pSample->GetSyncPoint();
		else 
			m_TrackInfo[trackID].flag = 1;

		m_TrackInfo[trackID].time = PTS;
		m_TrackInfo[trackID].totalDuration += duration;
	}

	pSample->Unlock();
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::SetMuxConfig( void )
{
	uint32_t i = 0;
	
	//	Ready Stream buffer
	m_StreamQueue.Reset();
	m_pSemStream->Init();
	for( i = 0; i < NUM_STRM_BUFFER; i++ )
	{
		m_StreamQueue.Push( m_pStreamBuffer[i], SIZE_STRM_BUFFER );
		m_pSemStream->Post();
	}
	m_FileQueue.Reset();

	for( i = 0; i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack; i++ )
	{
		NX_ASSERT( m_pTrackTempBuf[i] );
		memset( &m_TrackInfo[i], 0x00, sizeof(MP4_STREAM_INFO) );
		m_TrackInfo[i].pData = m_pTrackTempBuf[i];
	}

	if( m_FileName[0] == 0x00 )
	{
		GetFileNameFromCallback();	
	}

	unlink( (char*)m_FileName );
	m_OutFd = open( (char*)m_FileName, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0777 );
	printf("Create File( %d ) : %s\n", m_OutFd, m_FileName);
	if( m_OutFd < 0) {
		if( m_pNotify )
			m_pNotify->EventNotify( 0xF003, m_FileName, strlen((char*)m_FileName) + 1 );
	}
	
	for( i = 0; i < m_MP4Config.videoTrack +  m_MP4Config.audioTrack + m_MP4Config.textTrack; i++ )
	{
		// video / audio / text
		m_Mp4TrackInfo[i].bit_rate		= m_MP4Config.trackConfig[i].bitrate;

		// video / audio
		if( i < m_MP4Config.videoTrack +  m_MP4Config.audioTrack ) {
			m_Mp4TrackInfo[i].object_type	= m_MP4Config.trackConfig[i].codecType;	// MPEG4(0x20) / AVC1(0x21) / MP3(0x6b) / AAC(0x40)
		}

		if( i < m_MP4Config.videoTrack ) {
			m_Mp4TrackInfo[i].fcc_type		= MAKEFOURCC('m','p','4','v');
			m_Mp4TrackInfo[i].width			= m_MP4Config.trackConfig[i].width;
			m_Mp4TrackInfo[i].height		= m_MP4Config.trackConfig[i].height;
			m_Mp4TrackInfo[i].frame_rate	= m_MP4Config.trackConfig[i].frameRate;
		} else if( i < m_MP4Config.videoTrack +  m_MP4Config.audioTrack ) {
			m_Mp4TrackInfo[i].fcc_type		= MAKEFOURCC('m','p','4','a');
			m_Mp4TrackInfo[i].sampling_rate	= m_MP4Config.trackConfig[i].frequency;
			m_Mp4TrackInfo[i].channel_num	= m_MP4Config.trackConfig[i].channel;
		}
		else if( i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack ) {	// Text Track
			//m_Mp4TrackInfo[i].fcc_type		= MAKEFOURCC('m','p','4','a');		
		}

		m_Mp4TrackInfo[i].time_scale	= 1000;
		m_Mp4TrackInfo[i].bit_rate		= m_MP4Config.trackConfig[i].bitrate;
		m_Mp4TrackInfo[i].trak_id[0]	= i + 1;
		m_Mp4TrackInfo[i].dsi_size		= 0;
		memset( m_Mp4TrackInfo[i].dsi, 0x00, sizeof(m_Mp4TrackInfo[i].dsi) );	
	}

	m_hMp4Mux = NxMP4MuxInit( FileWriter, GetBuffer, this );

	// Add Track.
	for( i = 0; i < m_MP4Config.videoTrack + m_MP4Config.audioTrack + m_MP4Config.textTrack; i++ )
	{
#if(0)
		int32_t ret;
		ret = NxMP4MuxAddTrack( m_hMp4Mux, &m_Mp4TrackInfo[i], (i < m_MP4Config.videoTrack) ? TRACK_TYPE_VIDEO : (i < m_MP4Config.videoTrack + m_MP4Config.audioTrack) ? TRACK_TYPE_AUDIO : TRACK_TYPE_TEXT );
		printf("Add Track ( trackID = %d, TrackType = %s )\n", ret, (i < m_MP4Config.videoTrack) ? "VIDEO" : (i < m_MP4Config.videoTrack + m_MP4Config.audioTrack) ? "AUDIO" : "TEXT");
#else
		NxMP4MuxAddTrack( m_hMp4Mux, &m_Mp4TrackInfo[i], (i < m_MP4Config.videoTrack) ? TRACK_TYPE_VIDEO : (i < m_MP4Config.videoTrack + m_MP4Config.audioTrack) ? TRACK_TYPE_AUDIO : TRACK_TYPE_TEXT );
#endif
	}

	return true;
}


int32_t CNX_Mp4MuxerFilter::StartMuxing( void )
{
	m_bStartMuxing = false;
	m_bThreadExit  = false;
	if( 0 > pthread_create( &this->m_hThread, NULL, this->ThreadMain, this ) )
	{
		NX_TRACE(("CNX_Mp4MuxerFilter::%s: Fail, Create Thread\n", __FUNCTION__));
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t	CNX_Mp4MuxerFilter::StopMuxing( void )
{
	uint32_t FileSize, FilePos;
	NxDbgMsg( NX_DBG_VBS, ("%s IN\n", __func__) );
	{
		//CNX_AutoLock lock (&m_hWriteLock );
		NxMP4MuxUpdateInfo(m_hMp4Mux, &FilePos, &FileSize);
		NxMP4MuxClose(m_hMp4Mux);
	}
	m_bThreadExit = true;
	m_pSemFile->Post();
	pthread_join( m_hThread, NULL );

	//	Finalize MP4 File
	if( m_OutFd > 0){
		lseek(m_OutFd, FilePos, SEEK_SET);
		FileSize = MAKE4CC(FileSize);

		int32_t ret = write( m_OutFd, &FileSize, 4);
		if( 0 > ret ) {
			if( m_pNotify )
				m_pNotify->EventNotify( 0xF004, m_FileName, strlen((char*)m_FileName) + 1 );			
		}		

		close( m_OutFd );
		m_OutFd = 0;

		if( m_nWritingMode == WRITING_MODE_NORMAL ) {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1001, m_FileName, strlen((char*)m_FileName) + 1 );
		}
		else {
			if( m_pNotify ) 
				m_pNotify->EventNotify( 0x1002, m_FileName, strlen((char*)m_FileName) + 1 );
		}
	}
	m_hThread = 0x00;

	NxDbgMsg( NX_DBG_VBS, ("%s OUT\n", __func__) );

	return true;
}

//------------------------------------------------------------------------------
//
//	Mp4 muxer specification function for new file name.
//
int32_t CNX_Mp4MuxerFilter::SetFileName( const char *fileName )
{
	NxDbgMsg( NX_DBG_INFO, ("Set file name : %s\n", fileName) );
	strcpy( (char*)m_FileName, fileName );
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::EnableMp4Muxing( bool enable )
{
	CNX_AutoLock lock ( &m_hWriteLock );
	NxDbgMsg( NX_DBG_INFO, (TEXT("%s : %s -- > %s\n"), __func__, (m_bEnableMux)?"Enable":"Disable", (enable)?"Enable":"Disable") );
	if( !m_bRun )
	{
		m_bEnableMux = enable;
		return true;
	}

	if( enable )
	{
		//	Diable --> Enable
		if( !m_bEnableMux ){
			m_bEnableMux = enable;
			SetMuxConfig();
			StartMuxing();
		}
	}else{
		//	Enable --> Disable
		if( m_bEnableMux ){
			m_bEnableMux = enable;
			StopMuxing();
			memset( m_FileName, 0x00, sizeof(m_FileName) );
		}
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::SetDsiInfo( uint32_t trackID, uint8_t *dsiInfo, int32_t dsiSize )
{
	memset( m_TrackDsiInfo[trackID], 0x00, MAX_VID_DSI_SIZE );
	memcpy( m_TrackDsiInfo[trackID], dsiInfo, dsiSize );

	//NxDbgMsg( NX_DBG_DEBUG, (TEXT("%s(): DSI Infomation( TrackID = %d, size = %d ) :: "), __func__, trackID, dsiSize) );
	//for(int32_t i = 0; i < dsiSize; i++)
	//{
	//	printf( "0x%02x ", m_TrackDsiInfo[trackID][i] );
	//}
	//NxDbgMsg( NX_DBG_DEBUG, (TEXT("\n")) );

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_Mp4MuxerFilter::RegFileNameCallback( int32_t (*cbFunc)(uint8_t *, uint32_t) )
{
	NX_ASSERT( cbFunc );
	if( cbFunc ) {
		FileNameCallbackFunc = cbFunc;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_Mp4MuxerFilter::GetFileNameFromCallback( void )
{
	// user define filename
	if( FileNameCallbackFunc ) {
		uint32_t bufSize = 0;
		if( !FileNameCallbackFunc(m_FileName, bufSize) ) {
			// Error!
		}
	// default filename
	} else {
		time_t eTime;
		struct tm *eTm;

		time( &eTime );
		//eTm = gmtime( &eTime );
		eTm = localtime( &eTime );

		sprintf((char*)m_FileName, "clip_%04d%02d%02d_%02d%02d%02d.mp4",
			eTm->tm_year + 1900, eTm->tm_mon + 1, eTm->tm_mday, eTm->tm_hour, eTm->tm_min, eTm->tm_sec );
	}
}
