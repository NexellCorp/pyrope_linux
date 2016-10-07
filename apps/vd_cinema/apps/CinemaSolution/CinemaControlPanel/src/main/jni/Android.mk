LOCAL_PATH := $(call my-dir)

#
#   Add Prebuilt Library
#
include $(CLEAR_VARS)
LOCAL_MODULE := libnxcinema
LOCAL_SRC_FILES := lib/libnxcinema.so
include $(PREBUILT_SHARED_LIBRARY)

#
#   Build JNI Library
#
include $(CLEAR_VARS)
LOCAL_MODULE    := nxcinemactrl
LOCAL_SRC_FILES := NxCinemaCtrl.cpp

LOCAL_C_INCLUDES		:=	\
	$(JNI_H_INCLUDE)		\
	$(LOCAL_PATH)/include
LOCAL_LDFLAGS	+= \
	-L$(LOCAL_PATH)/lib		\
	-lnxcinema

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
