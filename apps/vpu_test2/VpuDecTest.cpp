#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>	//	gettimeofday

#include <nx_fourcc.h>
#include <nx_alloc_mem.h>
#include <nx_video_api.h>
#include "MediaExtractor.h"
#include "CodecInfo.h"
#include "Util.h"

#ifdef ANDROID
#include "NX_AndroidRenderer.h"
#else
#include "nx_dsp.h"
#endif


unsigned char streamBuffer[4*1024*1024];
unsigned char seqData[1024*4];

//#define _SEEK_TEST_


//	Display Window Screen Size
#define	WINDOW_WIDTH		1280
#define	WINDOW_HEIGHT		720
#define	NUMBER_OF_BUFFER	12


#ifndef ANDROID
/*static uint64_t NX_GetTickCount( void )
{
	uint64_t ret;
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	ret = ((uint64_t)tv.tv_sec)*1000000 + tv.tv_usec;
	return ret;
}*/
#endif

int32_t VpuDecMain( CODEC_APP_DATA *pAppData )
{
	VID_TYPE_E vpuCodecType;
	VID_ERROR_E vidRet;
	NX_VID_SEQ_IN seqIn;
	NX_VID_SEQ_OUT seqOut;
	NX_VID_DEC_HANDLE hDec;
	NX_VID_DEC_IN decIn;
	NX_VID_DEC_OUT decOut;
	int32_t seqSize = 0;
	int32_t bInit=0;
	int32_t outCount=0, frameCount=0;
	int32_t prevIdx = -1;
	int32_t size, key = 0;
	int64_t timeStamp = -1;
	int32_t mp4Class=0;
	int32_t codecTag=-1, codecId=-1;
	int32_t imgWidth=-1, imgHeight=-1;
	int32_t instanceIdx;
	int32_t brokenB = 0;
	int32_t iPrevIdx = -1;
#ifndef ANDROID
	uint64_t startTime, endTime, totalTime = 0;
#endif
	FILE *fpOut = NULL;

	CMediaReader *pMediaReader = new CMediaReader();
	if( !pMediaReader->OpenFile( pAppData->inFileName ) )
	{
		printf("Cannot open media file(%s)\n", pAppData->inFileName);
		exit(-1);
	}
	pMediaReader->GetVideoResolution(&imgWidth, &imgHeight);

	if( pAppData->outFileName )
	{
		fpOut = fopen( pAppData->outFileName, "wb" );
	}

#ifdef ANDROID
	CNX_AndroidRenderer *pAndRender = new CNX_AndroidRenderer(WINDOW_WIDTH, WINDOW_HEIGHT);
	NX_VID_MEMORY_HANDLE *pMemHandle;
	pAndRender->GetBuffers(NUMBER_OF_BUFFER, imgWidth, imgHeight, &pMemHandle );
	NX_VID_MEMORY_HANDLE hVideoMemory[NUMBER_OF_BUFFER];
	for( int32_t i=0 ; i<NUMBER_OF_BUFFER ; i++ )
	{
		hVideoMemory[i] = pMemHandle[i];
	}
#else
	//	Linux
	DISPLAY_HANDLE hDsp;
	DISPLAY_INFO dspInfo;
	dspInfo.port = 0;
	dspInfo.module = 0;
	dspInfo.width = imgWidth;
	dspInfo.height = imgHeight;
	dspInfo.numPlane = 1;
	dspInfo.dspSrcRect.left = 0;
	dspInfo.dspSrcRect.top = 0;
	dspInfo.dspSrcRect.right = imgWidth;
	dspInfo.dspSrcRect.bottom = imgHeight;
	dspInfo.dspDstRect.left = pAppData->dspX;
	dspInfo.dspDstRect.top = pAppData->dspY;
	dspInfo.dspDstRect.right = pAppData->dspX + pAppData->dspWidth;
	dspInfo.dspDstRect.bottom = pAppData->dspY + pAppData->dspHeight;
	hDsp = NX_DspInit( &dspInfo );
	NX_DspVideoSetPriority(0, 0);
	if( hDsp == NULL )
	{
		printf("Display Failed!!!\n");
		exit(-1);
	}
#endif	//	ANDROID

	pMediaReader->GetCodecTagId( AVMEDIA_TYPE_VIDEO, &codecTag, &codecId  );

	vpuCodecType = (VID_TYPE_E)(CodecIdToVpuType(codecId, codecTag));

	mp4Class = fourCCToMp4Class( codecTag );
	if( mp4Class == -1 )
		mp4Class = codecIdToMp4Class( codecId );
	//mp4Class = 0;

	printf("vpuCodecType = %d, mp4Class = %d\n", vpuCodecType, mp4Class );

	if( NULL == (hDec = NX_VidDecOpen(vpuCodecType, mp4Class, 0, &instanceIdx)) )
	{
		printf("NX_VidDecOpen(%d) failed!!!\n", vpuCodecType);
		return -1;
	}

	seqSize = pMediaReader->GetVideoSeqInfo( streamBuffer );

	while( 1 )
	{
		//	ReadStream
		if( 0 != pMediaReader->ReadStream( CMediaReader::MEDIA_TYPE_VIDEO, streamBuffer+seqSize, &size, &key, &timeStamp ) )
		{
			break;
		}

		if( !bInit && !key )
		{
			continue;
		}

		if( !bInit )
		{
			memset( &seqIn, 0, sizeof(seqIn) );
			seqIn.addNumBuffers = 4;
			seqIn.enablePostFilter = 0;
			seqIn.seqInfo = streamBuffer;
			seqIn.seqSize = seqSize + size;
			seqIn.enableUserData = 0;
			seqIn.disableOutReorder = 0;
#ifdef ANDROID
			//	Use External Video Memory
			seqIn.numBuffers = NUMBER_OF_BUFFER;
			seqIn.pMemHandle = &hVideoMemory[0];
#endif

#if 1
			vidRet = NX_VidDecParseVideoCfg(hDec, &seqIn, &seqOut);
			printf("Parser Return = %d \n", vidRet );
			seqIn.width = seqOut.width;
			seqIn.height = seqOut.height;
			vidRet = NX_VidDecInit( hDec, &seqIn );
#else
			vidRet = NX_VidDecInit( hDec, &seqIn, &seqOut );
#endif
			if( vidRet == VID_NEED_STREAM )
			{
				printf("VPU Initialize Failed!!!\n");
				break;
			}

			printf("<<<<<<<<<<< Init In >>>>>>>>>>>>>> \n");
			printf("seqInfo = %x \n", seqIn.seqInfo);
			printf("seqSize = %d \n", seqIn.seqSize);
			{
				int32_t i;
				for (i=0 ; i<seqIn.seqSize ; i++)
				{
					printf("%2x ", seqIn.seqInfo[i]);
					if (i > 16) break;
				}
				printf("\n");
			}
			printf("width = %d \n", seqIn.width);
			printf("heigh = %d \n", seqIn.height);
			printf("pMemHandle = %x \n", seqIn.pMemHandle);
			printf("numBuffers = %d \n", seqIn.numBuffers);
			printf("addNumBuffers = %d \n", seqIn.addNumBuffers);
			printf("disableOutReorder = %d \n", seqIn.disableOutReorder);
			printf("enablePostFilter = %d \n", seqIn.enablePostFilter);
			printf("enableUserData = %d \n", seqIn.enableUserData);

			printf("<<<<<<<<<<< Init Out >>>>>>>>>>>>>> \n");
			printf("minBuffers = %d \n", seqOut.minBuffers);
			printf("numBuffers = %d \n", seqOut.numBuffers);
			printf("width = %d \n", seqOut.width);
			printf("height = %d \n", seqOut.height);
			printf("frameBufDelay = %d \n", seqOut.frameBufDelay);
			printf("isInterace = %d \n", seqOut.isInterlace);
			printf("userDataNum = %d \n", seqOut.userDataNum);
			printf("userDataSize = %d \n", seqOut.userDataSize);
			printf("userDataBufFull = %d \n", seqOut.userDataBufFull);
			printf("frameRateNum = %d \n", seqOut.frameRateNum);
			printf("frameRateDen = %d \n", seqOut.frameRateDen);
			printf("vp8ScaleWidth = %d \n", seqOut.vp8ScaleWidth);
			printf("vp8ScaleHeight = %d \n", seqOut.vp8ScaleHeight);
			printf("unsupportedFeature = %d \n", seqOut.unsupportedFeature);

			seqSize = 0;
			bInit = 1;
			size = 0;
			continue;
		}

		memset(&decIn, 0, sizeof(decIn));
		decIn.strmBuf = streamBuffer;
#ifndef _SEEK_TEST_
		decIn.strmSize = size;
		/*if ( frameCount % 50 == 7 )
		{
			unsigned char *pbyTmp = streamBuffer;
			pbyTmp[size/4] = ~(pbyTmp[size/4]);
			pbyTmp[size/2] = ~(pbyTmp[size/2]);
		}*/
#else
		decIn.strmSize = ( frameCount % 50 == 7 ) ? (size / 2) : (size);
#endif
		decIn.timeStamp = timeStamp;
		decIn.eos = ( decIn.strmSize > 0 || frameCount == 0 ) ? (0) : (1);

#ifdef _SEEK_TEST_
		/*if ( frameCount < 4000 )
		{
			size = 0;
			frameCount++;
			continue;
		}
		else if ( frameCount == 4000 )
		{
			decOut.outFrmReliable_0_100 = 0;
			decOut.outDecIdx = 0;
		}*/

		if ( (decOut.outFrmReliable_0_100 != 100) && (decOut.outDecIdx >= 0) && (decIn.strmSize > 0) && (frameCount > 0) )
		{
			int32_t iFrameType;
			int32_t iCheckType = ( vpuCodecType == NX_AVC_DEC ) ? ( PIC_TYPE_IDR ) : ( PIC_TYPE_I );

			NX_VidDecGetFrameType( vpuCodecType, &decIn, &iFrameType );
			printf("[%d] Get Type = %d, size = %d \n", frameCount, iFrameType, size );

			if ( iFrameType != iCheckType )
			{
				size = 0;
				frameCount++;
				continue;
			}
			else
			{
				brokenB = 0;
				iPrevIdx = -1;
			}
		}

		if ( brokenB < 2 )
		{
			int32_t iFrameType;

			NX_VidDecGetFrameType( vpuCodecType, &decIn, &iFrameType );
			if ( iFrameType == PIC_TYPE_B )
			{
				size = 0;
				frameCount++;
				continue;
			}
		}
#endif

#ifndef ANDROID
		startTime = NX_GetTickCount();
#endif
		vidRet = NX_VidDecDecodeFrame( hDec, &decIn, &decOut );
#ifndef ANDROID
		endTime = NX_GetTickCount();
		totalTime += (endTime - startTime);

		//if ( decOut.outFrmReliable_0_100 != 100 ) {
		printf("(%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x) ", decIn.strmBuf[0], decIn.strmBuf[1], decIn.strmBuf[2], decIn.strmBuf[3], decIn.strmBuf[4], decIn.strmBuf[5], decIn.strmBuf[6], decIn.strmBuf[7],
			decIn.strmBuf[8], decIn.strmBuf[9], decIn.strmBuf[10], decIn.strmBuf[11], decIn.strmBuf[12], decIn.strmBuf[13], decIn.strmBuf[14], decIn.strmBuf[15]);
		printf("Frame[%5d]: size=%6d, DspIdx=%2d, DecIdx=%2d, InTimeStamp=%7lld, outTimeStamp=%7lld, time=%6lld, interlace=%d(%d), Reliable=%d, type = %d, MultiResol=%d, upW=%d, upH=%d\n",
			frameCount, decIn.strmSize, decOut.outImgIdx, decOut.outDecIdx, timeStamp, decOut.timeStamp, (endTime-startTime), decOut.isInterlace, decOut.topFieldFirst, decOut.outFrmReliable_0_100, decOut.picType, decOut.multiResolution, decOut.upSampledWidth, decOut.upSampledHeight);
		//}
#else
		printf("Frame[%5d]: size=%6d, DspIdx=%2d, DecIdx=%2d, InTimeStamp=%7lld, outTimeStamp=%7lld, interlace=%d(%d), Reliable=%d, type = %d, MultiResol=%d, upW=%d, upH=%d\n",
			frameCount, decIn.strmSize, decOut.outImgIdx, decOut.outDecIdx, timeStamp, decOut.timeStamp, decOut.isInterlace, decOut.topFieldFirst, decOut.outFrmReliable_0_100, decOut.picType, decOut.multiResolution, decOut.upSampledWidth, decOut.upSampledHeight);
#endif

		if( vidRet == VID_NEED_STREAM )
		{
			printf("VID_NEED_MORE_BUF NX_VidDecDecodeFrame\n");
			continue;
		}
		if( vidRet < 0 )
		{
			printf("Decoding Error!!!\n");
			exit(-2);
		}

#ifdef _SEEK_TEST_
		if ( (brokenB < 2) && (decOut.outDecIdx >= 0) )
		{
			int32_t iFrameType;

			NX_VidDecGetFrameType( vpuCodecType, &decIn, &iFrameType );
			if ( (iFrameType == PIC_TYPE_I) || (iFrameType == PIC_TYPE_P) || (iFrameType == PIC_TYPE_IDR) )
			{
				if ( iPrevIdx != decOut.outDecIdx )
				{
					brokenB++;
				}
				iPrevIdx = decOut.outDecIdx;
			}
		}
#endif

		if( decOut.outImgIdx >= 0  )
		{
#ifdef ANDROID
			pAndRender->DspQueueBuffer( NULL, decOut.outImgIdx );
			if( prevIdx != -1 )
			{
				pAndRender->DspDequeueBuffer(NULL, NULL);
			}
#else
			NX_DspQueueBuffer( hDsp, &decOut.outImg );
			if( outCount != 0 )
			{
				NX_DspDequeueBuffer( hDsp );
			}
#endif

			if ( fpOut )
			{
				int32_t h;
				uint8_t *pbyImg = (uint8_t *)(decOut.outImg.luVirAddr);
				for(h=0 ; h<decOut.height ; h++)
				{
					fwrite(pbyImg, 1, decOut.width, fpOut);
					pbyImg += decOut.outImg.luStride;
				}

				pbyImg = (uint8_t *)(decOut.outImg.cbVirAddr);
				for(h=0 ; h<decOut.height/2 ; h++)
				{
					fwrite(pbyImg, 1, decOut.width/2, fpOut);
					pbyImg += decOut.outImg.cbStride;
				}

				pbyImg = (uint8_t *)(decOut.outImg.crVirAddr);
				for(h=0 ; h<decOut.height/2 ; h++)
				{
					fwrite(pbyImg, 1, decOut.width/2, fpOut);
					pbyImg += decOut.outImg.crStride;
				}
			}

			if( prevIdx != -1 )
			{
				NX_VidDecClrDspFlag( hDec, &decOut.outImg, prevIdx );
			}
			prevIdx = decOut.outImgIdx;
			outCount ++;
		}
		else if ( decIn.eos == 1)		break;

		if ( (decOut.outFrmReliable_0_100 != 100) && (decOut.outDecIdx >= 0) )
		{
			NX_VidDecFlush( hDec );
		}

		//if ( frameCount > 5000 ) break;

		frameCount++;
	}

	NX_VidDecClose( hDec );

#ifdef ANDROID
	if( pAndRender )
		delete pAndRender;
#else
	if( hDsp )
		NX_DspClose(hDsp);
	printf("Avg Time = %6lld (%6lld / %d) \n", totalTime / frameCount, totalTime, frameCount);
#endif
	if( pMediaReader )
		delete pMediaReader;
	if ( fpOut )
		fclose(fpOut);

	return 0;
}
