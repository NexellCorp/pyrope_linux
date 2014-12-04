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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <nx_video_api.h>
#include <libnxscaler.h>

#include <nx_vip.h>
#include <nx_fourcc.h>

#define CAMERA_WIDTH		1024
#define CAMERA_HEIGHT		768

////////////////////////////////////////////////////////////////////////////////
//
//	Jpeg Hardware Encoder
//
int32_t jpeg_encode( uint8_t *pFileName, NX_VID_MEMORY_INFO *pVidMem, int32_t dstWidth, int32_t dstHeight, int32_t quality )
{
	FILE *outFile = NULL;
	uint8_t jpgHeader[4 * 1024];

	NX_VID_MEMORY_INFO 		*pDstMem = NULL;

	NX_VID_ENC_HANDLE 		hEnc = NULL;
	NX_VID_ENC_OUT 			encOut;
	NX_VID_ENC_INIT_PARAM 	encParam;
	int32_t					size;

	if( NULL == (outFile = fopen((char*)pFileName, "w")) ) {
		printf("Error: Cannot open file.\n");
		goto END;
	}

	if( NULL == (hEnc = NX_VidEncOpen( NX_JPEG_ENC, NULL )) ) {
		printf("Error: Encoder open failed!\n");
		goto END;
	}

	if( dstWidth == CAMERA_WIDTH && dstHeight == CAMERA_HEIGHT ) {
		pDstMem = pVidMem;
	}
	else {
		NX_SCALER_HANDLE hScaler;
		hScaler = NX_SCLOpen();
		pDstMem = NX_VideoAllocateMemory( 4096, dstWidth, dstHeight, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
		NX_SCLScaleImage( hScaler, pVidMem, pDstMem );
		NX_SCLClose( hScaler );
	}

	memset( jpgHeader, 0x00, sizeof(jpgHeader) );
	memset( &encOut, 0x00, sizeof(encOut) );

	memset( &encParam, 0, sizeof(encParam) );
	encParam.width			= pDstMem->imgWidth;
	encParam.height			= pDstMem->imgHeight;
	encParam.rotAngle		= 0;
	encParam.mirDirection	= 0;
	encParam.jpgQuality		= quality;

	NX_VidEncInit( hEnc, &encParam );

	NX_VidEncJpegGetHeader( hEnc, jpgHeader, &size );
	fwrite( jpgHeader, 1, size, outFile );

	NX_VidEncJpegRunFrame( hEnc, pDstMem, &encOut );
	fwrite( encOut.outBuf, 1, encOut.bufSize, outFile );

END:
	if( hEnc ) 		NX_VidEncClose( hEnc );
	if( outFile ) 	fclose(outFile);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//	Test Application
//
static void print_usage( void )
{
	printf("Usage : jpeg_enc [options]                                                    \n");
	printf("   -h                  : Help                                                 \n");
	printf("   -o [file]           : Output filename.                (mandatory)          \n");
	printf("   -r [width],[height] : Resize width, height.                                \n");
}

int main( int argc, char *argv[] )
{
	int32_t opt;
	uint8_t *outFileName = NULL;

	int32_t dstWidth 	= CAMERA_WIDTH;
	int32_t dstHeight 	= CAMERA_HEIGHT;

	NX_VID_MEMORY_INFO *pTmpMem = NULL;
	
	VIP_HANDLE 	hVip;
	VIP_INFO 	vipInfo;
	int64_t		vipTime;

	while( -1 != (opt = getopt(argc, argv, "ho:r:t")) )
	{
		switch( opt )
		{
			case 'h' :
				print_usage();
				goto END;
			case 'o' :
				outFileName = (uint8_t*)strdup( optarg );
				break;
			case 'r' :
				sscanf(optarg, "%d,%d", &dstWidth, &dstHeight);
				break;
			default :
				print_usage();
				goto END;
		}
	}

	if( !outFileName ) {
		printf("Error: No output file.\n");
		print_usage();
		goto END;	
	}

	// Memory allocator
	memset( &vipInfo, 0, sizeof(vipInfo) );
	vipInfo.port 		= VIP_PORT_MIPI;
	vipInfo.mode 		= VIP_MODE_CLIPPER;
	vipInfo.width 		= CAMERA_WIDTH;
	vipInfo.height 		= CAMERA_HEIGHT;
	vipInfo.numPlane 	= 1;
	vipInfo.cropX 		= 0;
	vipInfo.cropY 		= 0;
	vipInfo.cropWidth  	= CAMERA_WIDTH;
	vipInfo.cropHeight 	= CAMERA_HEIGHT;
	vipInfo.fpsNum 		= 30;
	vipInfo.fpsDen 		= 1;

	hVip = NX_VipInit( &vipInfo );
	
	pTmpMem = NX_VideoAllocateMemory( 4096, CAMERA_WIDTH, CAMERA_HEIGHT, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
	NX_VipQueueBuffer( hVip, pTmpMem );
	pTmpMem = NX_VideoAllocateMemory( 4096, CAMERA_WIDTH, CAMERA_HEIGHT, NX_MEM_MAP_LINEAR, FOURCC_MVS0 );
	NX_VipQueueBuffer( hVip, pTmpMem );
	NX_VipDequeueBuffer( hVip, &pTmpMem, &vipTime );

	jpeg_encode( outFileName, pTmpMem, dstWidth, dstHeight, 100 );

END:
	if( outFileName )	free( outFileName );

	return 0;
}
