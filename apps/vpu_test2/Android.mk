LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	MediaExtractor.cpp	\
	CodecInfo.cpp		\
	NX_Queue.cpp		\
	NX_Semaphore.cpp	\
	NX_AndroidRenderer.cpp \
	Util.cpp			\
	VpuDecTest.cpp		\
	main.cpp

#	VpuEncTest.cpp		\
#	VpuJpgTest.cpp		\
#	img_proc_main.cpp	\

LOCAL_C_INCLUDES += \
	frameworks/native/include		\
	system/core/include				\
	hardware/libhardware/include	\
	hardware/nexell/pyrope/include	\
	linux/pyrope/library/include	\
	$(LOCAL_PATH)/ffmpeg/include    \
	$(LOCAL_PATH)/ffmpeg/include    \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libcutils		\
	libbinder		\
	libutils		\
	libgui			\
	libui			\
	libui			\
	libion			\
	libion-nexell	\
	libnx_vpu		\
	libtheoraparser

LOCAL_LDFLAGS += \
	-L$(LOCAL_PATH)/ffmpeg/libs	\
	-lavutil-1.2 		\
	-lavcodec-1.2   		\
	-lavformat-1.2		\
	-lavdevice-1.2		\
	-lavfilter-1.2		\
	-lswresample-1.2

LOCAL_MODULE:= codec_tests

include $(BUILD_EXECUTABLE)
