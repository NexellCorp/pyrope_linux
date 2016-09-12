LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
VD_TOP := $(LOCAL_PATH)/../../../
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	sock_client.cpp

LOCAL_C_INCLUDES += \
	$(VD_TOP)/src/utils \
	$(VD_TOP)/src/include

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= nx_lsock_server
LOCAL_MODULE_PATH := $(VD_TOP)/bin

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
VD_TOP := $(LOCAL_PATH)/../../../
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	sock_server.cpp

LOCAL_C_INCLUDES += \
	$(VD_TOP)/src/utils \
	$(VD_TOP)/src/include

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= nx_lsock_client
LOCAL_MODULE_PATH := $(VD_TOP)/bin

include $(BUILD_EXECUTABLE)
