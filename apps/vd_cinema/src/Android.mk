LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Add TMS Server
LOCAL_SRC_FILES += \
	tms/tms_protocol.cpp \
	tms/NX_TMSClient.cpp \
	tms/NX_TMSServer.cpp

# Add UART Sources
LOCAL_SRC_FILES += \
	uart/CNX_Uart.cpp	\
	uart/NX_UartProtocol.cpp \
	uart/NX_SecureLinkServer.cpp

# Add I2C Sources
LOCAL_SRC_FILES += \
	i2c/CNX_I2C.cpp

# Add Utils Sources
LOCAL_SRC_FILES += \
	utils/crc32.cpp	\
	utils/NX_Utils.cpp \
	utils/ping.cpp \
	utils/NX_DbgMsg.cpp \
	utils/CNX_GpioControl.cpp \
	utils/NX_Pwm.cpp \
	utils/NX_Queue.cpp \
	utils/SockUtils.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/i2c \
	$(LOCAL_PATH)/uart \
	$(LOCAL_PATH)/utils \
	$(LOCAL_PATH)/tms

LOCAL_SHARED_LIBRARIES := \
	liblog

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS += 

LOCAL_MODULE:= libnxcinema
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../lib

include $(BUILD_SHARED_LIBRARY)
