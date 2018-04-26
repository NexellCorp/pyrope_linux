LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# ADD I2CController
LOCAL_SRC_FILES += \
	i2ccontroller/NX_I2CController.cpp

# ADD I2CCommand
LOCAL_SRC_FILES += \
	i2ccontroller/i2ccommand/NX_Command.cpp

# Add IPC Server
LOCAL_SRC_FILES += \
	ipc/ipc_protocol.cpp \
	ipc/NX_IPCClient.cpp \
	ipc/NX_IPCServer.cpp

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

# Add EEPRom Sources
LOCAL_SRC_FILES += \
	eeprom/CNX_EEPRom.cpp	\
	eeprom/CNX_EEPRomDataParser.cpp

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
	$(LOCAL_PATH)/i2ccontroller \
	$(LOCAL_PATH)/i2ccontroller/i2ccommand \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/i2c \
	$(LOCAL_PATH)/uart \
	$(LOCAL_PATH)/utils \
	$(LOCAL_PATH)/ipc \
	$(LOCAL_PATH)/gdc \
	$(LOCAL_PATH)/tms

LOCAL_SHARED_LIBRARIES := \
	liblog

LOCAL_STATIC_LIBRARIES +=

LOCAL_LDFLAGS +=

LOCAL_MODULE:= libnxcinema
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../lib

include $(BUILD_SHARED_LIBRARY)
