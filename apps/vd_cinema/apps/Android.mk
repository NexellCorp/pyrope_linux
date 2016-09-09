LOCAL_PATH:= $(call my-dir)
NX_CINEMA_APP	:= $(LOCAL_PATH)

include $(NX_CINEMA_APP)/nap_server/Android.mk
include $(NX_CINEMA_APP)/nap_con_client/Android.mk
#include $(NX_CINEMA_APP)/test/Android.mk
