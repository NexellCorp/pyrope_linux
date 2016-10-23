//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
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

#ifndef __NX_DBGMSG_H__
#define __NX_DBGMSG_H__

#include <stdint.h>

#define NX_DBG_VBS			2	// LOG_VERBOSE
#define NX_DBG_DEBUG		3	// LOG_DEBUG
#define	NX_DBG_INFO			4	// LOG_INFO
#define	NX_DBG_WARN			5	// LOG_WARN
#define	NX_DBG_ERR			6	// LOG_ERROR
#define NX_DBG_DISABLE		9

#ifndef NX_DTAG
#define	NX_DTAG		"[]"
#endif


#define FUNC_TRACE
#define	FUNC_TRACE_LEVEL	NX_DBG_DEBUG


#ifndef NX_DBG_OFF
#ifndef ANDROID
#include <stdio.h>
#define DBG_PRINT			printf
#define NxTrace(...)		do {										\
								DBG_PRINT(NX_DTAG);						\
								DBG_PRINT(" ");							\
								DBG_PRINT(__VA_ARGS__);					\
							} while(0)

#define NxDbgMsg(A, ...)	do {										\
								if( gNxDebugLevel <= A ) {				\
									DBG_PRINT(NX_DTAG);					\
									DBG_PRINT(" ");						\
									DBG_PRINT(__VA_ARGS__);				\
								}										\
							} while(0)
#define NxErrMsg(...)		do{											\
								DBG_PRINT(NX_DTAG);						\
								DBG_PRINT(" ERROR: %s Line(%d) : ", __func__, __LINE__);	\
								DBG_PRINT(__VA_ARGS__);					\
							} while(0)

#else	//	ANDROID
#include <android/log.h>
#define DBG_PRINT			__android_log_print
#define NxTrace(...)		DBG_PRINT(ANDROID_LOG_VERBOSE, NX_DTAG, __VA_ARGS__);

#define NxDbgMsg(A, ...)	do {										\
								if( gNxDebugLevel <= A ) {				\
									DBG_PRINT(A, NX_DTAG, __VA_ARGS__);	\
								}										\
							} while(0)
#define NxErrMsg(...)		DBG_PRINT(ANDROID_LOG_ERROR, NX_DTAG, __VA_ARGS__);
#endif	//	ANDROID
#else	//	NX_DBG_OFF

#define NxTrace(...)
#define NxDbgMsg(A, ...)

#endif	//	NX_DBG_OFF

#ifdef FUNC_TRACE
#define FUNC_IN()		do{											\
							NxDbgMsg(FUNC_TRACE_LEVEL, "%s() IN\n", __FUNCTION__);	\
						} while(0)

#define FUNC_OUT()		do{											\
							NxDbgMsg(FUNC_TRACE_LEVEL, "%s() OUT\n", __FUNCTION__);	\
						} while(0)
#else
#define FUNC_IN()		do{} while(0)
#define FUNC_OUT()		do{} while(0)
#endif // FUNC_TRACE


extern uint32_t gNxDebugLevel;
void NxChgDebugLevel( uint32_t level );
uint32_t NxGetDebugLevel(void);

#endif	//__NX_DBGMSG_H__
