LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
VD_TOP := $(LOCAL_PATH)/../../../
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	main.cpp

LOCAL_C_INCLUDES += \
	system/core/include	\
	$(VD_TOP)/src/include \
	$(VD_TOP)/src/utils

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= NxCinemaDiagnostics
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../result/bin/

include $(BUILD_EXECUTABLE)
