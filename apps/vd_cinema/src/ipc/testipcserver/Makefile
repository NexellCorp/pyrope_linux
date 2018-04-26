################################################################################
# Get Linux Build Enviornment:
include ./build.env

################################################################################
# Build options
INCLUDE	+= -I./ \
			-I./../ \
			-I./../../include/
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CPPOBJS	:= main.o \
			./../../utils/NX_Utils.o \
			./../../utils/NX_DbgMsg.o \
			./../../utils/SockUtils.o \
			./../ipc_protocol.o \
			./../NX_IPCServer.o

TARGET	:= NxTest

################################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
		$(CC) $(CPPFLAGS) $(OBJS) -o $(TARGET) $(LIBRARY) -g

clean:
		rm -f $(TARGET) *.o .depend

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
		$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
