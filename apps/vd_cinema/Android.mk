LOCAL_PATH 		:= $(call my-dir)
NX_CINEMA_TOP	:= $(LOCAL_PATH)

include $(NX_CINEMA_TOP)/src/Android.mk
include $(NX_CINEMA_TOP)/apps/Android.mk
include $(NX_CINEMA_TOP)/apps/CinemaDiagnostics/Android.mk
