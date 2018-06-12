LOCAL_PATH := $(call my-dir)

#
#   Add Prebuilt Library
#
include $(CLEAR_VARS)
LOCAL_MODULE := libnxpreference
LOCAL_SRC_FILES := lib/libnxpreference.so
include $(PREBUILT_SHARED_LIBRARY)

#
#   Build JNI Library
#
include $(CLEAR_VARS)
LOCAL_MODULE    := nxpreferencendk
LOCAL_SRC_FILES := NX_Preference.cpp

LOCAL_C_INCLUDES		:=	\
	$(JNI_H_INCLUDE)		\
	$(LOCAL_PATH)/include

LOCAL_LDFLAGS	+= \
	-L$(LOCAL_PATH)/lib		\
	-lnxpreference

LOCAL_SHARED_LIBRARIES	:=	\
    liblog					\
    libandroid

LOCAL_LDFLAGS	+= \
	-L$(LOCAL_PATH)/lib

LOCAL_LDLIBS	:= 			\
	-llog					\
	-landroid

LOCAL_MODULE_TAGS       := optional
LOCAL_PRELINK_MODULE    := false

include $(BUILD_SHARED_LIBRARY)
