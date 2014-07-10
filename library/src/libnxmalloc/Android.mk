LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES :=	\
	liblog \
	libcutils

LOCAL_C_INCLUDES += system/core/include/ion \
					$(TOP)/hardware/nexell/pyrope/include \
					$(TOP)/linux/pyrope/library/include

LOCAL_CFLAGS += 

LOCAL_SRC_FILES := \
	nx_alloc_mem_ion.c

LOCAL_MODULE := libnxmalloc

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
