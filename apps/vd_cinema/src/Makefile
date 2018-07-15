################################################################################
# Get Linux Build Enviornment:
VD_CINEMA_TOP := ../
include $(VD_CINEMA_TOP)/build.env


################################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS	+= -fpic -DARM_11
CXXFLAGS+= -fpic -DARM_11

INCLUDE += \
	-I $(VD_CINEMA_TOP)/src/include \
	-I $(VD_CINEMA_TOP)/src/uart \
	-I $(VD_CINEMA_TOP)/src/gdc \
	-I $(VD_CINEMA_TOP)/src/utils

LIBRARY += -lssl -lcrypto
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CPPOBJS	:=

# Add UART Sources
CXXOBJS += \
	uart/CNX_Uart.o	\
	uart/NX_UartProtocol.o \
	uart/NX_SecureLinkServer.o

# Add Utils Sources
CXXOBJS += \
	utils/crc32.o	\
	utils/NX_Utils.o \
	utils/ping.o \
	utils/NX_DbgMsg.o \
	utils/NX_Queue.o \
	utils/CNX_GpioControl.o \
	utils/SockUtils.o

CXXOBJS +=	\
	gdc/gdc_protocol.o		\
	gdc/CNX_OpenSSL.o		\
	gdc/NX_GDCServer.o		\
	gdc/NX_GDCClient.o

LIBNAME	:= libnxcinema_linux
TARGET	:= $(LIBNAME).so

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)

clean:
	rm -f $(TARGET) $(OBJS) .depend

install:
	cp $(TARGET) ../lib/

distclean: clean
	rm ../lib/$(TARGET)

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(INCS) $(SRCS) > .depend
