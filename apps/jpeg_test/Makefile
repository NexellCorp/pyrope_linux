########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib
LIBRARY += -lnxjpeg -lturbojpeg -lnxdsp -lnxv4l2 -lnxvmem
LIBRARY	+= -lstdc++

######################################################################
# Target
COBJS	:= jpeg_dec.o
CPPOBJS	:=  

CFLAGS	+= -g

TARGET := jpeg_dec

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	rm -f $(TARGET) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
