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

#include <android/log.h>

#include <CNX_Base.h>
#include <INX_Preference.h>

#define NX_DTAG     "libnxpreferencendk"

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

#define MAX_STR_SIZE        512

static CNX_Mutex hLock;

//------------------------------------------------------------------------------
JNIEXPORT jstring JNICALL NX_PreferenceRead( JNIEnv *env, jclass obj, jstring config, jstring key )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );
    CNX_AutoLock lock( &hLock );

    const char *pConfig = env->GetStringUTFChars( config, 0 );
    const char *pKey    = env->GetStringUTFChars( key, 0 );

    jstring result = NULL;

    INX_Preference *pPreference = GetPreferenceHandle();
    if( NULL != pPreference )
    {
        char *pValue = NULL;
        char szPath[MAX_STR_SIZE] = {0x00, };
        snprintf(szPath, sizeof(szPath), "%s/%s.xml", CINEMA_PREFERENCE_PATH, pConfig);

        pPreference->Open( szPath );
        pPreference->Read( (char*)pKey, &pValue );
        // NxDbgMsg( NX_DBG_INFO, ">> path( %s ), key( %s ), value( %s )\n", szPath, pKey, pValue );
        if( NULL != pValue ) result = env->NewStringUTF(pValue);
        pPreference->Close();
        delete pPreference;
    }

    env->ReleaseStringUTFChars( config, pConfig );
    env->ReleaseStringUTFChars( key, pKey );

    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
    return result;
}

//------------------------------------------------------------------------------
JNIEXPORT void JNICALL NX_PreferenceWrite( JNIEnv *env, jclass obj, jstring config, jstring key, jstring value )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );
    CNX_AutoLock lock( &hLock );

    const char *pConfig = env->GetStringUTFChars( config, 0 );
    const char *pKey    = env->GetStringUTFChars( key, 0 );
    const char *pValue  = env->GetStringUTFChars( value, 0 );

    INX_Preference *pPreference = GetPreferenceHandle();
    if( NULL != pPreference )
    {
        char szPath[MAX_STR_SIZE] = {0x00, };
        snprintf(szPath, sizeof(szPath), "%s/%s.xml", CINEMA_PREFERENCE_PATH, pConfig);

        pPreference->Open( szPath );
        pPreference->Write( (char*)pKey, (char*)pValue );
        pPreference->Close();
        delete pPreference;
    }

    env->ReleaseStringUTFChars( config, pConfig );
    env->ReleaseStringUTFChars( key, pKey );
    env->ReleaseStringUTFChars( value, pValue );
    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
}


//------------------------------------------------------------------------------
JNIEXPORT void JNICALL NX_PreferenceRemove( JNIEnv *env, jclass obj, jstring config, jstring key )
{
    NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );
    CNX_AutoLock lock( &hLock );

    const char *pConfig = env->GetStringUTFChars( config, 0 );
    const char *pKey    = env->GetStringUTFChars( key, 0 );

    INX_Preference *pPreference = GetPreferenceHandle();
    if( NULL != pPreference )
    {
        char szPath[MAX_STR_SIZE] = {0x00, };
        snprintf(szPath, sizeof(szPath), "%s/%s.xml", CINEMA_PREFERENCE_PATH, pConfig);

        pPreference->Open( szPath );
        pPreference->Remove( (char*)pKey );
        pPreference->Close();
        delete pPreference;
    }

    env->ReleaseStringUTFChars( config, pConfig );
    env->ReleaseStringUTFChars( key, pKey );
    NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
}

//------------------------------------------------------------------------------
static JNINativeMethod sMethods[] = {
    //  Native Function Name,           Signature,                                                  C++ Function Name
    { "NX_PreferenceRead",              "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)NX_PreferenceRead    },
    { "NX_PreferenceWrite",             "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",(void*)NX_PreferenceWrite   },
    { "NX_PreferenceRemove",            "(Ljava/lang/String;Ljava/lang/String;)V",                  (void*)NX_PreferenceRemove  },
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

    if( RegisterNativeMethods(env, "com/samsung/vd/baseutils/VdPreference", sMethods, sizeof(sMethods) / sizeof(sMethods[0]) ) != JNI_TRUE ) {
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
