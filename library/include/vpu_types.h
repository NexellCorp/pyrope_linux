#ifndef __VPU_TYPES_H__
#define __VPU_TYPES_H__

#include "nx_alloc_mem.h"

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} VPU_RECT;

typedef struct
{
	//	Input Arguments
	int codecStd;
	int isEncoder;			//	Encoder
	int mp4Class;			//	Mpeg4 Class

	NX_MEMORY_INFO	instanceBuf;
	NX_MEMORY_INFO	streamBuf;

	//	Output Arguments
	int instIndex;			//	Instance Index
} VPU_OPEN_ARG;

typedef struct
{
	//	
	int srcWidth;		//	source image's width
	int srcHeight;		//	source image's height

	//	Set Stream Buffer Handle
	unsigned int strmBufVirAddr;
	unsigned int strmBufPhyAddr;
	int strmBufSize;

	int frameRate;		//	frame rate
	int gopSize;		//	group of picture size

	//	Rate Control
	int	enableRC;		//	Enable Rate Control
	int bitrate;		//	Bitrate
	int rcAutoSkip;		//	Auto Skip
	int maxQScale;		//	Max Quantization Scale
	int userQScale;		//	User Quantization Scale

	//	Input Buffer Chroma Interleaved
	int chromaInterleave;	//	Input Buffer Chroma Interleaved Format
	int refChromaInterleave;//	Reference Buffer's Chorma Interleaved Format

	//	ME Search Range
	int searchRange;	//	ME_SEARCH_RAGME_[0~3] ( recomand ME_SEARCH_RAGME_2 )

	//	JPEG Specific
	int rotAngle;
	int mirDirection;
	int quality;

}VPU_ENC_SEQ_ARG;


typedef struct
{
	//	Reconstruct Buffer
	int	numFrameBuffer;					//	Number Of Frame Buffers
	NX_VID_MEMORY_INFO frameBuffer[2];	//	Frame Buffer Informations

	//	Sub Sample A/B Buffer ( 1 sub sample buffer size = Framebuffer size/4 )
	NX_MEMORY_INFO subSampleBuffer[2];	//	

	//	Data partition Buffer size ( MAX WIDTH * MAX HEIGHT * 3 / 4 )
	NX_MEMORY_INFO dataPartitionBuffer;	//	Mpeg4 Only
}VPU_ENC_SET_FRAME_ARG;


typedef union
{
	struct {
		unsigned char vosData[512];
		int vosSize;
		unsigned char volData[512];
		int volSize;
	} mp4Header;
	struct {
		unsigned char spsData[512];
		int spsSize;
		unsigned char ppsData[512];
		int ppsSize;
	} avcHeader;
	struct {
		unsigned char jpegHeader[1024];
		int headerSize;
	} jpgHeader;
}VPU_ENC_GET_HEADER_ARG;


typedef struct
{
	//------------------------------------------------------------------------
	//	Input Prameter
	NX_VID_MEMORY_INFO	inImgBuffer;
	//	Rate Control Parameters
	int changeFlag;
	int enableRc;
	int forceIPicture;
	int quantParam;			//	User quantization Parameter
	int skipPicture;

	//	Dynamic Configurable Parameters


	//------------------------------------------------------------------------
	//	Output Parameter
	int frameType;					//	I, P, B, SKIP,.. etc
	unsigned char *outStreamAddr;	//	mmapped virtual address
	int outStreamSize;				//	Stream buffer size
}VPU_ENC_RUN_FRAME_ARG;


typedef struct {
	int fixedFrameRateFlag;
	int timingInfoPresent;
	int chromaLocBotField;
	int chromaLocTopField;
	int chromaLocInfoPresent;
	int colorPrimaries;
	int colorDescPresent;
	int isExtSAR;
	int vidFullRange;
	int vidFormat;
	int vidSigTypePresent;
	int vuiParamPresent;
	int vuiPicStructPresent;
	int vuiPicStruct;
} AvcVuiInfo;


//
//	Decoder Structures
//

typedef struct
{
	//	Input Information
	unsigned char *seqData;
	int seqDataSize;
	int disableOutReorder;

	//	General Output Information
	int outWidth;
	int outHeight;
	int frameRateNum;	//	Frame Rate Numerator
	int frameRateDen;	//	Frame Rate Denominator
	unsigned int bitrate;

	int profile;
	int level;
	int interlace;
	int direct8x8Flag;
	int constraint_set_flag[4];
	int aspectRateInfo;

	//	Frame Buffer Information
	int minFrameBufCnt;
	int frameBufDelay;

	//	Mpeg4 Specific Info
	int mp4ShortHeader;
	int mp4PartitionEnable;
	int mp4ReversibleVlcEnable;
	int h263AnnexJEnable;
	unsigned int mp4Class;

	//	VP8 Specific Info
	int vp8HScaleFactor;
	int vp8VScaleFactor;
	int vp8ScaleWidth;
	int vp8ScaleHeight;


	//	H.264(AVC) Specific Info
	AvcVuiInfo avcVuiInfo;
	int avcIsExtSAR;
	int cropLeft;
	int cropTop;
	int cropRight;
	int cropBottom;
	int numSliceSize;
	int worstSliceSize;
	int maxNumRefFrmFlag;

	//	VC-1
	int	vc1Psf;

	//	Mpeg2
    int mp2LowDelay;
    int mp2DispVerSize;
    int mp2DispHorSize;
	int userDataNum;
	int userDataSize;
	int userDataBufFull;
	int enableUserData;
	NX_MEMORY_INFO userDataBuffer;

}VPU_DEC_SEQ_INIT_ARG;


