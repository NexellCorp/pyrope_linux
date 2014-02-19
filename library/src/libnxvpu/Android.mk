LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# LOCAL_PRELINK_MODULE := false

NX_PYROPE_INCLUDE := $(TOP)/hardware/nexell/pyrope/include
NX_LINUX_INCLUDE  := $(TOP)/linux/pyrope/library/include

LOCAL_SHARED_LIBRARIES :=	\
	liblog \
	libcutils \
	libion \
	libion-nexell

LOCAL_STATIC_LIBRARIES := \
	libnxmalloc

LOCAL_C_INCLUDES := system/core/include/ion \
					$(NX_PYROPE_INCLUDE) \
					$(NX_LINUX_INCLUDE)

LOCAL_CFLAGS := 

LOCAL_SRC_FILES := \
	nx_video_api.c

LOCAL_MODULE := libnx_vpu

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
