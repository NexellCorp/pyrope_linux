//------------------------------------------------------------------------------
//
//  Copyright (C) 2016 Nexell Co. All Rights Reserved
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
//-----------------------------------------------------------------------------

#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <stdlib.h> //malloc , free

#include <android/log.h>

#include <NX_IPCClient.h>

#define NX_DTAG     "libnxcinemacontrol"

//
//  Debug Tools
//
#define ENABLE_HEX_DUMP     0

#define NX_DBG_VBS          2   // ANDROID_LOG_VERBOSE
#define NX_DBG_DEBUG        3   // ANDROID_LOG_DEBUG
#define NX_DBG_INFO         4   // ANDROID_LOG_INFO
#define NX_DBG_WARN         5   // ANDROID_LOG_WARN
#define NX_DBG_ERR          6   // ANDROID_LOG_ERROR
#define NX_DBG_DISABLE      9

int gNxFilterDebugLevel     = NX_DBG_ERR;

#define DBG_PRINT           __android_log_print
#define NxDbgMsg(A, ...)    do {                                        \
                                if( gNxFilterDebugLevel <= A ) {        \
                                    DBG_PRINT(A, NX_DTAG, __VA_ARGS__); \
                                }                                       \
                            } while(0);

//#define MAX_PAYLOAD_SIZE    65533
//#define MAX_PAYLOAD_SIZE    5*1024*1024

//------------------------------------------------------------------------------
#if ENABLE_HEX_DUMP
void NX_HexDump( const void *data, int32_t size )
{
    int32_t i=0, offset = 0;
    char tmp[32];
    static char lineBuf[1024];
    const uint8_t *_data = (const uint8_t*)data;
    while( offset < size )
    {
        sprintf( lineBuf, "%08lx :  ", (unsigned long)offset );
        for( i=0 ; i<16 ; ++i )
        {
            if( i == 8 ){
                strcat( lineBuf, " " );
            }
            if( offset+i >= size )
            {
                strcat( lineBuf, "   " );
            }
            else{
                sprintf(tmp, "%02x ", _data[offset+i]);
                strcat( lineBuf, tmp );
            }
        }
        strcat( lineBuf, "   " );

        //     Add ACSII A~Z, & Number & String
        for( i=0 ; i<16 ; ++i )
        {
            if( offset+i >= size )
            {
                break;
            }
            else{
                if( isprint(_data[offset+i]) )
                {
                    sprintf(tmp, "%c", _data[offset+i]);
                    strcat(lineBuf, tmp);
                }
                else
                {
                    strcat( lineBuf, "." );
                }
            }
        }

        strcat(lineBuf, "\n");
        __android_log_print(ANDROID_LOG_DEBUG, NX_DTAG, "%s", lineBuf );

        offset += 16;
    }
}
#else
void NX_HexDump( const void * /*data*/, int32_t /*size*/ )
{
}
#endif

//------------------------------------------------------------------------------
JNIEXPORT jbyteArray JNICALL NX_CinemaCtrl( JNIEnv *env, jclass obj, jint cmd, jbyteArray inArray )
{
	int32_t iRet = 0;
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

    //uint8_t*    pTmpBuf = (uint8_t*)malloc( sizeof(uint8_t) * MAX_PAYLOAD_SIZE );
    uint8_t*    pTmpBuf = NULL;
    uint32_t    nTmpSize = 0;

    //memset( pTmpBuf, 0x00, sizeof(uint8_t) * MAX_PAYLOAD_SIZE );

    if( NULL != inArray )
    {
        jbyte* pData = env->GetByteArrayElements( inArray, NULL );
        jlong  dataSize = env->GetArrayLength( inArray );

        nTmpSize = (uint32_t)dataSize;

        pTmpBuf = (uint8_t*)malloc( sizeof(uint8_t) * nTmpSize );
        memset( pTmpBuf, 0x00, sizeof(uint8_t) * nTmpSize );

        memcpy( pTmpBuf, (uint8_t*)pData, dataSize );
        env->ReleaseByteArrayElements( inArray, pData, 0 );
    }
    else
    {
        nTmpSize = 4;
        pTmpBuf = (uint8_t*)malloc( sizeof(uint8_t) * nTmpSize );
        memset( pTmpBuf, 0x00, sizeof(uint8_t) * nTmpSize );
    }

	iRet = NX_IPCSendCommand( cmd, &pTmpBuf, &nTmpSize );
    if( 0 > iRet )
    {
       __android_log_print(ANDROID_LOG_INFO, "JNI", "NX_IPCSendCommand failed ret : %d " , iRet);
       NxDbgMsg( NX_DBG_ERR, "Fail, NX_IPCSendCommand(). Ret : %d\n" ,iRet);
       NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
       return NULL;
    }

    jbyteArray outArray = env->NewByteArray( nTmpSize );
    if( NULL == outArray )
    {
        NxDbgMsg( NX_DBG_ERR, "Fail, NewCharArray().\n" );
        NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
        return NULL;
    }

    env->SetByteArrayRegion( outArray, 0, nTmpSize, (jbyte*)pTmpBuf );
    NX_HexDump( pTmpBuf, nTmpSize );
    free( pTmpBuf );

    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
    return outArray;
}

//------------------------------------------------------------------------------
static JNINativeMethod sMethods[] = {
    //  Native Function Name,           Signature,                          C++ Function Name
    { "NX_CinemaCtrl",                  "(I[B)[B",                          (void*)NX_CinemaCtrl        },
};

//------------------------------------------------------------------------------
static int RegisterNativeMethods( JNIEnv *env, const char *className, JNINativeMethod *gMethods, int numMethods )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

    jclass clazz;
    int result = JNI_FALSE;

    clazz = env->FindClass( className );
    if( clazz == NULL ) {
        NxDbgMsg( NX_DBG_ERR, "%s(): Native registration unable to find class '%s'", __FUNCTION__, className );
        goto FAIL;
    }

    if( env->RegisterNatives( clazz, gMethods, numMethods) < 0 ) {
        NxDbgMsg( NX_DBG_ERR, "%s(): RegisterNatives failed for '%s'", __FUNCTION__, className);
        goto FAIL;
    }

    result = JNI_TRUE;

FAIL:
    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
    return result;
}

//------------------------------------------------------------------------------
jint JNI_OnLoad( JavaVM *vm, void *reserved )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

    jint result = -1;
    JNIEnv *env = NULL;

    if( vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK ) {
        NxDbgMsg( NX_DBG_ERR, "%s(): GetEnv failed!\n", __FUNCTION__ );
        goto FAIL;
    }

    if( RegisterNativeMethods(env, "com/samsung/vd/cinemacontrolpanel/Utils/NxCinemaCtrl", sMethods, sizeof(sMethods) / sizeof(sMethods[0]) ) != JNI_TRUE ) {
        NxDbgMsg( NX_DBG_ERR, "%s(): RegisterNativeMethods failed!", __FUNCTION__ );
        goto FAIL;
    }

    result = JNI_VERSION_1_4;

FAIL:
    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
    return result;
}

//------------------------------------------------------------------------------
void JNI_OnUnload( JavaVM *vm, void *reserved )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

    JNIEnv *env = NULL;

    if( vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK ) {
        NxDbgMsg( NX_DBG_ERR, "%s(): GetEnv failed!", __FUNCTION__ );
        goto FAIL;
    }

FAIL:
    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
}
