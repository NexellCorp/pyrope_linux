LOCAL_PATH:= $(call my-dir)
VD_TOP := $(LOCAL_PATH)/../../..

#
#	Build I2C Test Application for Samsung Protocol
#
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	main.cpp

LOCAL_C_INCLUDES += \
	$(VD_TOP)/src/i2c \
	$(VD_TOP)/src/include

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= nx_i2c
LOCAL_MODULE_PATH := $(VD_TOP)/bin

include $(BUILD_EXECUTABLE)


#
#	Build I2C Validation Application for Samsung Protocol
#
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	validation.cpp

LOCAL_C_INCLUDES += \
	$(VD_TOP)/src/i2c \
	$(VD_TOP)/src/include

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= nx_i2c_validate
LOCAL_MODULE_PATH := $(VD_TOP)/bin

include $(BUILD_EXECUTABLE)


#
# Build I2C Test Application for Samsung Protocol
#
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	i2c_test_cinema.c

LOCAL_C_INCLUDES += \
	$(VD_TOP)/src/include

LOCAL_SHARED_LIBRARIES := \
	libcutils	\
	libutils

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += \
	-L$(VD_TOP)/lib -lnxcinema

LOCAL_MODULE:= i2c_test
LOCAL_MODULE_PATH := $(VD_TOP)/bin

include $(BUILD_EXECUTABLE)