typedef struct
{
	//	Frame Buffers
	int	numFrameBuffer;					//	Number Of Frame Buffers
	NX_VID_MEMORY_INFO frameBuffer[20];	//	Frame Buffer Informations

	//	MV Buffer Address
	NX_MEMORY_INFO colMvBuffer;

	//	AVC Slice Buffer
	NX_MEMORY_INFO sliceBuffer;

	//	VPX Codec Specific
	NX_MEMORY_INFO pvbSliceBuffer;

}VPU_DEC_REG_FRAME_ARG;

// VP8 specific display information
typedef struct {
	unsigned int hScaleFactor   : 2;
	unsigned int vScaleFactor   : 2;
	unsigned int picWidth       : 14;
	unsigned int picHeight      : 14;
} Vp8ScaleInfo;

// VP8 specific header information
typedef struct {
	unsigned int showFrame      : 1;
	unsigned int versionNumber  : 3;
	unsigned int refIdxLast     : 8;
	unsigned int refIdxAltr     : 8;
	unsigned int refIdxGold     : 8;
} Vp8PicInfo;

typedef struct
{
	//	Input Arguments
	unsigned char	*strmData;
	int strmDataSize;
	int iFrameSearchEnable;
	int skipFrameMode;
	int decSkipFrameNum;

	//	Output Arguments
	int outWidth;
	int outHeight;

	VPU_RECT outRect;

	int indexFrameDecoded;
	int indexFrameDisplay;

	int picType;
	int picTypeFirst;
	int isInterace;
	int picStructure;
	int topFieldFirst;
	int repeatFirstField;
	int progressiveFrame;
	int fieldSequence;

	int isSuccess;

	int errReason;
	int errAddress;
	int numOfErrMBs;
	int sequenceChanged;

	//	AVC Specific Informations
	int avcFpaSeiExist;
	int avcFpaSeiValue1;
	int avcFpaSeiValue2;

	//	Output Bitstream Information
	unsigned int frameStartPos;
	unsigned int frameEndPos;

	// 
	unsigned int notSufficientPsBuffer;
	unsigned int notSufficientSliceBuffer;

	//
	unsigned int fRateNumerator;
	unsigned int fRateDenominator;
	unsigned int aspectRateInfo;	//	Use vp8ScaleInfo & vp8PicInfo in VP8

	//
	unsigned int mp4ModuloTimeBase;
	unsigned int mp4TimeIncrement;

	//	VP8 Scale Info
	Vp8ScaleInfo vp8ScaleInfo;
	Vp8PicInfo vp8PicInfo;

	NX_VID_MEMORY_INFO outFrameBuffer;

	//	MPEG2 User Data
	int userDataNum;        // User Data
	int userDataSize;
	int userDataBufFull;
    int activeFormat;

} VPU_DEC_DEC_FRAME_ARG;


typedef struct
{
	int indexFrameDisplay;
	NX_VID_MEMORY_INFO frameBuffer;
} VPU_DEC_CLR_DSP_FLAG_ARG;


//////////////////////////////////////////////////////////////////////////////
//
//		Command Arguments
//


//	Define Codec Standard
enum {
	CODEC_STD_AVC	= 0,
	CODEC_STD_VC1	= 1,
	CODEC_STD_MPEG2	= 2,
	CODEC_STD_MPEG4	= 3,
	CODEC_STD_H263	= 4,
	CODEC_STD_DIV3	= 5,
	CODEC_STD_RV	= 6,
	CODEC_STD_AVS	= 7,
	CODEC_STD_MJPG	= 8,

	CODEC_STD_THO	= 9,
	CODEC_STD_VP3	= 10,
	CODEC_STD_VP8	= 11
};

//	Search Range
enum {
	ME_SEARCH_RAGME_0,		//	Horizontal( -128 ~ 127 ), Vertical( -64 ~ 64 )
	ME_SEARCH_RAGME_1,		//	Horizontal(  -64 ~  63 ), Vertical( -32 ~ 32 )
	ME_SEARCH_RAGME_2,		//	Horizontal(  -32 ~  31 ), Vertical( -16 ~ 15 )	//	default
	ME_SEARCH_RAGME_3,		//	Horizontal(  -16 ~  15 ), Vertical( -16 ~ 15 )
};

//	Frame Buffer Format for JPEG
enum {
    IMG_FORMAT_420 = 0,
    IMG_FORMAT_422 = 1,
    IMG_FORMAT_224 = 2,
    IMG_FORMAT_444 = 3,
    IMG_FORMAT_400 = 4
};

//	JPEG Mirror Direction
enum {
	MIRDIR_NONE,
	MIRDIR_VER,
	MIRDIR_HOR,
	MIRDIR_HOR_VER,
};

//
//
//////////////////////////////////////////////////////////////////////////////

#endif	//	__VPU_TYPES_H__
