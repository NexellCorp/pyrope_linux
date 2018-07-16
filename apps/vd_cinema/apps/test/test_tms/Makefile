################################################################################
# Native Linux Build Enviornment
OBJTREE		:= $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR))

################################################################################
# Toolchain
CC 		 	:= gcc
CXX		 	:= g++
AR 		 	:= ar
AS			:= as
LD 		 	:= ld
NM 		 	:= nm
RANLIB 	 	:= ranlib
OBJCOPY	 	:= objcopy
STRIP	 	:= strip

################################################################################
# Library & Header
INCLUDE   	:=
LIBRARY		:=

################################################################################
# Build Options
OPTS		:= -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter -Wshadow -Wwrite-strings -Wcast-qual -fno-strict-aliasing -fstrict-overflow -fsigned-char -fno-omit-frame-pointer -fno-optimize-sibling-calls
COPTS 		:= $(OPTS)
CXXOPTS 	:= $(OPTS) -Wnon-virtual-dtor

CFLAGS 	 	:= $(COPTS)
CXXFLAGS 	:= $(CXXOPTS)
AFLAGS 		:=

ARFLAGS		:= crv
LDFLAGS  	:=

################################################################################
# Generic Rules
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

################################################################################
# Build options
INCLUDE += -I./ -I../../../src/include
INCLUDE += -I./ -I../../../src/gdc

LIBRARY	+= -L../../../lib
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CXXOBJS	:= tms_client.o
CXXOBJS	+= ../../../src/gdc/gdc_protocol.o
CXXOBJS	+= ../../../src/gdc/NX_TMSClient.o
CXXOBJS += ../../../src/utils/NX_Utils.o
CXXOBJS += ../../../src/utils/SockUtils.o
TARGET	:= nx_tms_client

COBJS2	:=
CXXOBJS2:= tms_server.o
CXXOBJS2+= ../../../src/gdc/gdc_protocol.o
CXXOBJS2+= ../../../src/gdc/NX_TMSServer.o
CXXOBJS2+= ../../../src/utils/NX_Utils.o
CXXOBJS2+= ../../../src/utils/SockUtils.o
CXXOBJS2+= ../../../src/utils/NX_DbgMsg.o
TARGET2	:= nx_tms_server

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)
OBJS2	:= $(COBJS2) $(CXXOBJS2)

all: $(TARGET) $(TARGET2)

$(TARGET): depend $(OBJS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(OBJS) -o $(TARGET) $(LIBRARY) -g

$(TARGET2): depend $(OBJS2)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(OBJS2) -o $(TARGET2) $(LIBRARY) -g

clean:
	rm -f $(TARGET) $(TARGET2) *.o .depend
	rm -f ../../../src/gdc/*.o
	rm -f ../../../src/utils/*.o

install:

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp) $(AOBJS:.o=.s)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(INCS) $(SRCS) > .depend
