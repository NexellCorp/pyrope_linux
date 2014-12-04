LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
		src/bitpack.c		\
		src/decode.c		\
		src/info.c			\
		src/internal.c		\
		src/state.c			\
		src/theora_parser.c

LOCAL_C_INCLUDES += \
	frameworks/native/include	\
	$(LOCAL_PATH)/include    	\

LOCAL_SHARED_LIBRARIES := \
	libcutils		\
	libutils		\
	libion			\

LOCAL_MODULE:= libtheoraparser

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
