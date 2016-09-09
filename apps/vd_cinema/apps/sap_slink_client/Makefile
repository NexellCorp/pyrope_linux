################################################################################
# Get Linux Build Enviornment:
VD_CINEMA_TOP := ../..
include $(VD_CINEMA_TOP)/build.env

################################################################################
# Build options
INCLUDE += \
	-I./ \
	-I$(VD_CINEMA_TOP)/src/include \
	-I$(VD_CINEMA_TOP)/src/utils \
	-I$(VD_CINEMA_TOP)/src/uart


LIBRARY	+= -L$(VD_CINEMA_TOP)/lib
LIBRARY += -lnxcinema_linux
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CPPOBJS	:= main.o

TARGET	:= sap_slink_client

################################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET) $(TARGET2)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(TARGET) $(LIBRARY) -g

clean:
	rm -f $(TARGET) $(TARGET2) *.o .depend

install:

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
