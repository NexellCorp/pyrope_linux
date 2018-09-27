LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Add Protocol
LOCAL_SRC_FILES += \
	protocol/ipc_protocol.cpp \
	protocol/NX_IPCClient.cpp \
	protocol/NX_IPCServer.cpp \
	protocol/gdc_protocol.cpp \
	protocol/NX_TMSClient.cpp \
	protocol/NX_TMSServer.cpp

# Add Cinema
LOCAL_SRC_FILES += \
	cinema/CNX_CinemaManager.cpp

# Add UART Sources
LOCAL_SRC_FILES += \
	uart/CNX_Uart.cpp	\
	uart/NX_UartProtocol.cpp \
	uart/NX_SecureLinkServer.cpp

# Add I2C Sources
LOCAL_SRC_FILES += \
	i2c/CNX_I2C.cpp

# Add EEPRom Sources
LOCAL_SRC_FILES += \
	eeprom/CNX_EEPRom.cpp	\
	eeprom/CNX_EEPRomData.cpp

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
	$(LOCAL_PATH)/protocol	\
	$(LOCAL_PATH)/cinema

LOCAL_SHARED_LIBRARIES := \
	liblog

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS +=

LOCAL_MODULE:= libnxcinema
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../lib

include $(BUILD_SHARED_LIBRARY)
